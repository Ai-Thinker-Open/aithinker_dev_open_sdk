## Introduction
This Mbed TLS layer is ported from the open source Mbed TLS, and we only modified the needed parts
to adapt to LN-SEMI Things, so this layer provides the same APIs and features as the open source
Mbed TLS. For details, please refer to https://tls.mbed.org/ .

The major modification for porting Mbed TLS includes:
- provides our network socket APIs in port_ln/library/net_sockets.c
- provides our threading mutex APIs in port_ln/library/threading_alt.c
- provides our memory management APIs in port_ln/library/platform.c
- provides our configuration in port_ln/include/mbedtls_config.h

## Dependencies
N/A

