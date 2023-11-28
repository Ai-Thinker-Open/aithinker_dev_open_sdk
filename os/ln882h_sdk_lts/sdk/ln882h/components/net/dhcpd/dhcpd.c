#include "dhcp_packet.h"
#include "dhcpd.h"
#include "dhcpd_api.h"
#include "lwip/sockets.h"
#include "osal/osal.h"
#include "ln_types.h"
#include "wifi.h"

#define DHCPD_TASK_STACK_SIZE   256*5

#define DHCP_HOST_NETMASK                 ("255.255.255.0")
#define DHCP_HOST_DNS1                    ("114.114.114.114")
#define DHCP_SERVER_NAME_STR              ("LN-DHCP-Server")
#define DHCP_HOST_NAME_STR                ("LN882H-SoftAP")

struct raw_msg {
    char               *send_buff;
    char               *recv_buff;
    uint32_t            recv_len;
    struct sockaddr_in  sockaddr_from;
    int                 socket_fd;
};

typedef enum {
    DHCPD_TASK_NOT_CREATED = 0,
    DHCPD_TASK_IS_CREATED,
    DHCPD_TASK_IS_RUNNING,
    DHCPD_TASK_SIGNAL_EXIT,
} dhcpd_flag_e;

typedef struct dhcpd_ctrl{
    OS_Thread_t                 dhcpd_thr;
    int                         dhcp_socket;
    volatile dhcpd_flag_e       flag;
    server_config_t             server_config;
    dhcpd_ip_item_t             ip_pool[DHCPD_IP_POOL_SIZE];
    struct raw_msg              msg;
} dhcpd_ctrl_t;

static dhcpd_ctrl_t g_dhcpd_ctrl = {0, -1, DHCPD_TASK_NOT_CREATED, };

static void dhcpd_handle_msg(struct raw_msg *msg);
static void dhcpd_close_socket(int *dhcp_socket);

static int dhcpd_dispatch(struct raw_msg *msg, uint16_t *send_pkt_len, uint16_t max_mtu);
static int do_discover(struct raw_msg *msg, uint16_t *send_pkt_len, uint16_t max_mtu);
static int do_request (struct raw_msg *msg, uint16_t *send_pkt_len, uint16_t max_mtu);
static int do_release (struct raw_msg *msg, uint16_t *send_pkt_len, uint16_t max_mtu);
static int do_inform  (struct raw_msg *msg, uint16_t *send_pkt_len, uint16_t max_mtu);
static int do_decline (struct raw_msg *msg, uint16_t *send_pkt_len, uint16_t max_mtu);

static dhcpd_ctrl_t *dhcpd_get_handle(void)
{
    return &g_dhcpd_ctrl;
}

static void dhcpd_htonl_copy(uint8_t * dest, uint32_t src)
{
    uint32_t tmp = lwip_htonl(src);
    memcpy(dest, &tmp, 4);
}

static void dhcpd_task( void *pvParameters )
{
    int so_broadcast = 1, so_reuseaddr = 1, rcv_size = 0;
    struct sockaddr_in server_address;
    struct raw_msg *msg = NULL;
    dhcpd_ctrl_t *dhcpd = (dhcpd_ctrl_t *)pvParameters;
    server_config_t *server_config = &(dhcpd->server_config);
    fd_set read_set;
    struct timeval timeout;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    timeout.tv_sec  = 0;
    timeout.tv_usec = 50*1000;//units:us

    if((dhcpd->dhcp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {//216Byte
        DHCPD_ERR("Cannot open the socket!\r\n");
        goto out;
    }

    setsockopt(dhcpd->dhcp_socket, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));
    setsockopt(dhcpd->dhcp_socket, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof(so_broadcast));

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_config->port);
    server_address.sin_addr.s_addr = ip4_addr_get_u32(&server_config->server);

    if(bind(dhcpd->dhcp_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        DHCPD_ERR("Cannot bind the socket with the address!\r\n");
        goto out;
    }
	DHCPD_PRINTF("dhcpd_task start...\r\n");

    msg = &(dhcpd->msg);
    memset(msg, 0, sizeof(struct raw_msg));
    memset(&msg->sockaddr_from, 0, sizeof(struct sockaddr));
    msg->sockaddr_from.sin_family = AF_INET;
    msg->sockaddr_from.sin_port = htons(server_config->port);
    msg->sockaddr_from.sin_addr.s_addr = htonl(INADDR_ANY);
    msg->socket_fd = dhcpd->dhcp_socket;

    dhcpd->flag = DHCPD_TASK_IS_RUNNING;

    while(dhcpd->flag == DHCPD_TASK_IS_RUNNING)
    {
        FD_ZERO(&read_set);
        FD_SET(dhcpd->dhcp_socket, &read_set);

        if(0 < select(dhcpd->dhcp_socket+1, &read_set, NULL, NULL, &timeout))
        {
            if (FD_ISSET(dhcpd->dhcp_socket, &read_set))
            {
                if (msg->recv_buff == NULL) {
                    if(NULL == (msg->recv_buff = OS_Malloc(DHCP_MAX_MTU))) {
                        DHCPD_ERR("[%s, %d]Allocate memory failed!\r\n", __func__, __LINE__);
                        continue;
                    }
                }

                if (msg->send_buff == NULL) {
                    if(NULL == (msg->send_buff = OS_Malloc(DHCP_MAX_MTU))) {
                        DHCPD_ERR("[%s, %d]Allocate memory failed!\r\n", __func__, __LINE__);
                        continue;
                    }
                }

                rcv_size = recvfrom(dhcpd->dhcp_socket, msg->recv_buff, DHCP_MAX_MTU, 0, (struct sockaddr*)&msg->sockaddr_from, &addr_len);
                if(rcv_size < 0) {
                    DHCPD_ERR("Receive data error!\r\n");
                    continue;
                }
                DHCPD_PRINTF("%d bytes received\r\n", rcv_size);

                msg->recv_len = (uint32_t)rcv_size;
                dhcpd_handle_msg(msg);

                if(msg->recv_buff != NULL) {
                    OS_Free(msg->recv_buff);
                    msg->recv_buff = NULL;
                }
                if(msg->send_buff != NULL) {
                    OS_Free(msg->send_buff);
                    msg->send_buff = NULL;
                }
            }
        }
    }

out:
	if (dhcpd->dhcp_socket >= 0) {
        dhcpd_close_socket(&dhcpd->dhcp_socket);
    }
	DHCPD_PRINTF("dhcpd_task exit!\r\n");

    if (dhcpd->msg.recv_buff != NULL) {
        OS_Free(dhcpd->msg.recv_buff);
        dhcpd->msg.recv_buff = NULL;
    }

    if (dhcpd->msg.send_buff != NULL) {
        OS_Free(dhcpd->msg.send_buff);
        dhcpd->msg.send_buff = NULL;
    }

    dhcpd->flag = DHCPD_TASK_NOT_CREATED;
    OS_ThreadDelete(&(dhcpd->dhcpd_thr));
    OS_ThreadSetInvalid( &(dhcpd->dhcpd_thr) );
    return;
}

int dhcpd_start(void)
{
    dhcpd_ctrl_t *dhcpd = dhcpd_get_handle();

    // DHCPD_ERR("------  DHCP START  ------\r\n");

    //TODO:
//    extern int system_parameter_get_dhcpd_config(server_config_t *server_config);
//    system_parameter_get_dhcpd_config(&(dhcpd->server_config));

    server_config_t * server_config = &(dhcpd->server_config);
    dhcpd_ip_item_t *ip_item = NULL;
    uint8_t ip_addr1, ip_addr2, ip_addr3, ip_addr4;

    if(dhcpd->flag != DHCPD_TASK_NOT_CREATED){
        return DHCPD_ERR_STATE;
    }
    dhcpd->flag = DHCPD_TASK_IS_CREATED;

    if(0 == ip4_addr_get_u32(&(server_config->server)) || 0 == server_config->port || 0 == server_config->lease || 0 == server_config->renew) {
        DHCPD_ERR("server_config parameter error!\r\n");
        return DHCPD_ERR_INVALID_PARAM;
    }

    DHCPD_PRINTF("-------DUMP DHCP SERVER_CONFIG----------\r\n");
    DHCPD_PRINTF("server_config.server=%d.%d.%d.%d\r\n", ip4_addr1(&(server_config->server)), ip4_addr2(&(server_config->server)), ip4_addr3(&(server_config->server)), ip4_addr4(&(server_config->server)));
    DHCPD_PRINTF("server_config.port=%d\r\n", server_config->port);
    DHCPD_PRINTF("server_config.lease=%u\r\n", server_config->lease);
    DHCPD_PRINTF("server_config.renew=%u\r\n", server_config->renew);
    DHCPD_PRINTF("server_config.ip_start=%d.%d.%d.%d\r\n", ip4_addr1(&(server_config->ip_start)), ip4_addr2(&(server_config->ip_start)), ip4_addr3(&(server_config->ip_start)), ip4_addr4(&(server_config->ip_start)));
    DHCPD_PRINTF("server_config.ip_end=%d.%d.%d.%d\r\n", ip4_addr1(&(server_config->ip_end)), ip4_addr2(&(server_config->ip_end)), ip4_addr3(&(server_config->ip_end)), ip4_addr4(&(server_config->ip_end)));
    DHCPD_PRINTF("server_config.max=%d\r\n", server_config->client_max);
    DHCPD_PRINTF("-----------------END--------------\r\n");

    uint32_t ip_pool_num = DHCPD_IP_POOL_SIZE < server_config->client_max ? DHCPD_IP_POOL_SIZE : server_config->client_max;
    for (uint32_t i = 0; i < ip_pool_num; i++) {
        ip_item = &(dhcpd->ip_pool[i]);
        ip_item->allocted = LN_FALSE;
        ip_addr1 = ip4_addr1(&(server_config->server));
        ip_addr2 = ip4_addr2(&(server_config->server));
        ip_addr3 = ip4_addr3(&(server_config->server));
        ip_addr4 = ip4_addr4(&(server_config->ip_start)) + i;
        IP_ADDR4(&(ip_item->ip), ip_addr1, ip_addr2, ip_addr3, ip_addr4);
        DHCPD_PRINTF("IP_POOL: i=%d %d.%d.%d.%d\r\n", i, ip4_addr1(&(ip_item->ip)), ip4_addr2(&(ip_item->ip)), ip4_addr3(&(ip_item->ip)), ip4_addr4(&(ip_item->ip)));
    }

    dhcpd->dhcp_socket = -1;

    if (OS_OK != OS_ThreadCreate(&(dhcpd->dhcpd_thr), "dhcpd", dhcpd_task, (void *)dhcpd, OS_PRIORITY_BELOW_NORMAL, DHCPD_TASK_STACK_SIZE)) {
        DHCPD_ERR("[%s, %d]OS_ThreadCreate dhcpd->dhcpd_thr fail.\r\n", __func__, __LINE__);
        dhcpd->flag = DHCPD_TASK_NOT_CREATED;
        return DHCPD_ERR_OS_SERVICE;
    }

    return DHCPD_ERR_NONE;
}

int dhcpd_stop(void)
{
    dhcpd_ctrl_t *dhcpd = dhcpd_get_handle();

    if(dhcpd->flag == DHCPD_TASK_IS_RUNNING)
    {
        /* notify dhcpd task exit */
        dhcpd->flag = DHCPD_TASK_SIGNAL_EXIT;
        do {
            OS_MsDelay(10);
        } while (dhcpd->flag != DHCPD_TASK_NOT_CREATED);

        // DHCPD_ERR("------  DHCP STOP  ------\r\n");
    }
    return DHCPD_ERR_NONE;
}

int dhcpd_is_running(void)
{
    dhcpd_ctrl_t *dhcpd = dhcpd_get_handle();
    return (dhcpd->flag == DHCPD_TASK_IS_RUNNING);
}

static void dhcpd_close_socket(int *dhcp_socket)
{
    if(*dhcp_socket >= 0) {
        close(*dhcp_socket);
        *dhcp_socket = -1;
    }
}

dhcpd_ip_item_t *dhcpd_get_ip_pool(void) {
    return g_dhcpd_ctrl.ip_pool;
}

uint8_t dhcpd_get_client_max(void) {
    return g_dhcpd_ctrl.server_config.client_max;
}

void dhcpd_handle_msg(struct raw_msg *msg)
{
    struct sockaddr_in sockaddr_to = {0};
    int send_len = 0;
    uint16_t send_pkt_len = 0;

    struct dhcp_packet *request  = (struct dhcp_packet *)msg->recv_buff;
    struct dhcp_packet *response = (struct dhcp_packet *)msg->recv_buff;

    if (!request || !response) {
        return;
    }

    if (LN_TRUE != dhcpd_dispatch(msg, &send_pkt_len, DHCP_MAX_MTU)) {
        return;
    }

    DHCPD_PRINTF("\r\n+------Dump BOOTP (Send to Client)-----\r\n");
    dhcpd_packet_printf((struct dhcp_packet *)msg->send_buff);
    DHCPD_PRINTF("+--------------------------------------------\r\n\r\n");

    if(send_pkt_len > 0) {
        sockaddr_to.sin_family = AF_INET;
        sockaddr_to.sin_port = htons(BOOTP_REPLAY_PORT);
        sockaddr_to.sin_addr.s_addr = htonl(INADDR_BROADCAST);

        send_len = sendto(msg->socket_fd, msg->send_buff, send_pkt_len, 0, (struct sockaddr*)&sockaddr_to, sizeof(struct sockaddr_in));//200Byte
        if(send_len < 0) {
            DHCPD_ERR("Send data error!\r\n");
        } else {
            DHCPD_PRINTF("Total %d bytes send!\r\n", send_len);
        }
    } else {
        DHCPD_PRINTF("dhcp dispatch filed, maybe the recv packet is invalid!\r\n");
    }
}

static int dhcpd_dispatch(struct raw_msg *msg, uint16_t *send_pkt_len, uint16_t max_mtu)
{
    dhcp_option_t option = {0};
    int ret = DHCPD_ERR_NONE;

    if(BOOT_REQUEST != ((struct dhcp_packet *)msg->recv_buff)->op) {
        DHCPD_ERR("Packet is not send from dhcp client, ignor!\r\n");
        return DHCPD_ERR_MSG;
    }

    if (LN_TRUE != dhcpd_recv_packet_check((uint8_t*)msg->recv_buff, msg->recv_len)) {
        return DHCPD_ERR_MSG;
    }

    option.code = DHO_DHCP_MESSAGE_TYPE;
    if (LN_TRUE != dhcpd_parse_option((uint8_t*)msg->recv_buff, msg->recv_len, &option)) {
        DHCPD_ERR("No dhcp message type found in the packet!\r\n");
        return DHCPD_ERR_MSG;
    }

    switch(*option.value) {
        case DHCP_DISCOVER:
            ret = do_discover(msg, send_pkt_len, max_mtu);
            break;
        case DHCP_RELEASE:
            ret = do_release(msg, send_pkt_len, max_mtu);
            break;
        case DHCP_INFORM:
            ret = do_inform(msg, send_pkt_len, max_mtu);
            break;
        case DHCP_REQUEST:
            ret = do_request(msg, send_pkt_len, max_mtu);
            break;
        case DHCP_DECLINE:
            ret = do_decline(msg, send_pkt_len, max_mtu);
            break;
        default:
            DHCPD_ERR("DHCP recv message type invalied or non-supported!\r\n");
            return DHCPD_ERR_MSG;
    }

    DHCPD_PRINTF("DHCP msg type =%d\r\n", option.code);
    return ret;
}

int do_discover(struct raw_msg *msg, uint16_t *send_pkt_len, uint16_t max_mtu)
{
    struct dhcp_packet *request  = (struct dhcp_packet *)msg->recv_buff;
    struct dhcp_packet *response = (struct dhcp_packet *)msg->send_buff;
    ip4_addr_t alloc_ipaddr;
    dhcpd_ctrl_t *dhcpd = dhcpd_get_handle();
    server_config_t * server_config = &(dhcpd->server_config);
    uint8_t tmp_len = 0;
    uint8_t *client_mac = &request->chaddr[0];

    if(DHCPD_ERR_NONE != dhcpd_ip_allocate(dhcpd->ip_pool, (char *)client_mac, &alloc_ipaddr, LN_FALSE)) {
        DHCPD_ERR("Cannot assign IP address!\r\n");
        return LN_FALSE;
    }

    //bootp
    uint16_t offset = 0;
    response->op    = BOOT_REPLY;
    response->htype = request->htype;
    response->hlen  = request->hlen;
    response->hops  = 0;
    memcpy(response->xid, request->xid, 4);
    memset(response->secs, 0, 2);
    memcpy(response->flags, request->flags, 2);
    //ciaddr yiaddr siaddr giaddr
    memcpy(response->ciaddr, request->ciaddr, 4);
    memcpy(response->yiaddr, &alloc_ipaddr.addr, 4);
    memset(response->siaddr, 0, 4);
    memset(response->giaddr, 0, 4);
    //chaddr
    memcpy(response->chaddr, request->chaddr, 16);
    //sname, filename
    memset(response->sname, 0, 64);
    strcpy((char*)response->sname, DHCP_SERVER_NAME_STR);
    memset(response->file, 0, 128);
    offset += BOOTP_ABSOLUTE_MIN_LEN;

    //magic cookie
    dhcpd_htonl_copy(response->cookie, DHCP_MAGIC_COOKIE_U32);
    offset += DHCP_MAGIC_COOKIE_LEN;

    //options
    uint8_t * optionptr = (uint8_t * )response;

    //message type
    optionptr[offset++] = DHO_DHCP_MESSAGE_TYPE;
    optionptr[offset++] = 1;
    optionptr[offset++] = DHCP_OFFER;

    //server identifier
    optionptr[offset++] = DHO_DHCP_SERVER_IDENTIFIER;
    optionptr[offset++] = 4;
    memcpy(&optionptr[offset], &server_config->server.addr, 4);
    offset += 4;

    //lease time
    optionptr[offset++] = DHO_DHCP_LEASE_TIME;
    optionptr[offset++] = 4;
    dhcpd_htonl_copy(&optionptr[offset], server_config->lease);//RFC 2132 9.2, time in seconds
    offset += 4;

    //renew time
    optionptr[offset++] = DHO_DHCP_RENEWAL_TIME;
    optionptr[offset++] = 4;
    dhcpd_htonl_copy(&optionptr[offset], (server_config->lease >> 1));//RFC 2132 T1=lease_time*50%
    offset += 4;

    //rebind time
    optionptr[offset++] = DHO_DHCP_REBINDING_TIME;
    optionptr[offset++] = 4;
    dhcpd_htonl_copy(&optionptr[offset], (server_config->lease >> 3)*7);//RFC 2132 T2=lease_time*87.5%
    offset += 4;

    //router/gateway
    optionptr[offset++] = DHO_ROUTERS;
    optionptr[offset++] = 4;
    memcpy(&optionptr[offset], &server_config->server.addr, 4);
    offset += 4;

    //netmask
    ip4_addr_t ip_tmp;
    optionptr[offset++] = DHO_SUBNET;
    optionptr[offset++] = 4;
    ip4addr_aton(DHCP_HOST_NETMASK, &ip_tmp);
    memcpy(&optionptr[offset], &ip_tmp.addr, 4);
    offset += 4;

    //DNS1
    optionptr[offset++] = DHO_DOMAIN_NAME_SERVERS;
    optionptr[offset++] = 4;
    ip4addr_aton(DHCP_HOST_DNS1, &ip_tmp);
    memcpy(&optionptr[offset], &ip_tmp.addr, 4);
    offset += 4;

    //DHCP_HOST_NAME
    tmp_len = strlen(DHCP_HOST_NAME_STR);
    optionptr[offset++] = DHO_HOST_NAME;
    optionptr[offset++] = tmp_len;
    memcpy(&optionptr[offset], DHCP_HOST_NAME_STR, tmp_len);
    offset += tmp_len;

    //END
    optionptr[offset++] = DHO_END;
    optionptr[offset++] = DHO_PAD;
    tmp_len = LN_ROUNDUP(offset, 4) - offset;
    if (tmp_len > 0) {
        memset(&optionptr[offset], DHO_PAD, tmp_len);
    }
    offset += tmp_len;
    *send_pkt_len = offset;

    if(offset > max_mtu) {
        DHCPD_ERR("DHCP DISCORVE reply msg len > dhcp_max_mtu!!!\r\n");
        return LN_FALSE;
    }
    return LN_TRUE;
}

wifi_mode_t ln_get_wifi_mode_from_at(void);
void    ln_at_printf(const char *format, ...);
int do_request(struct raw_msg *msg, uint16_t *send_pkt_len, uint16_t max_mtu)
{
    struct dhcp_packet *request  = (struct dhcp_packet *)msg->recv_buff;
    struct dhcp_packet *response = (struct dhcp_packet *)msg->send_buff;

    uint8_t dhcp_msg_type = DHCP_ACK;

    dhcpd_ctrl_t *dhcpd = dhcpd_get_handle();
    server_config_t * server_config = &(dhcpd->server_config);
    int offset = 0;

    uint8_t tmp_len = 0;

    ip4_addr_t alloc_ipaddr;
    char confirm_ipaddr[4] = {0};
    dhcp_option_t option = {0};
    uint8_t *client_mac = &request->chaddr[0];

    //1. get confirm ip from REQUEST Packet.
    option.value = NULL; /* Must init as NULL */
    option.code = DHO_DHCP_REQUESTED_ADDRESS;
    if (LN_TRUE == dhcpd_parse_option((uint8_t *)request, msg->recv_len, &option)) {
        memcpy(confirm_ipaddr, option.value, 4);
        DHCPD_ERR("REQUEST: Confirm ip(DHO_DHCP_REQUESTED_ADDRESS)=\"%d.%d.%d.%d.\"\r\n", \
                        option.value[0], option.value[1], option.value[2], option.value[3]);
		
		if (ln_get_wifi_mode_from_at() == WIFI_MODE_AP)
		{
			ln_at_printf("+DIST_STA_IP:\"%02x:%02x:%02x:%02x:%02x:%02x\",\"%d.%d.%d.%d\"", 
						client_mac[0], client_mac[1], client_mac[2], client_mac[3], client_mac[4], client_mac[5],
						option.value[0], option.value[1], option.value[2], option.value[3]);
		}
    } else {
        if(0 != memcmp("\x00\x00\x00\x00", request->ciaddr, 4)) {
            memcpy(confirm_ipaddr, request->ciaddr, 4);
            DHCPD_ERR("REQUEST: No DHO_DHCP_REQUESTED_ADDRESS option!\r\n");
            DHCPD_ERR("REQUEST: Confirm ip(BOOTP ciaddr)=\"%d.%d.%d.%d.\"\r\n", \
                        request->ciaddr[0], request->ciaddr[1], request->ciaddr[2], request->ciaddr[3]);
        } else {
            DHCPD_ERR("REQUEST: REQUEST packet no confirm ip!!!\r\n");
            return LN_FALSE;
        }
    }

    //2. lookup the allocate IP by OFFER phase.
    if(DHCPD_ERR_NONE != dhcpd_ip_allocate(dhcpd->ip_pool, (char *)client_mac, &alloc_ipaddr, LN_TRUE)) {
        DHCPD_ERR("Cannot assign IP address!\r\n");
        return LN_FALSE;
    }

    // compare it
    if(0 != memcmp(&(alloc_ipaddr), confirm_ipaddr, 4)) {
        DHCPD_ERR("REQUEST: Confirm ip is not the allocate IP by OFFER phase,send NAK msg to client!\r\n");
        dhcp_msg_type = DHCP_NAK;
        memset(confirm_ipaddr, 0, 4);
        dhcpd_ip_release((char *)client_mac);
    }

    //bootp
    response->op    = BOOT_REPLY;
    response->htype = request->htype;
    response->hlen  = request->hlen;
    response->hops  = 0;
    memcpy(response->xid, request->xid, 4);
    memset(response->secs, 0, 2);
    memcpy(response->flags, request->flags, 2);
    //ciaddr yiaddr siaddr giaddr
    memcpy(response->ciaddr, request->ciaddr, 4);
    memcpy(response->yiaddr, confirm_ipaddr, 4);
    memset(response->siaddr, 0, 4);
    memset(response->giaddr, 0, 4);
    //chaddr
    memcpy(response->chaddr, request->chaddr, 16);
    //sname, filename
    memset(response->sname, 0, 64);
    strcpy((char*)response->sname, DHCP_SERVER_NAME_STR);
    memset(response->file, 0, 128);
    offset += BOOTP_ABSOLUTE_MIN_LEN;

    //magic cookie
    dhcpd_htonl_copy(response->cookie, DHCP_MAGIC_COOKIE_U32);
    offset += DHCP_MAGIC_COOKIE_LEN;

    //options
    uint8_t * optionptr = (uint8_t * )response;

    //message type
    optionptr[offset++] = DHO_DHCP_MESSAGE_TYPE;
    optionptr[offset++] = 1;
    optionptr[offset++] = dhcp_msg_type;

    //server identifier
    optionptr[offset++] = DHO_DHCP_SERVER_IDENTIFIER;
    optionptr[offset++] = 4;
    memcpy(&optionptr[offset], &server_config->server.addr, 4);
    offset += 4;

    if (DHCP_ACK == dhcp_msg_type) {
        //lease time
        optionptr[offset++] = DHO_DHCP_LEASE_TIME;
        optionptr[offset++] = 4;
        dhcpd_htonl_copy(&optionptr[offset], server_config->lease);//RFC 2132 9.2, time in seconds
        offset += 4;

        //renew time
        optionptr[offset++] = DHO_DHCP_RENEWAL_TIME;
        optionptr[offset++] = 4;
        dhcpd_htonl_copy(&optionptr[offset], (server_config->lease >> 1));//RFC 2132 T1=lease_time*50%
        offset += 4;

        //rebind time
        optionptr[offset++] = DHO_DHCP_REBINDING_TIME;
        optionptr[offset++] = 4;
        dhcpd_htonl_copy(&optionptr[offset], (server_config->lease >> 3)*7);//RFC 2132 T2=lease_time*87.5%
        offset += 4;

        //router/gateway
        optionptr[offset++] = DHO_ROUTERS;
        optionptr[offset++] = 4;
        memcpy(&optionptr[offset], &server_config->server.addr, 4);
        offset += 4;

        //netmask
        ip4_addr_t ip_tmp;
        optionptr[offset++] = DHO_SUBNET;
        optionptr[offset++] = 4;
        ip4addr_aton(DHCP_HOST_NETMASK, &ip_tmp);
        memcpy(&optionptr[offset], &ip_tmp.addr, 4);
        offset += 4;

        //DNS1
        optionptr[offset++] = DHO_DOMAIN_NAME_SERVERS;
        optionptr[offset++] = 4;
        ip4addr_aton(DHCP_HOST_DNS1, &ip_tmp);
        memcpy(&optionptr[offset], &ip_tmp.addr, 4);
        offset += 4;
    }

    //DHCP_HOST_NAME
    tmp_len = strlen(DHCP_HOST_NAME_STR);
    optionptr[offset++] = DHO_HOST_NAME;
    optionptr[offset++] = tmp_len;
    memcpy(&optionptr[offset], DHCP_HOST_NAME_STR, tmp_len);
    offset += tmp_len;

    //END
    optionptr[offset++] = DHO_END;
    optionptr[offset++] = DHO_PAD;
    tmp_len = LN_ROUNDUP(offset, 4) - offset;
    if (tmp_len > 0) {
        memset(&optionptr[offset], DHO_PAD, tmp_len);
    }
    offset += tmp_len;
    *send_pkt_len = offset;

    if(offset > max_mtu) {
        DHCPD_ERR("DHCP REQUEST reply msg len > dhcp_max_mtu!!!\r\n");
        return LN_FALSE;
    }
    return LN_TRUE;
}

int do_inform(struct raw_msg *msg, uint16_t *send_pkt_len, uint16_t max_mtu)
{
    struct dhcp_packet *request  = (struct dhcp_packet *)msg->recv_buff;
    struct dhcp_packet *response = (struct dhcp_packet *)msg->send_buff;

    dhcpd_ctrl_t *dhcpd = dhcpd_get_handle();
    server_config_t * server_config = &(dhcpd->server_config);
    int offset = 0;
    uint8_t tmp_len = 0;

    ip4_addr_t alloc_ipaddr;
    uint8_t *client_mac = request->chaddr;

    if(DHCPD_ERR_NONE != dhcpd_ip_allocate(dhcpd->ip_pool, (char *)client_mac, &alloc_ipaddr, LN_FALSE)) {
        DHCPD_ERR("Cannot assign IP address!\r\n");
        return LN_FALSE;
    }

    //bootp
    response->op    = BOOT_REPLY;
    response->htype = request->htype;
    response->hlen  = request->hlen;
    response->hops  = 0;
    memcpy(response->xid, request->xid, 4);
    memset(response->secs, 0, 2);
    memcpy(response->flags, request->flags, 2);
    //ciaddr yiaddr siaddr giaddr
    memcpy(response->ciaddr, request->ciaddr, 4);
    memcpy(response->yiaddr, &alloc_ipaddr.addr, 4);
    memset(response->siaddr, 0, 4);
    memset(response->giaddr, 0, 4);
    //chaddr
    memcpy(response->chaddr, request->chaddr, 16);
    //sname, filename
    memset(response->sname, 0, 64);
    strcpy((char*)response->sname, DHCP_SERVER_NAME_STR);
    memset(response->file, 0, 128);
    offset += BOOTP_ABSOLUTE_MIN_LEN;

    //magic cookie
    dhcpd_htonl_copy(response->cookie, DHCP_MAGIC_COOKIE_U32);
    offset += DHCP_MAGIC_COOKIE_LEN;

    //options
    uint8_t * optionptr = (uint8_t * )response;

    //message type
    optionptr[offset++] = DHO_DHCP_MESSAGE_TYPE;
    optionptr[offset++] = 1;
    optionptr[offset++] = DHCP_ACK;

    //server identifier
    optionptr[offset++] = DHO_DHCP_SERVER_IDENTIFIER;
    optionptr[offset++] = 4;
    memcpy(&optionptr[offset], &server_config->server.addr, 4);
    offset += 4;

    //lease time
    optionptr[offset++] = DHO_DHCP_LEASE_TIME;
    optionptr[offset++] = 4;
    dhcpd_htonl_copy(&optionptr[offset], server_config->lease);//RFC 2132 9.2, time in seconds
    offset += 4;

    //renew time
    optionptr[offset++] = DHO_DHCP_RENEWAL_TIME;
    optionptr[offset++] = 4;
    dhcpd_htonl_copy(&optionptr[offset], (server_config->lease >> 1));//RFC 2132 T1=lease_time*50%
    offset += 4;

    //rebind time
    optionptr[offset++] = DHO_DHCP_REBINDING_TIME;
    optionptr[offset++] = 4;
    dhcpd_htonl_copy(&optionptr[offset], (server_config->lease >> 3)*7);//RFC 2132 T2=lease_time*87.5%
    offset += 4;

    //router/gateway
    optionptr[offset++] = DHO_ROUTERS;
    optionptr[offset++] = 4;
    memcpy(&optionptr[offset], &server_config->server.addr, 4);
    offset += 4;

    //netmask
    ip4_addr_t ip_tmp;
    optionptr[offset++] = DHO_SUBNET;
    optionptr[offset++] = 4;
    ip4addr_aton(DHCP_HOST_NETMASK, &ip_tmp);
    memcpy(&optionptr[offset], &ip_tmp.addr, 4);
    offset += 4;

    //DNS1
    optionptr[offset++] = DHO_DOMAIN_NAME_SERVERS;
    optionptr[offset++] = 4;
    ip4addr_aton(DHCP_HOST_DNS1, &ip_tmp);
    memcpy(&optionptr[offset], &ip_tmp.addr, 4);
    offset += 4;

    //DHCP_HOST_NAME
    tmp_len = strlen(DHCP_HOST_NAME_STR);
    optionptr[offset++] = DHO_HOST_NAME;
    optionptr[offset++] = tmp_len;
    memcpy(&optionptr[offset], DHCP_HOST_NAME_STR, tmp_len);
    offset += tmp_len;

    //END
    optionptr[offset++] = DHO_END;
    optionptr[offset++] = DHO_PAD;
    tmp_len = LN_ROUNDUP(offset, 4) - offset;
    if (tmp_len > 0) {
        memset(&optionptr[offset], DHO_PAD, tmp_len);
    }
    offset += tmp_len;
    *send_pkt_len = offset;

    if(offset > max_mtu) {
        DHCPD_ERR("DHCP INFORM reply msg len > dhcp_max_mtu!!!\r\n");
        return LN_FALSE;
    }
    return LN_TRUE;
}

int do_release(struct raw_msg *msg, uint16_t *send_pkt_len, uint16_t max_mtu) {
    LN_UNUSED(msg);
    LN_UNUSED(send_pkt_len);
    LN_UNUSED(max_mtu);

    DHCPD_PRINTF("Support [RELEASE msg] in the future!\r\n");
    return LN_FALSE;//Don't reply.
}

int do_decline(struct raw_msg *msg, uint16_t *send_pkt_len, uint16_t max_mtu) {
    LN_UNUSED(msg);
    LN_UNUSED(send_pkt_len);
    LN_UNUSED(max_mtu);

    DHCPD_PRINTF("Support [DECLINE msg] in the future!\r\n");
    return LN_FALSE;//Don't reply.
}

int dhcpd_curr_config_set(server_config_t *server_config)
{
    dhcpd_ctrl_t *dhcpd = dhcpd_get_handle();
    if (dhcpd->flag == DHCPD_TASK_IS_RUNNING) {
        return DHCPD_ERR_STATE;
    }
    memcpy(&(dhcpd->server_config), server_config, sizeof(server_config_t));
    return DHCPD_ERR_NONE;
}

int dhcpd_curr_config_get(server_config_t *server_config)
{
    dhcpd_ctrl_t *dhcpd = dhcpd_get_handle();
    memcpy(server_config, &(dhcpd->server_config), sizeof(server_config_t));
    return DHCPD_ERR_NONE;
}


