// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2019 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 * Author: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 */

#ifndef LAPI_QUOTACTL_H__
#define LAPI_QUOTACTL_H__

#include <sys/quota.h>

#ifdef HAVE_STRUCT_IF_NEXTDQBLK
# include <linux/quota.h>
#else
# ifdef HAVE_LINUX_TYPES_H
# include <linux/types.h>
struct if_nextdqblk {
	__u64 dqb_bhardlimit;
	__u64 dqb_bsoftlimit;
	__u64 dqb_curspace;
	__u64 dqb_ihardlimit;
	__u64 dqb_isoftlimit;
	__u64 dqb_curinodes;
	__u64 dqb_btime;
	__u64 dqb_itime;
	__u32 dqb_valid;
	__u32 dqb_id;
};
#endif
#endif /* HAVE_STRUCT_IF_NEXTDQBLK */

#ifndef Q_XGETNEXTQUOTA
# define Q_XGETNEXTQUOTA XQM_CMD(9)
#endif

#ifndef Q_GETNEXTQUOTA
# define Q_GETNEXTQUOTA 0x800009 /* get disk limits and usage >= ID */
#endif

#endif /* LAPI_QUOTACTL_H__ */
