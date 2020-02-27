// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Oracle and/or its affiliates.
 */

#ifndef LAPI_SCTP_H__
#define LAPI_SCTP_H__

#ifdef HAVE_NETINET_SCTP_H
# include <netinet/sctp.h>
#endif

#ifndef SCTP_SOCKOPT_BINDX_ADD
# define SCTP_SOCKOPT_BINDX_ADD	100
#endif

#endif	/* LAPI_SCTP_H__ */
