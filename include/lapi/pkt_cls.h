// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef LAPI_PKT_CLS_H__
#define LAPI_PKT_CLS_H__

#include <linux/pkt_cls.h>

#ifndef TC_ACT_PIPE
# define TC_ACT_PIPE	3
#endif

#ifndef TCA_MATCHALL_ACT
# define TCA_MATCHALL_ACT	2
#endif

#endif /* LAPI_PKT_CLS_H__ */
