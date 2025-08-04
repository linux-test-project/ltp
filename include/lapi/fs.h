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
#ifndef HAVE_LINUX_FS
# include <linux/fs.h>
#endif

#include <stdint.h>
#include <sys/user.h>
#include <limits.h>
#include "lapi/abisize.h"

#ifndef HAVE_STRUCT_FSXATTR
struct fsxattr {
	uint32_t fsx_xflags;	        /* xflags field value (get/set) */
	uint32_t fsx_extsize;	        /* extsize field value (get/set)*/
	uint32_t fsx_nextents;	        /* nextents field value (get)	*/
	uint32_t fsx_projid;            /* project identifier (get/set) */
	uint32_t fsx_cowextsize;	/* CoW extsize field value (get/set)*/
	unsigned char fsx_pad[8];
};
#endif

#ifndef FS_IOC_GETFLAGS
# define	FS_IOC_GETFLAGS	_IOR('f', 1, long)
#endif

#ifndef FS_IOC_SETFLAGS
# define	FS_IOC_SETFLAGS	_IOW('f', 2, long)
#endif

#ifndef FS_IOC_FSGETXATTR
# define FS_IOC_FSGETXATTR _IOR('X', 31, struct fsxattr)
#endif

#ifndef FS_IOC_FSSETXATTR
# define FS_IOC_FSSETXATTR _IOW('X', 32, struct fsxattr)
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
