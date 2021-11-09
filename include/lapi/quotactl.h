// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2021 FUJITSU LIMITED. All rights reserved
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

#ifndef LAPI_QUOTACTL_H__
#define LAPI_QUOTACTL_H__

#include "config.h"
#include <sys/quota.h>
#include "lapi/syscalls.h"

#ifndef HAVE_QUOTACTL_FD
static inline int quotactl_fd(int fd, int cmd, int id, caddr_t addr)
{
	return tst_syscall(__NR_quotactl_fd, fd, cmd, id, addr);
}
#endif

#ifdef HAVE_STRUCT_IF_NEXTDQBLK
# include <linux/quota.h>
#else
# include <stdint.h>
struct if_nextdqblk {
	uint64_t	dqb_bhardlimit;
	uint64_t	dqb_bsoftlimit;
	uint64_t	dqb_curspace;
	uint64_t	dqb_ihardlimit;
	uint64_t	dqb_isoftlimit;
	uint64_t	dqb_curinodes;
	uint64_t	dqb_btime;
	uint64_t	dqb_itime;
	uint32_t	dqb_valid;
	uint32_t	dqb_id;
};
#endif /* HAVE_STRUCT_IF_NEXTDQBLK */

#ifndef HAVE_STRUCT_FS_QUOTA_STATV
# include <stdint.h>
struct fs_qfilestatv {
	uint64_t	qfs_ino;
	uint64_t	qfs_nblks;
	uint32_t	qfs_nextents;
	uint32_t	qfs_pad;
};

struct fs_quota_statv {
	int8_t			qs_version;
	uint8_t			qs_pad1;
	uint16_t		qs_flags;
	uint32_t		qs_incoredqs;
	struct fs_qfilestatv	qs_uquota;
	struct fs_qfilestatv	qs_gquota;
	struct fs_qfilestatv	qs_pquota;
	int32_t			qs_btimelimit;
	int32_t			qs_itimelimit;
	int32_t			qs_rtbtimelimit;
	uint16_t		qs_bwarnlimit;
	uint16_t		qs_iwarnlimit;
	uint64_t		qs_pad2[8];
};
# define FS_QSTATV_VERSION1 1
#endif /* HAVE_STRUCT_FS_QUOTA_STATV */

#ifndef PRJQUOTA
# define PRJQUOTA 2
#endif

#ifndef Q_XQUOTARM
# define Q_XQUOTARM XQM_CMD(6)
#endif

#ifndef Q_XGETQSTATV
# define Q_XGETQSTATV XQM_CMD(8)
#endif

#ifndef Q_XGETNEXTQUOTA
# define Q_XGETNEXTQUOTA XQM_CMD(9)
#endif

#ifndef Q_GETNEXTQUOTA
# define Q_GETNEXTQUOTA 0x800009 /* get disk limits and usage >= ID */
#endif

#ifndef QFMT_VFS_V0
# define QFMT_VFS_V0 2
#endif

#ifndef QFMT_VFS_V1
# define QFMT_VFS_V1 4
#endif

#endif /* LAPI_QUOTACTL_H__ */
