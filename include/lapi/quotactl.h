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

#ifndef HAVE_STRUCT_FS_QUOTA_STATV
# ifdef HAVE_LINUX_TYPES_H
#  include <linux/types.h>
struct fs_qfilestatv {
	__u64           qfs_ino;
	__u64           qfs_nblks;
	__u32           qfs_nextents;
	__u32           qfs_pad;
};

struct fs_quota_statv {
	__s8                    qs_version;
	__u8                    qs_pad1;
	__u16                   qs_flags;
	__u32                   qs_incoredqs;
	struct fs_qfilestatv    qs_uquota;
	struct fs_qfilestatv    qs_gquota;
	struct fs_qfilestatv    qs_pquota;
	__s32                   qs_btimelimit;
	__s32                   qs_itimelimit;
	__s32                   qs_rtbtimelimit;
	__u16                   qs_bwarnlimit;
	__u16                   qs_iwarnlimit;
	__u64                   qs_pad2[8];
};
#  define FS_QSTATV_VERSION1 1
# endif /* HAVE_LINUX_TYPES_H */
#endif /* HAVE_STRUCT_FS_QUOTA_STATV */

#ifndef Q_XGETQSTATV
# define Q_XGETQSTATV XQM_CMD(8)
#endif

#ifndef Q_XGETNEXTQUOTA
# define Q_XGETNEXTQUOTA XQM_CMD(9)
#endif

#ifndef Q_GETNEXTQUOTA
# define Q_GETNEXTQUOTA 0x800009 /* get disk limits and usage >= ID */
#endif

#endif /* LAPI_QUOTACTL_H__ */
