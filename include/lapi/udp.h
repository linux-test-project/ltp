// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Oracle and/or its affiliates.
 */

#ifndef LAPI_UDP_H__
#define LAPI_UDP_H__

#include <netinet/udp.h>

#ifndef UDPLITE_SEND_CSCOV
# define UDPLITE_SEND_CSCOV   10 /* sender partial coverage (as sent) */
#endif
#ifndef UDPLITE_RECV_CSCOV
# define UDPLITE_RECV_CSCOV   11 /* receiver partial coverage (threshold ) */
#endif

#ifndef UDP_ENCAP
# define UDP_ENCAP 100
#endif

#ifndef UDP_ENCAP_ESPINUDP
# define UDP_ENCAP_ESPINUDP 2
#endif

#ifndef TCP_ENCAP_ESPINTCP
# define TCP_ENCAP_ESPINTCP 7
#endif

#endif	/* LAPI_UDP_H__ */
