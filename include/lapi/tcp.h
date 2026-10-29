// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 */

#ifndef LAPI_TCP_H__
#define LAPI_TCP_H__

#include <netinet/tcp.h>

#ifndef TCP_FASTOPEN
# define TCP_FASTOPEN	23
#endif

#ifndef TCP_ULP
# define TCP_ULP          31
#endif

#ifndef TCP_FASTOPEN_CONNECT
# define TCP_FASTOPEN_CONNECT	30	/* Attempt FastOpen with connect */
#endif

#endif	/* LAPI_TCP_H__ */
