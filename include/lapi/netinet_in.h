// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 */

#ifndef LAPI_IN_H__
#define LAPI_IN_H__

#include <netinet/in.h>

#ifndef IPPROTO_DCCP
#define IPPROTO_DCCP		33
#endif

#ifndef IPPROTO_UDPLITE
# define IPPROTO_UDPLITE	136 /* UDP-Lite (RFC 3828) */
#endif

#ifndef IP_BIND_ADDRESS_NO_PORT
# define IP_BIND_ADDRESS_NO_PORT	24
#endif

#endif	/* LAPI_IN_H__ */
