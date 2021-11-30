//SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Referred from linux kernel -github/torvalds/linux/include/uapi/linux/fcntl.h
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 * Email: code@zilogic.com
 */

#ifndef LAPI_STAT_H__
#define LAPI_STAT_H__

#include <stdint.h>
#include <unistd.h>
#include "lapi/syscalls.h"
/*
 * Timestamp structure for the timestamps in struct statx.
 *
 * tv_sec holds the number of seconds before (negative) or after (positive)
 * 00:00:00 1st January 1970 UTC.
 *
 * tv_nsec holds a number of nanoseconds (0..999,999,999) after the tv_sec time.
 *
 * __reserved is held in case we need a yet finer resolution.
 */
#if defined(HAVE_STRUCT_STATX_TIMESTAMP)
#include <sys/stat.h>
#else
struct statx_timestamp {
	int64_t tv_sec;
	uint32_t tv_nsec;
	int32_t __reserved;
};
#endif
/*
 * Structures for the extended file attribute retrieval system call
 * (statx()).
 *
 * The caller passes a mask of what they're specifically interested in as a
 * parameter to statx().  What statx() actually got will be indicated in
 * st_mask upon return.
 *
 * For each bit in the mask argument:
 *
 * - if the datum is not supported:
 *
 *   - the bit will be cleared, and
 *
 *   - the datum will be set to an appropriate fabricated value if one is
 *     available (eg. CIFS can take a default uid and gid), otherwise
 *
 *   - the field will be cleared;
 *
 * - otherwise, if explicitly requested:
 *
 *   - the datum will be synchronised to the server if AT_STATX_FORCE_SYNC is
 *     set or if the datum is considered out of date, and
 *
 *   - the field will be filled in and the bit will be set;
 *
 * - otherwise, if not requested, but available in approximate form without any
 *   effort, it will be filled in anyway, and the bit will be set upon return
 *   (it might not be up to date, however, and no attempt will be made to
 *   synchronise the internal state first);
 *
 * - otherwise the field and the bit will be cleared before returning.
 *
 * Items in STATX_BASIC_STATS may be marked unavailable on return, but they
 * will have values installed for compatibility purposes so that stat() and
 * co. can be emulated in userspace.
 */
#if defined(HAVE_STRUCT_STATX)
#include <sys/stat.h>
#else
struct statx {
	/* 0x00 */
	uint32_t	stx_mask;
	uint32_t	stx_blksize;
	uint64_t	stx_attributes;
	/* 0x10 */
	uint32_t	stx_nlink;
	uint32_t	stx_uid;
	uint32_t	stx_gid;
	uint16_t	stx_mode;
	uint16_t	__spare0[1];
	/* 0x20 */
	uint64_t	stx_ino;
	uint64_t	stx_size;
	uint64_t	stx_blocks;
	uint64_t	stx_attributes_mask;
	/* 0x40 */
	const struct statx_timestamp	stx_atime;
	const struct statx_timestamp	stx_btime;
	const struct statx_timestamp	stx_ctime;
	const struct statx_timestamp	stx_mtime;
	/* 0x80 */
	uint32_t	stx_rdev_major;
	uint32_t	stx_rdev_minor;
	uint32_t	stx_dev_major;
	uint32_t	stx_dev_minor;
	/* 0x90 */
	uint64_t	__spare2[14];
	/* 0x100 */
};
#endif

#if !defined(HAVE_STATX)

/*
 * statx: wrapper function of statx
 *
 * Returns: It returns status of statx syscall
 */
static inline int statx(int dirfd, const char *pathname, unsigned int flags,
			unsigned int mask, struct statx *statxbuf)
{
	return tst_syscall(__NR_statx, dirfd, pathname, flags, mask, statxbuf);
}
#endif

/*
 * Flags to be stx_mask
 *
 * Query request/result mask for statx() and struct statx::stx_mask.
 *
 * These bits should be set in the mask argument of statx() to request
 * particular items when calling statx().
 */
#ifndef STATX_TYPE
# define STATX_TYPE		0x00000001U
#endif

#ifndef STATX_MODE
# define STATX_MODE		0x00000002U
#endif

#ifndef STATX_NLINK
# define STATX_NLINK		0x00000004U
#endif

#ifndef STATX_UID
# define STATX_UID		0x00000008U
#endif

#ifndef STATX_GID
# define STATX_GID		0x00000010U
#endif

#ifndef STATX_ATIME
# define STATX_ATIME		0x00000020U
#endif

#ifndef STATX_MTIME
# define STATX_MTIME		0x00000040U
#endif

#ifndef STATX_CTIME
# define STATX_CTIME		0x00000080U
#endif

#ifndef STATX_INO
# define STATX_INO		0x00000100U
#endif

#ifndef STATX_SIZE
# define STATX_SIZE		0x00000200U
#endif

#ifndef STATX_BLOCKS
# define STATX_BLOCKS		0x00000400U
#endif

#ifndef STATX_BASIC_STATS
# define STATX_BASIC_STATS	0x000007ffU
#endif

#ifndef STATX_BTIME
# define STATX_BTIME		0x00000800U
#endif

#ifndef STATX_MNT_ID
# define STATX_MNT_ID		0x00001000U
#endif

#ifndef STATX_ALL
# define STATX_ALL		0x00000fffU
#endif

#ifndef STATX__RESERVED
# define STATX__RESERVED	0x80000000U
#endif

/*
 * Attributes to be found in stx_attributes and masked in stx_attributes_mask.
 *
 * These give information about the features or the state of a file that might
 * be of use to ordinary userspace programs such as GUIs or ls rather than
 * specialised tools.
 *
 * Note that the flags marked [I] correspond to generic FS_IOC_FLAGS
 * semantically.  Where possible, the numerical value is picked to correspond
 * also.
 */
#ifndef STATX_ATTR_COMPRESSED
# define STATX_ATTR_COMPRESSED	0x00000004
#endif

#ifndef STATX_ATTR_IMMUTABLE
# define STATX_ATTR_IMMUTABLE	0x00000010
#endif

#ifndef STATX_ATTR_APPEND
# define STATX_ATTR_APPEND	0x00000020
#endif

#ifndef STATX_ATTR_NODUMP
# define STATX_ATTR_NODUMP	0x00000040
#endif

#ifndef STATX_ATTR_ENCRYPTED
# define STATX_ATTR_ENCRYPTED	0x00000800
#endif

#ifndef STATX_ATTR_AUTOMOUNT
# define STATX_ATTR_AUTOMOUNT	0x00001000
#endif

#ifndef AT_SYMLINK_NOFOLLOW
# define AT_SYMLINK_NOFOLLOW	0x100
#endif

#ifndef AT_REMOVEDIR
# define AT_REMOVEDIR		0x200
#endif

#ifndef AT_SYMLINK_FOLLOW
# define AT_SYMLINK_FOLLOW	0x400
#endif

#ifndef AT_NO_AUTOMOUNT
# define AT_NO_AUTOMOUNT	0x800
#endif

#ifndef AT_EMPTY_PATH
# define AT_EMPTY_PATH		0x1000
#endif

#ifndef AT_STATX_SYNC_TYPE
# define AT_STATX_SYNC_TYPE	0x6000
#endif

#ifndef AT_STATX_SYNC_AS_STAT
# define AT_STATX_SYNC_AS_STAT	0x0000
#endif

#ifndef AT_STATX_FORCE_SYNC
# define AT_STATX_FORCE_SYNC	0x2000
#endif

#ifndef AT_STATX_DONT_SYNC
# define AT_STATX_DONT_SYNC	0x4000
#endif

#endif /* LAPI_STAT_H__ */
