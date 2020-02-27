// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 */

#ifndef LAPI_DCCP_H__
#define LAPI_DCCP_H__

#ifdef HAVE_LINUX_DCCP_H
# include <linux/dccp.h>
#endif

#ifndef DCCP_SOCKOPT_SERVICE
# define DCCP_SOCKOPT_SERVICE	2
#endif

#endif	/* LAPI_DCCP_H__ */
