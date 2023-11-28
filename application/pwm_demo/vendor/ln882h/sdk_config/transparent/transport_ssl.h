#ifndef _TRANSPORT_SSL_H_
#define _TRANSPORT_SSL_H_

#include "netif/ethernetif.h"
#include "utils/debug/log.h"
#include "utils/debug/ln_assert.h"

#include "proj_config.h"
//#include "atcmd/at_parser.h"
#include "lwip/sockets.h"


#include "flash_partition_table.h"
#include <time.h>

enum
{
	TYPE_PK,
	TYPE_CERT,
	TYPE_CA
};

#define CERT_CA_LEN_THLD			2040
#define CERT_BLOCK_SIZE 			2048
#define CERT_TOTAL_SIZE				(CERT_BLOCK_SIZE * 3)
#define CERT_SSL_CA_PK_START 		USER_SPACE_OFFSET



extern int ssl_rece(unsigned char* _buf, size_t _len);
extern int ssl_send(const unsigned char* _buf, size_t _len);
extern int transp_create_ssl_link(int id);
extern void transp_delete_ssl_link(int id);

#endif
