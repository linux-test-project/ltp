// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
 */

#ifndef LAPI_IF_ETHER_H__
#define LAPI_IF_ETHER_H__

#include "config.h"

#ifdef HAVE_LINUX_IF_ETHER_H
# include <linux/if_ether.h>
#endif

#ifndef ETH_P_ALL
# define ETH_P_ALL 0x0003
#endif

#endif /* LAPI_IF_ETHER_H__ */
