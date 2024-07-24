// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Referred from linux kernel include/uapi/linux/fs.h
 * Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 * Email: code@zilogic.com
 */

#ifndef LAPI_FS_H__
#define LAPI_FS_H__

#include "config.h"
#ifndef HAVE_MOUNT_SETATTR
# ifdef HAVE_LINUX_FS_H
#  include <linux/fs.h>
# endif
#endif

#include <sys/user.h>
#include <limits.h>
#include <stdint.h>
#include "lapi/abisize.h"

#ifndef HAVE_STRUCT_FILE_CLONE_RANGE
struct file_clone_range {
	int64_t src_fd;
	uint64_t src_offset;
	uint64_t src_length;
	uint64_t dest_offset;
};
#endif

#ifndef FS_IOC_GETFLAGS
# define	FS_IOC_GETFLAGS	_IOR('f', 1, long)
#endif

#ifndef FS_IOC_SETFLAGS
# define	FS_IOC_SETFLAGS	_IOW('f', 2, long)
#endif

#ifndef FS_COMPR_FL
# define	FS_COMPR_FL        0x00000004 /* Compress file */
#endif

#ifndef FS_IMMUTABLE_FL
# define FS_IMMUTABLE_FL	   0x00000010 /* Immutable file */
#endif

#ifndef FS_APPEND_FL
# define FS_APPEND_FL	   0x00000020 /* writes to file may only append */
#endif

#ifndef FS_NODUMP_FL
# define FS_NODUMP_FL	   0x00000040 /* do not dump file */
#endif

#ifndef FS_VERITY_FL
# define FS_VERITY_FL	   0x00100000 /* Verity protected inode */
#endif

#ifndef FICLONE
# define FICLONE		_IOW(0x94, 9, int)
#endif

#ifndef FICLONERANGE
# define FICLONERANGE		_IOW(0x94, 13, struct file_clone_range)
#endif

/*
 * Helper function to get MAX_LFS_FILESIZE.
 * Missing PAGE_SHIFT on some libc prevents defining MAX_LFS_FILESIZE.
 *
 * 64 bit: macro taken from kernel from include/linux/fs.h
 * 32 bit: own implementation
 */
static inline long long tst_max_lfs_filesize(void)
{
#ifdef TST_ABI64
	return LLONG_MAX;
#else
        long page_size = getpagesize();
        long long ret = ULONG_MAX;

        while (page_size >>= 1)
                ret <<= 1;

        return ret;
#endif
}

#endif /* LAPI_FS_H__ */
