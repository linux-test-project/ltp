// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2014-2023
 */

#ifndef LAPI_FCNTL_H__
#define LAPI_FCNTL_H__

#include "config.h"
#include <fcntl.h>
#include <sys/socket.h>

/* NOTE: #define _GNU_SOURCE if you need O_DIRECT in tests */

#ifndef O_CLOEXEC
# define O_CLOEXEC 02000000
#endif

#ifndef SOCK_CLOEXEC
# define SOCK_CLOEXEC O_CLOEXEC
#endif

#ifndef SOCK_NONBLOCK
# define SOCK_NONBLOCK O_NONBLOCK
#endif

#ifndef O_TMPFILE
# define O_TMPFILE (020000000 | O_DIRECTORY)
#endif

#ifndef F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC 1030
#endif

#ifndef F_SETPIPE_SZ
# define F_SETPIPE_SZ 1031
#endif

#ifndef F_GETPIPE_SZ
# define F_GETPIPE_SZ 1032
#endif

/*
 * Set/Get seals
 */
#ifndef F_ADD_SEALS
# define F_ADD_SEALS     (1033)
#endif

#ifndef F_GET_SEALS
# define F_GET_SEALS     (1034)
#endif

#ifndef F_SEAL_SEAL
# define F_SEAL_SEAL     0x0001  /* prevent further seals from being set */
#endif

#ifndef F_SEAL_SHRINK
# define F_SEAL_SHRINK   0x0002  /* prevent file from shrinking */
#endif
#ifndef F_SEAL_GROW
# define F_SEAL_GROW     0x0004  /* prevent file from growing */
#endif
#ifndef F_SEAL_WRITE
# define F_SEAL_WRITE    0x0008  /* prevent writes */
#endif

#ifndef F_OWNER_PGRP
# define F_OWNER_PGRP 2
#endif

#ifndef F_OFD_GETLK
# define F_OFD_GETLK	36
#endif

#ifndef F_OFD_SETLK
# define F_OFD_SETLK	37
#endif

#ifndef F_OFD_SETLKW
# define F_OFD_SETLKW	38
#endif

#ifndef AT_FDCWD
# define AT_FDCWD -100
#endif

#ifndef AT_SYMLINK_NOFOLLOW
# define AT_SYMLINK_NOFOLLOW	0x100
#endif

#ifndef AT_REMOVEDIR
# define AT_REMOVEDIR		0x200
#endif

#ifndef AT_HANDLE_FID
# define AT_HANDLE_FID		AT_REMOVEDIR
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

#ifndef AT_STATX_SYNC_AS_STAT
# define AT_STATX_SYNC_AS_STAT	0x0000
#endif

#ifndef AT_STATX_FORCE_SYNC
# define AT_STATX_FORCE_SYNC	0x2000
#endif

#ifndef AT_STATX_DONT_SYNC
# define AT_STATX_DONT_SYNC	0x4000
#endif

#ifndef AT_STATX_SYNC_TYPE
# define AT_STATX_SYNC_TYPE	0x6000
#endif

#ifndef O_NOATIME
# define O_NOATIME 01000000
#endif

#ifndef O_PATH
# ifdef __sparc__
#  define O_PATH 0x1000000
# else
#  define O_PATH 010000000
# endif
#endif

#ifndef FALLOC_FL_KEEP_SIZE
# define FALLOC_FL_KEEP_SIZE 1
#endif

#ifndef RENAME_NOREPLACE
# define RENAME_NOREPLACE	(1 << 0)
#endif

#ifndef RENAME_EXCHANGE
# define RENAME_EXCHANGE		(1 << 1)
#endif

#ifndef RENAME_WHITEOUT
# define RENAME_WHITEOUT		(1 << 2)
#endif

#ifndef F_LINUX_SPECIFIC_BASE
#define F_LINUX_SPECIFIC_BASE 1024
#endif

#ifndef F_CREATED_QUERY
#define F_CREATED_QUERY (F_LINUX_SPECIFIC_BASE + 4)
#endif

/* splice, vmsplice, tee */

#ifndef SPLICE_F_NONBLOCK
# define SPLICE_F_NONBLOCK 2
#endif

#ifndef MAX_HANDLE_SZ
# define MAX_HANDLE_SZ	128
#endif

#define TST_OPEN_NEEDS_MODE(oflag) \
	(((oflag) & O_CREAT) != 0 || ((oflag) & O_TMPFILE) == O_TMPFILE)

#ifndef HAVE_STRUCT_FILE_HANDLE
struct file_handle {
	unsigned int handle_bytes;
	int handle_type;
	/* File identifier.  */
	unsigned char f_handle[0];
};
#endif /* HAVE_STRUCT_FILE_HANDLE */

#endif /* LAPI_FCNTL_H__ */
