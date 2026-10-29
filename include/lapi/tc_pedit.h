// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 SUSE LLC <andrea.cervesato@suse.com>
 */

#ifndef LAPI_TC_PEDIT_H__
#define LAPI_TC_PEDIT_H__

#include "config.h"
#include <linux/tc_act/tc_pedit.h>

#if !HAVE_DECL_TCA_PEDIT_PARMS_EX
# define TCA_PEDIT_PARMS_EX		4
#endif

#if !HAVE_DECL_TCA_PEDIT_KEYS_EX
# define TCA_PEDIT_KEYS_EX		5
#endif

#if !HAVE_DECL_TCA_PEDIT_KEY_EX
# define TCA_PEDIT_KEY_EX		6
#endif

#if !HAVE_DECL_TCA_PEDIT_KEY_EX_HTYPE
# define TCA_PEDIT_KEY_EX_HTYPE		1
#endif

#if !HAVE_DECL_TCA_PEDIT_KEY_EX_CMD
# define TCA_PEDIT_KEY_EX_CMD		2
#endif

#ifndef HAVE_ENUM_PEDIT_HEADER_TYPE
enum pedit_header_type {
	TCA_PEDIT_KEY_EX_HDR_TYPE_NETWORK = 0,
	TCA_PEDIT_KEY_EX_HDR_TYPE_ETH,
	TCA_PEDIT_KEY_EX_HDR_TYPE_IP4,
	TCA_PEDIT_KEY_EX_HDR_TYPE_IP6,
	TCA_PEDIT_KEY_EX_HDR_TYPE_TCP,
	TCA_PEDIT_KEY_EX_HDR_TYPE_UDP,
	__PEDIT_HDR_TYPE_MAX,
};
#endif

#ifndef HAVE_ENUM_PEDIT_CMD
enum pedit_cmd {
	TCA_PEDIT_KEY_EX_CMD_SET = 0,
	TCA_PEDIT_KEY_EX_CMD_ADD,
	__PEDIT_CMD_MAX,
};
#endif

#endif /* LAPI_TC_PEDIT_H__ */
