// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef LAPI_PKT_SCHED_H__
#define LAPI_PKT_SCHED_H__

#include <linux/pkt_sched.h>

#ifndef TC_H_CLSACT
# define TC_H_CLSACT	TC_H_INGRESS
#endif

#ifndef TC_H_MIN_EGRESS
# define TC_H_MIN_EGRESS	0xFFF3U
#endif

#endif /* LAPI_PKT_SCHED_H__ */
