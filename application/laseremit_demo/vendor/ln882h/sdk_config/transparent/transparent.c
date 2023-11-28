#include "transparent.h"
#include "net/dhcpd/dhcpd.h"

#include "ln_at.h"
#include "ln_at_transfer.h"
#include "system_parameter.h"
#include "wifi.h"

#include "transport_udp.h"
#include "transport_tcp.h"
#include "transport_ssl.h"

static OS_Thread_t g_try_exit_thread;
static OS_Thread_t g_cipserver_thread;
static OS_Semaphore_t g_cipserver_sema;

static void init_trans_sys_handle(void);
static int transp_enter(int id);
static void transp_exit(int id);
static int push_back_fifo_buffer(unsigned char *data, unsigned int data_len);
static int cip_server(int mode, int port);
static void try_task_exit(void *params);

ln_trans_sys_stat_item_t g_TransSysStatCtrl = {
    ._init 			= init_trans_sys_handle,
	._start_link 	= transp_enter,
	._exit_link 	= transp_exit,
	._push_back 	= push_back_fifo_buffer,
	._start_server	= cip_server,
};

ln_trans_sys_stat_handle_t ln_get_tcpip_handle(void)
{
    return &g_TransSysStatCtrl;
}

static void init_trans_sys_handle(void)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
	
	p->bCipDinfo = false;

    p->ServerSock = -1;
    p->ServerPort = 0;
	p->ServerMaxConn = 5;

    p->ssl_existed = false;
    p->ssl_buff_size = 2048;
	
	for (int i = 0; i < TRANSP_MAX_LINK_NUM; i++)
	{
		strcpy(p->tLinkItems[i].peerName, "ssl_client");
	}

    if (OS_OK != OS_ThreadCreate(&g_try_exit_thread, "try_task_exit", try_task_exit, NULL, OS_PRIORITY_BELOW_NORMAL, TRY_EXIT_TASK_STACK_SIZE))
    {
        LN_ASSERT(1);
    }
}

static void transp_from_buf_to_net_timer_cb(void *arg)
{
    int *pId = (int *)arg;
    Log_d("ID: %d", *pId);
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    ln_netlink_handle_t pLink = &(p->tLinkItems[*pId]);

    unsigned int data_len = Fifo_2_Get_Buflen(&pLink->transp_fifo);
    if (data_len == 0)
        return;

    if (OS_OK != OS_SemaphoreWait(&pLink->transp_sem, 0))
    {
        return;
    }

    data_len = Fifo_2_Get_Data(&pLink->transp_fifo, pLink->tx_buff + pLink->tx_offset, data_len);
    OS_SemaphoreRelease(&pLink->transp_sem);

    pLink->tx_offset += data_len;
    pLink->tx_buff[pLink->tx_offset] = 0;
    Log_d("tx_offset: %d", pLink->tx_offset);
    Log_i("pLink->tx_buff: %d bytes: %s", data_len, pLink->tx_buff);

    if ((pLink->type == TRANSPARENT_MODE_TCP) || (pLink->type == TRANSPARENT_MODE_UDP))
    {
        if (p->bCipMode)
        {
            pLink->tcpHandle->_write(pLink->tcpHandle, (const char *)pLink->tx_buff, data_len, 0);
            pLink->tx_offset = 0;
        }
        else
        {
            Log_d("txOfst= %d, sendlen= %d", pLink->tx_offset, pLink->sendLen);
            if (pLink->tx_offset >= pLink->sendLen)
            {
                if (pLink->tx_offset > pLink->sendLen)
                {
                    ln_at_printf("busy s...\r\n");
                }

                char tmp[20] = {0};
                sprintf(tmp, "\r\nRecv %d bytes\r\n", pLink->sendLen);
                ln_at_printf("%s", tmp);

                pLink->tcpHandle->_write(pLink->tcpHandle, (const char *)pLink->tx_buff, pLink->sendLen, 0);

                pLink->sendLen = 0;
                pLink->tx_offset = 0;
                pLink->tx_buff[0] = 0;
                p->bSend = 0;
                ln_at_printf("\r\nSEND OK\r\n");
            }
        }
    }
    else if (pLink->type == TRANSPARENT_MODE_SSL)
    {
        if (p->bCipMode)
        {
            ssl_send(pLink->tx_buff, data_len);
            pLink->tx_offset = 0;
        }
        else
        {
            Log_d("txOfst= %d, sendlen= %d", pLink->tx_offset, pLink->sendLen);
            if (pLink->tx_offset >= pLink->sendLen)
            {
                if (pLink->tx_offset > pLink->sendLen)
                {
                    ln_at_printf("busy s...\r\n");
                }

                char tmp[20] = {0};
                sprintf(tmp, "\r\nRecv %d bytes\r\n", pLink->sendLen);
                ln_at_printf("%s", tmp);

                ssl_send(pLink->tx_buff, pLink->sendLen);

                pLink->sendLen = 0;
                pLink->tx_offset = 0;
                pLink->tx_buff[0] = 0;
                p->bSend = 0;
                ln_at_printf("\r\nSEND OK\r\n");
            }
        }
    }
}

static void transp_net_recv_task(void *params)
{
    char *pId = (char *)params;
    Log_d("ID: %d", *pId);
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    ln_netlink_handle_t pLink = &(p->tLinkItems[*pId]);
    int bytes_received;
		pLink->bCipRcvHasFetched = 1;

    while (1)
    {
        if ((pLink->type == TRANSPARENT_MODE_TCP) || (pLink->type == TRANSPARENT_MODE_UDP))
        {
            if (p->bCipRecvMode 
            && TRANSPARENT_MODE_TCP == pLink->type 
            && pLink->rx_offset >= pLink->buff_size - 1)
            {
                // TCP && ciprecvmode == 1, if rx_buff is full
                continue;
            }
            else
            {
                bytes_received = pLink->tcpHandle->_read(pLink->tcpHandle, (char *)pLink->rx_buff + pLink->rx_offset, pLink->buff_size - pLink->rx_offset - 1, 0);
                if (bytes_received >= 0)
				{
					pLink->rx_offset += bytes_received;
				}
            }
        }
        else if (pLink->type == TRANSPARENT_MODE_SSL)
        {
            bytes_received = ssl_rece(pLink->rx_buff, pLink->buff_size - 1);
            pLink->rx_offset = bytes_received;
        }

//        Log_i("bytes_received = %d", bytes_received);
        if (bytes_received < 0)
        {
            Log_i("transp_tcp_task recv error %d.", bytes_received);
			if (!p->bCipRecvMode)
			{
				OS_QueueSend(&p->tryCloseQueue, &pLink->id, 100);
			}
			else
			{
				pLink->bIsClosed = true;
			}

			while (1)
            ;
        }

        pLink->rx_buff[pLink->rx_offset] = 0;
        if (bytes_received > 0)
        {
            if (p->bCipMode && pLink->type != TRANSPARENT_MODE_SSL)
            {
                //cipmode == 1
				ln_at_write((char*)pLink->rx_buff, pLink->rx_offset);
                pLink->rx_offset = 0;
            }
            else if (p->bCipRecvMode && pLink->type == TRANSPARENT_MODE_TCP)
            {
                //TCP && ciprecvmode == 1
				if (pLink->bCipRcvHasFetched)
				{
					pLink->bCipRcvHasFetched = 0;
					if (p->bCipMux)
					{
						ln_at_printf("\r\n+IPD,%d,%d\r\n", pLink->id, pLink->rx_offset);
					}
					else
					{
						ln_at_printf("\r\n+IPD,%d\r\n", pLink->rx_offset);
					}					
				}
            }
            else
            {
                if (p->bCipMux)
                {
                    ln_at_printf("\r\n+IPD,%d,%d", pLink->id, pLink->rx_offset);
                }
                else
                {
                    ln_at_printf("\r\n+IPD,%d", pLink->rx_offset);
                }
				
				if (p->bCipDinfo)
				{
					ln_at_printf(",\"%s\",%d", pLink->remoteIp, pLink->remotePort);
				}
				ln_at_printf(":");
				ln_at_write((char*)pLink->rx_buff, pLink->rx_offset);
                ln_at_write("\r\n", 2);
                pLink->rx_offset = 0;
            }
        }
    }
}

static int transp_create_tcpudp_link(int id, ln_transparent_link_mode_t mode)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    ln_netlink_handle_t pLink = &(p->tLinkItems[id]);
    Log_d("ID:%d", id);

    if (pLink->tcpHandle == 0)
    {
        Log_d("ID:%d socket initial!", id);
        if (mode == TRANSPARENT_MODE_TCP)
            pLink->tcpHandle = ln_transport_tcp_init();
        else if (mode == TRANSPARENT_MODE_UDP)
        {
            pLink->tcpHandle = ln_transport_udp_init();
        }
        else
            return -1;
    }

    if (pLink->tcpHandle->_connect(pLink->tcpHandle, pLink->remoteIp, pLink->remotePort, 5000))
    {
        Log_i("TCP Connect fail!\r\n");
        pLink->tcpHandle->_destroy(pLink->tcpHandle);
        pLink->tcpHandle = NULL;
        return -1;
    }
    Log_i("Link[%d] Connect successful!", id);

    return 0;
}

static void transp_delete_tcpudp_link(int id)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    ln_netlink_handle_t pLink = &(p->tLinkItems[id]);

    pLink->transp_task_tcp.handle = NULL;
    pLink->tcpHandle->_destroy(pLink->tcpHandle);
    ln_transport_destroy(pLink->tcpHandle);
    pLink->tcpHandle = NULL;
}

static int malloc_tran_buf(ln_netlink_handle_t _pLink)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

    if (_pLink->isMalloced)
    {
        return 0;
    }

    _pLink->buff_size = TRANSP_BUF_LEN;
    if (_pLink->type == TRANSPARENT_MODE_SSL)
    {
        _pLink->buff_size = p->ssl_buff_size;
    }

    _pLink->rx_buff = OS_Malloc(sizeof(unsigned char) * _pLink->buff_size);
    LN_TRANSPORT_MEM_CHECK(_pLink->rx_buff, return -1);
    _pLink->tx_buff = OS_Malloc(sizeof(unsigned char) * _pLink->buff_size);
    LN_TRANSPORT_MEM_CHECK(_pLink->tx_buff, return -1);
    _pLink->fifo_buff = OS_Malloc(sizeof(unsigned char) * _pLink->buff_size);
    LN_TRANSPORT_MEM_CHECK(_pLink->fifo_buff, return -1);

    _pLink->isMalloced = true;

    return 0;
}

static void delete_tran_buf(ln_netlink_handle_t _pLink)
{
    if (!_pLink->isMalloced)
    {
        return;
    }

    Log_i("Delete _pLink buff");

    OS_Free(_pLink->rx_buff);
    _pLink->rx_buff = NULL;
    OS_Free(_pLink->tx_buff);
    _pLink->tx_buff = NULL;
    OS_Free(_pLink->fifo_buff);
    _pLink->fifo_buff = NULL;

    _pLink->isMalloced = false;
    _pLink->type = TRANSPARENT_NO_LINK;
}

static int init_transp_timer(ln_netlink_handle_t _pLink)
{
    if (_pLink == NULL)
    {
        Log_e("invalid plink handle!");
        return -1;
    }
    
    if (OS_SemaphoreCreate(&_pLink->transp_sem, 1, 1024) != OS_OK)
    {
        Log_e("OS_SemaphoreCreate sem fail.");
        return -1;
    }

    Fifo_2_Init_Buffer(&_pLink->transp_fifo, _pLink->fifo_buff, _pLink->buff_size);
    OS_TimerCreate(&_pLink->transp_timer, OS_TIMER_PERIODIC, transp_from_buf_to_net_timer_cb, (void *)&(_pLink->id), 10);
    OS_TimerStart(&_pLink->transp_timer);

    return 0;
}

static int init_tcp_udp_link(ln_netlink_handle_t _pLink)
{
    char taskName[20] = {0};
    if (transp_create_tcpudp_link(_pLink->id, (ln_transparent_link_mode_t)_pLink->type) != 0)
    {
        Log_e("transp_create_link failed");
        return -1;
    }

    if (malloc_tran_buf(_pLink) != 0)
    {
        Log_e("malloc_tran_buf failed");
        goto err;
    }

    sprintf(taskName, "tcptask%d", _pLink->id);
    if (OS_OK != OS_ThreadCreate(&_pLink->transp_task_tcp, taskName, transp_net_recv_task, (void *)&(_pLink->id), OS_PRIORITY_BELOW_NORMAL, TRANSP_STACK_SIZE))
    {
        Log_e("OS_ThreadCreate fail!");
        goto err;
    }

    if (init_transp_timer(_pLink) != 0)
    {
        goto err;
    }

    return 0;
err:
    delete_tran_buf(_pLink);
    _pLink->tcpHandle->_destroy(_pLink->tcpHandle);
    _pLink->tcpHandle = NULL;

    return -1;
}

static int init_ssl_link(ln_netlink_handle_t _pLink)
{
	ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
	
    if (transp_create_ssl_link(_pLink->id) != 0)
    {
        Log_e("transp_create_ssl_link failed!");
        return -1;
    }

    if (malloc_tran_buf(_pLink) != 0)
    {
        Log_e("malloc_tran_buf failed!");
        goto err;
    }

    if (OS_OK != OS_ThreadCreate(&_pLink->transp_task_tcp, "task-ssl", transp_net_recv_task, (void *)&(_pLink->id), OS_PRIORITY_BELOW_NORMAL, TRANSP_SSL_STACK_SIZE))
    {
        Log_e("OS_ThreadCreate ssl task fail!");
        goto err;
    }

    if (init_transp_timer(_pLink) != 0)
    {
        goto err;
    }

    p->ssl_existed = true;

    return 0;
err:
    transp_delete_ssl_link(_pLink->id);
    delete_tran_buf(_pLink);
    return -1;
}

static int transp_enter(int id)
{
	ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    ln_netlink_handle_t pLink = &(p->tLinkItems[id]);
	pLink->bCipRcvHasFetched = 1;

    switch (pLink->type)
    {
    case TRANSPARENT_MODE_UDP:
    case TRANSPARENT_MODE_TCP:
        return init_tcp_udp_link(pLink);
    case TRANSPARENT_MODE_SSL:
        return init_ssl_link(pLink);
    default:
        Log_e("Error TRANSPARENT TYPE : %d!", pLink->type);
        return -1;
    }
}

static void transp_exit(int id)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    ln_netlink_handle_t pLink = &(p->tLinkItems[id]);

    if (pLink->type == TRANSPARENT_NO_LINK)
    {
        return;
    }
		
    if (p->bCipMux)
    {
        ln_at_printf("%d,CLOSED\r\n", id);
    }
    else
    {
        ln_at_printf("CLOSED\r\n");
    }
    Log_i("OS_TimerStop");

    pLink->rx_offset = 0;
    p->bSend = 0;

    if (pLink->transp_timer.handle)
    {
        OS_TimerStop(&pLink->transp_timer);
        OS_TimerDelete(&pLink->transp_timer);
        OS_SemaphoreDelete(&pLink->transp_sem);
    }

    if (pLink->transp_task_tcp.handle != NULL)
    {
        Log_i("OS_ThreadDelete");
        OS_ThreadDelete(&pLink->transp_task_tcp);
        pLink->transp_task_tcp.handle = NULL;
    }

    if (pLink->type == TRANSPARENT_MODE_TCP || pLink->type == TRANSPARENT_MODE_UDP)
    {
        Log_i("Do delete tcpudp link");
        transp_delete_tcpudp_link(id);
    }
    else if (pLink->type == TRANSPARENT_MODE_SSL)
    {
        Log_i("Do delete ssl link");
        transp_delete_ssl_link(id);
    }

    delete_tran_buf(pLink);
}

static int push_back_fifo_buffer(unsigned char *data, unsigned int data_len)
{
    unsigned char StopSendFlag[3] = {0};
    unsigned int fifo_data_len = 0;

    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    ln_netlink_handle_t pLink = &(p->tLinkItems[p->curId]);
    //LOG(LOG_LVL_DEBUG, "Current id:%d\r\n", p->curId);

    if (pLink->type >= TRANSPARENT_MODE_ERROR || pLink->type == TRANSPARENT_NO_LINK)
    {
        Log_e("Error Type: %d!", pLink->type);
        return -1;
    }

    if (data_len == 0 || data == NULL)
    {
        Log_e("data len error!");
        return -1;
    }

    if (OS_OK == OS_SemaphoreWait(&pLink->transp_sem, OS_WAIT_FOREVER))
    {
        if (data_len == 1)
            Fifo_2_Save_Char(&pLink->transp_fifo, *data);
        else
            Fifo_2_Save_Data(&pLink->transp_fifo, data, data_len);

        fifo_data_len = Fifo_2_Get_Buflen(&pLink->transp_fifo);
        if (fifo_data_len == 3)
        {
            Fifo_2_Get_Data(&pLink->transp_fifo, StopSendFlag, fifo_data_len);
            if (StopSendFlag[0] == '+' && StopSendFlag[1] == '+' && StopSendFlag[2] == '+')
            {
                Log_i("Get \"+++\", exit cipmode send!");
                OS_SemaphoreRelease(&pLink->transp_sem);
                return 0;
            }
            else
            {
                Fifo_2_Save_Data(&pLink->transp_fifo, StopSendFlag, fifo_data_len);
            }
        }
    }

    OS_SemaphoreRelease(&pLink->transp_sem);
    return 1;
}

static void try_task_exit(void *params)
{
    int i = 0;
	int closeId = 0;
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

	if (OS_QueueCreate(&p->tryCloseQueue, 5, sizeof(int)) != OS_OK)
	{
		Log_e("try close queue create failed...");
        while (1)
            ;
	}

    while (1)
    {
		if(OS_OK == OS_QueueReceive(&p->tryCloseQueue, (void*)&closeId, OS_WAIT_FOREVER))
		{
			if (closeId >= TRANSP_MAX_LINK_NUM || closeId < 0)
			{
				continue;
			}
			
            p->_exit_link(closeId);
            p->cStaStat = STA_DISCON;
            for (i = 0; i < 5; i++)
            {
                //LOG(LOG_LVL_INFO, "p->tLinkItems[%d].type = %d\r\n", i, p->tLinkItems[i].type);
                if (p->tLinkItems[i].type != TRANSPARENT_NO_LINK)
                {
                    p->cStaStat = STA_TCPUDP;
                    break;
                }
            }
        }
    }
}

static int transp_bind_tcpudp_link(ln_transparent_link_mode_t mode, int sock_connected)
{
	char taskName[20] = {0};
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    ln_netlink_handle_t pLink = NULL;
    int index;
		struct sockaddr_in client_addr;
		socklen_t size = sizeof(struct sockaddr_in);
		
    for (index = 0; index < TRANSP_MAX_LINK_NUM; index++)
    {
        pLink = &(p->tLinkItems[index]);
        if (pLink->tcpHandle == NULL)
            break;
    }

    if (index >= TRANSP_MAX_LINK_NUM)
        return -1;

    Log_i("ID:%d bind socket %d !", index, sock_connected);
    if (mode == TRANSPARENT_MODE_TCP)
    {
        pLink->tcpHandle = ln_transport_tcp_init();
        ln_transport_tcp_bind_socket(pLink->tcpHandle, sock_connected);
    }
    else if (mode == TRANSPARENT_MODE_UDP)
    {
        pLink->tcpHandle = ln_transport_udp_init();
        ln_transport_udp_bind_socket(pLink->tcpHandle, sock_connected);
    }
    else
        return -1;

		if(-1==getpeername(sock_connected , (struct sockaddr *)&client_addr , &size))
		{
			Log_e("tcp server get client ip fail...\r\n");
			goto err;
		}
		
		pLink->tcpHandle->port = p->ServerPort;
		pLink->remotePort = client_addr.sin_port;
		pLink->tetype = 1;
		strcpy(pLink->remoteIp , inet_ntoa(client_addr.sin_addr));
		pLink->id = index;
    pLink->type = mode;

    if (malloc_tran_buf(pLink) != 0)
    {
        Log_e("malloc_tran_buf failed");
        goto err;
    }
	
    sprintf(taskName, "tcptask%d", pLink->id);
    if (OS_OK != OS_ThreadCreate(&pLink->transp_task_tcp, taskName, transp_net_recv_task, (void *)&(pLink->id), OS_PRIORITY_BELOW_NORMAL, TRANSP_STACK_SIZE))
    {
        Log_e("OS_ThreadCreate fail!");
        goto err;
    }

    if (init_transp_timer(pLink) != 0)
    {
        goto err;
    }
		

    ln_at_printf("%d,CONNECT\r\n", index);

    return 0;
err:
    delete_tran_buf(pLink);
    pLink->tcpHandle->_destroy(pLink->tcpHandle);
    pLink->tcpHandle = NULL;
    return -1;
}

static void cipserver_close(void)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

    if (p->ServerSock > -1)
    {
        Log_i("close server socket:%d!\r\n", p->ServerSock);
        //shutdown(p->ServerSock, SHUT_RDWR);
        close(p->ServerSock);
        p->ServerSock = -1;
		OS_ThreadDelete(&g_cipserver_thread);
    }
}

wifi_mode_t ln_get_wifi_mode_from_at(void);
static void cipserver_task_entry(void *params)
{
    socklen_t sin_size;
    int sock, connected = -1;
    struct sockaddr_in server_addr, client_addr;
    int one = 1;
	
		ln_netlink_handle_t pLink = NULL;

    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        Log_e("Socket error");
        return;
    }
	
    /* 初始化服务端地址 */
	tcpip_ip_info_t ip_info = {0};
	netdev_get_ip_info(netdev_get_active(), &ip_info);
	
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(p->ServerPort); /* 服务端工作的端口 */
    server_addr.sin_addr.s_addr = ip_info.ip.addr;
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
    {
        Log_e("setsockopt SO_REUSEADDR fail!");
        goto __exit;
    }

    Log_i("Sock:%d ready to bind and listen %d @ %s ...", sock, p->ServerPort, inet_ntoa(ip_info.ip.addr));
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        Log_e("bind Server %d @ %s fail!", p->ServerPort, inet_ntoa(ip_info.ip.addr));
        goto __exit;
    }
	Log_i("bind Server %d @ %s OK", p->ServerPort, inet_ntoa(ip_info.ip.addr));

    if (listen(sock, p->ServerMaxConn) == -1)
    {
        Log_e("listen Server %d @ %s fail!", p->ServerPort, inet_ntoa(ip_info.ip.addr));
        goto __exit;
    }
	Log_i("listen Server %d @ %s OK", p->ServerPort, inet_ntoa(ip_info.ip.addr));

    p->ServerSock = sock;
    OS_SemaphoreRelease(&g_cipserver_sema);
    sin_size = sizeof(struct sockaddr_in);
    while (connected < 0)
    {
        /* 接受一个客户端连接socket的请求，这个函数调用是阻塞式的 */
        Log_i("Waiting Client ...(%d)", connected);
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
		//netconn_set_recvtimeout(newconn, 200);
        /* 返回的是连接成功的socket */
        if (connected < 0)
        {
            Log_e("Accept connection failed! errno = %d", errno);
            continue;
        }
        else
				{
					Log_i("Client (%d) Connected", connected);
				
					for (uint8_t index = 0; index < TRANSP_MAX_LINK_NUM; index++)
					{
							pLink = &(p->tLinkItems[index]);
							if (pLink->tcpHandle == NULL)
									break;
					}
					
					transp_bind_tcpudp_link(TRANSPARENT_MODE_TCP, connected);
					p->cStaStat = STA_TCPUDP;
			
					connected = -1;
				}
    }

__exit:
    OS_ThreadDelete(NULL);
    return;
}

static int cip_server(int mode, int port)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    if (p->bCipMux != 1)
        return LN_ERR_INVALID_STATE;

    Log_i("mode:%d, port:%d", mode, port);
    if (mode == 1)
    {
        Log_i("ServerSock:%d", p->ServerSock);
        if (p->ServerSock != -1)
        {
            //ln_at_printf("no change");
            return LN_OK;
        }

        p->ServerPort = port;

        if (OS_OK != OS_SemaphoreCreate(&g_cipserver_sema, 0, 1))
        {
            Log_e("OS_SemaphoreCreate error!");
            return LN_ERR_INVALID_ARG;
        }

        if (OS_OK != OS_ThreadCreate(&g_cipserver_thread, "cip", cipserver_task_entry, NULL, OS_PRIORITY_NORMAL, TCP_CIPSERVER_TASK_STACK_SIZE))
        {
            Log_e("OS_ThreadCreate error!");
			OS_SemaphoreDelete(&g_cipserver_sema);
            return LN_ERR_INVALID_ARG;
        }

        Log_i("wait for OS_SemaphoreRelease");
        if (OS_OK != OS_SemaphoreWait(&g_cipserver_sema, 40000))
		{
			OS_SemaphoreDelete(&g_cipserver_sema);
			return LN_ERR_TIMEOUT;
		}
        OS_SemaphoreDelete(&g_cipserver_sema);
    }
    else if (mode == 0)
    {
		cipserver_close();
    }
    else
        return LN_ERR_INVALID_ARG;

    return LN_OK;
}
