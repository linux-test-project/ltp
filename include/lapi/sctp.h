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

#ifndef IPPROTO_SCTP
# define IPPROTO_SCTP	132
#endif

#ifndef SCTP_PEER_ADDR_PARAMS
# define SCTP_PEER_ADDR_PARAMS	9
#endif

#ifndef SCTP_STATUS
# define SCTP_STATUS	14
#endif

#ifndef SCTP_GET_PEER_ADDR_INFO
# define SCTP_GET_PEER_ADDR_INFO	15
#endif

#ifndef SPP_HB_ENABLE
# define SPP_HB_ENABLE	(1 << 0)
#endif

#ifndef SPP_HB_DISABLE
# define SPP_HB_DISABLE	(1 << 1)
#endif

#ifndef SPP_HB_DEMAND
# define SPP_HB_DEMAND	(1 << 2)
#endif

#ifndef SCTP_ACTIVE
# define SCTP_ACTIVE	2
#endif

/* SCTP wire format values, see include/linux/sctp.h in the kernel */
#ifndef SCTP_CID_INIT
# define SCTP_CID_INIT		1
#endif

#ifndef SCTP_CID_INIT_ACK
# define SCTP_CID_INIT_ACK	2
#endif

#ifndef SCTP_CID_ASCONF_ACK
# define SCTP_CID_ASCONF_ACK	0x80
#endif

#ifndef SCTP_CID_ASCONF
# define SCTP_CID_ASCONF		0xc1
#endif

#ifndef SCTP_PARAM_IPV4_ADDRESS
# define SCTP_PARAM_IPV4_ADDRESS	5
#endif

#ifndef SCTP_PARAM_DEL_IP
# define SCTP_PARAM_DEL_IP	0xc002
#endif

#ifndef SCTP_PARAM_ERR_CAUSE
# define SCTP_PARAM_ERR_CAUSE	0xc003
#endif

#ifndef SCTP_ERROR_REQ_REFUSED
# define SCTP_ERROR_REQ_REFUSED	0x00a4
#endif

#endif	/* LAPI_SCTP_H__ */
