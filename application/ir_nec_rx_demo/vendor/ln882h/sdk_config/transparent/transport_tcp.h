// Copyright 2015-2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _LN_TRANSPORT_TCP_H_
#define _LN_TRANSPORT_TCP_H_

#include "transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      Create TCP transport, the transport handle must be release ln_transport_destroy callback
 *
 * @return  the allocated ln_transport_handle_t, or NULL if the handle can not be allocated
 */
extern ln_transport_handle_t ln_transport_tcp_init(void);
extern int ln_transport_tcp_bind_socket(ln_transport_handle_t t, int sock);


#ifdef __cplusplus
}
#endif

#endif /* _LN_TRANSPORT_TCP_H_ */
