/*
 * Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
