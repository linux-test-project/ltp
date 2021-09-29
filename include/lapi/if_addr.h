// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Petr Vorel <petr.vorel@gmail.com>
 */

#ifndef LAPI_IF_ADDR_H__
#define LAPI_IF_ADDR_H__

#include <linux/if_addr.h>

#ifndef IFA_FLAGS
# define IFA_FLAGS 8
#endif

#ifndef IFA_F_NOPREFIXROUTE
# define IFA_F_NOPREFIXROUTE	0x200
#endif

#endif /* LAPI_IF_ADDR_H__ */
