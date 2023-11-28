#include "dhcp_packet.h"
#include "dhcpd.h"


void dhcpd_packet_printf(struct dhcp_packet * p)
{
    DHCPD_PRINTF("| op    =%d\r\n",              p->op);
    DHCPD_PRINTF("| htype =%d\r\n",              p->htype);
    DHCPD_PRINTF("| hlen  =%d\r\n",              p->hlen);
    DHCPD_PRINTF("| hops  =%d\r\n",              p->hops);
    DHCPD_PRINTF("| xid   =%2x,%2x,%2x,%2x\r\n", p->xid[0], p->xid[1], p->xid[2], p->xid[3]);
    DHCPD_PRINTF("| secs  =%2x,%2x\r\n",         p->secs[0], p->secs[1]);
    DHCPD_PRINTF("| flags =%2x,%2x\r\n",         p->flags[0],  p->flags[1]);
    DHCPD_PRINTF("| ciaddr=\"%d.%d.%d.%d\"\r\n", p->ciaddr[0], p->ciaddr[1], p->ciaddr[2], p->ciaddr[3]);
    DHCPD_PRINTF("| yiaddr=\"%d.%d.%d.%d\"\r\n", p->yiaddr[0], p->yiaddr[1], p->yiaddr[2], p->yiaddr[3]);
    DHCPD_PRINTF("| siaddr=\"%d.%d.%d.%d\"\r\n", p->siaddr[0], p->siaddr[1], p->siaddr[2], p->siaddr[3]);
    DHCPD_PRINTF("| giaddr=\"%d.%d.%d.%d\"\r\n", p->giaddr[0], p->giaddr[1], p->giaddr[2], p->giaddr[3]);
    DHCPD_PRINTF("| chaddr=0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x\r\n", \
        p->chaddr[0], p->chaddr[1], p->chaddr[2],  p->chaddr[3],  p->chaddr[4],  p->chaddr[5],  p->chaddr[6],  p->chaddr[7],
        p->chaddr[8], p->chaddr[9], p->chaddr[10], p->chaddr[11], p->chaddr[12], p->chaddr[13], p->chaddr[14], p->chaddr[15]);
    DHCPD_PRINTF("| sname =%s\r\n", p->sname);
    DHCPD_PRINTF("| file  =%s\r\n", p->file);
}

int dhcpd_parse_option(uint8_t *packet, uint16_t packet_len, dhcp_option_t *option)
{
    struct dhcp_packet *p = (struct dhcp_packet *)packet;
    uint32_t magic_cookie = lwip_htonl(DHCP_MAGIC_COOKIE_U32);

    //magic cookie check
    if (0 != memcmp(&magic_cookie, p->cookie, DHCP_MAGIC_COOKIE_LEN)) {
        DHCPD_ERR("DHCP packet magic cookie error!\r\n");
        return LN_FALSE;
    }

    // option format check
    #define FIELD_FLAG_OPTION    (0)
    #define FIELD_FLAG_FILE      (1)
    #define FIELD_FLAG_SNAME     (2)
    #define OVERLOAD_BIT_FILE    (1U << 0)
    #define OVERLOAD_BIT_SNAME   (1U << 1)

    int8_t iterate_done = LN_FALSE;
    uint8_t overload    = 0;
    uint8_t curr_flag   = FIELD_FLAG_OPTION;

    uint8_t *optionptr  = packet;
    uint16_t len        = packet_len;
    int offset          = DHCP_OPTS_POS;

    while (!iterate_done)
    {
        if (offset >= len) {
            return LN_FALSE;
        }

        if (option != NULL)
        {
            if (optionptr[offset] == option->code) {
                option->len = optionptr[offset + OPT_LEN_OFS];
                if ((offset + 2 + option->len) >= len) {
                    return LN_FALSE;
                }
                option->value = &optionptr[offset + OPT_VAL_OFS];
                return LN_TRUE;
            }
        }

        switch (optionptr[offset])
        {
            case DHO_PAD:
                offset++;
                break;
            case DHO_DHCP_OPTION_OVERLOAD:
                if (offset + 2 + optionptr[offset + OPT_LEN_OFS] >= len) {
                    return LN_FALSE;
                }
                overload = optionptr[offset + OPT_VAL_OFS];
                offset += (2 + optionptr[offset + OPT_LEN_OFS]);
                break;
            case DHO_END:
                if ((curr_flag == FIELD_FLAG_OPTION) && (overload & OVERLOAD_BIT_FILE)) {
                    optionptr = p->file;
                    offset = DHCP_FILE_POS;
                    len = DHCP_FILE_POS + DHCP_FILE_LEN;
                    curr_flag = FIELD_FLAG_FILE;
                } else if ((curr_flag == FIELD_FLAG_OPTION) && (overload & OVERLOAD_BIT_SNAME)) {
                    optionptr = p->sname;
                    offset = DHCP_SNAME_POS;
                    len = DHCP_SNAME_POS + DHCP_SNAME_LEN;
                    curr_flag = FIELD_FLAG_SNAME;
                } else {
                    iterate_done = LN_TRUE;
                }
                break;
            default:
                offset += (2 + optionptr[offset + OPT_LEN_OFS]);
                break;
         }
    }

    if (option->value == NULL) {
        return LN_FALSE;
    }
    return LN_TRUE;
}

int dhcpd_recv_packet_check(uint8_t *packet, uint16_t len)
{
    int ret = LN_FALSE;
    if (len < (BOOTP_ABSOLUTE_MIN_LEN + DHCP_MAGIC_COOKIE_LEN + 4)) {
        DHCPD_ERR("Recv dhcp packet length is less than min size.\r\n");
        return ret;
    }

    DHCPD_PRINTF("\r\n+------Dump BOOTP (Recv from Client)-----\r\n");
    dhcpd_packet_printf((struct dhcp_packet *)packet);
    DHCPD_PRINTF("+--------------------------------------------\r\n\r\n");

    if (LN_TRUE != (ret = dhcpd_parse_option(packet, len, NULL))) {
        DHCPD_ERR("Recv dhcp packet check failed.\r\n");
    }
    return ret;
}
