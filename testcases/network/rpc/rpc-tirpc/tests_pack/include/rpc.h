// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018-2020 Petr Vorel <pvorel@suse.cz>
 */

#ifndef RPC_H__
#define RPC_H__

#include "config.h"

#include <rpc/rpc.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/svc.h>

/*
 * For pmap_unset() and clnt_broadcast().
 * Needed for glibc, which does not include <rpc/pmap_clnt.h> in <rpc/rpc.h>.
 */
#include <rpc/pmap_clnt.h>

#endif	/* RPC_H__ */
