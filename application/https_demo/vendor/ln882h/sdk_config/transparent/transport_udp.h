#ifndef _TRANSPORT_UDP_H_
#define _TRANSPORT_UDP_H_

#include "transport.h"

extern ln_transport_handle_t ln_transport_udp_init(void);
extern int ln_transport_udp_bind_socket(ln_transport_handle_t t, int sock);


#endif

