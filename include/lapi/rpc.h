// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
 */

#ifndef LAPI_RPC_H__
#define LAPI_RPC_H__

#include "config.h"

#ifdef HAVE_NETCONFIG_H
# include <netconfig.h>
# include <rpc/rpc.h>
# include <rpc/types.h>
# include <rpc/xdr.h>
# include <rpc/svc.h>
#elif defined(HAVE_TIRPC_NETCONFIG_H)
# include <tirpc/netconfig.h>
# include <tirpc/rpc/rpc.h>
# include <tirpc/rpc/types.h>
# include <tirpc/rpc/xdr.h>
# include <tirpc/rpc/svc.h>
#else
# error Missing rpc headers!
#endif

#endif	/* LAPI_RPC_H__ */
