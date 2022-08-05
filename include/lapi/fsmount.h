// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021-2022
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef LAPI_FSMOUNT_H__
#define LAPI_FSMOUNT_H__

#include "config.h"
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/mount.h>

#ifndef HAVE_FSOPEN
# ifdef HAVE_LINUX_MOUNT_H
#  include <linux/mount.h>
# endif
#endif

#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

/*
 * Mount attributes.
 */
#ifndef MOUNT_ATTR_RDONLY
# define MOUNT_ATTR_RDONLY       0x00000001 /* Mount read-only */
#endif
#ifndef MOUNT_ATTR_NOSUID
# define MOUNT_ATTR_NOSUID       0x00000002 /* Ignore suid and sgid bits */
#endif
#ifndef MOUNT_ATTR_NODEV
# define MOUNT_ATTR_NODEV        0x00000004 /* Disallow access to device special files */
#endif
#ifndef MOUNT_ATTR_NOEXEC
# define MOUNT_ATTR_NOEXEC       0x00000008 /* Disallow program execution */
#endif
#ifndef MOUNT_ATTR_NODIRATIME
# define MOUNT_ATTR_NODIRATIME   0x00000080 /* Do not update directory access times */
#endif
#ifndef MOUNT_ATTR_NOSYMFOLLOW
# define MOUNT_ATTR_NOSYMFOLLOW  0x00200000 /* Do not follow symlinks */
#endif

#ifndef ST_NOSYMFOLLOW
# define ST_NOSYMFOLLOW 0x2000 /* do not follow symlinks */
#endif

#ifndef HAVE_STRUCT_MOUNT_ATTR
/*
 * mount_setattr()
 */
struct mount_attr {
	uint64_t attr_set;
	uint64_t attr_clr;
	uint64_t propagation;
	uint64_t userns_fd;
};
#endif

#ifndef HAVE_FSOPEN
static inline int fsopen(const char *fsname, unsigned int flags)
{
	return tst_syscall(__NR_fsopen, fsname, flags);
}
#endif /* HAVE_FSOPEN */

#ifndef HAVE_FSCONFIG
static inline int fsconfig(int fd, unsigned int cmd, const char *key,
                           const void *value, int aux)
{
	return tst_syscall(__NR_fsconfig, fd, cmd, key, value, aux);
}
#endif /* HAVE_FSCONFIG */

#ifndef HAVE_FSMOUNT
static inline int fsmount(int fd, unsigned int flags, unsigned int mount_attrs)
{
	return tst_syscall(__NR_fsmount, fd, flags, mount_attrs);
}
#endif /* HAVE_FSMOUNT */

#ifndef HAVE_FSPICK
static inline int fspick(int dirfd, const char *pathname, unsigned int flags)
{
	return tst_syscall(__NR_fspick, dirfd, pathname, flags);
}
#endif /* HAVE_FSPICK */

#ifndef HAVE_MOVE_MOUNT
static inline int move_mount(int from_dirfd, const char *from_pathname,
                             int to_dirfd, const char *to_pathname,
                             unsigned int flags)
{
	return tst_syscall(__NR_move_mount, from_dirfd, from_pathname, to_dirfd,
			   to_pathname, flags);
}
#endif /* HAVE_MOVE_MOUNT */

#ifndef HAVE_OPEN_TREE
static inline int open_tree(int dirfd, const char *pathname, unsigned int flags)
{
	return tst_syscall(__NR_open_tree, dirfd, pathname, flags);
}
#endif /* HAVE_OPEN_TREE */

#ifndef HAVE_MOUNT_SETATTR
static inline int mount_setattr(int dirfd, const char *from_pathname, unsigned int flags,
				struct mount_attr *attr, size_t size)
{
	return tst_syscall(__NR_mount_setattr, dirfd, from_pathname, flags,
			   attr, size);
}
#endif /* HAVE_MOUNT_SETATTR */

/*
 * New headers added in kernel after 5.2 release, create them for old userspace.
*/

#ifndef OPEN_TREE_CLONE

/*
 * open_tree() flags.
 */
#define OPEN_TREE_CLONE		1		/* Clone the target tree and attach the clone */
#define OPEN_TREE_CLOEXEC	O_CLOEXEC	/* Close the file on execve() */

/*
 * move_mount() flags.
 */
#define MOVE_MOUNT_F_SYMLINKS		0x00000001 /* Follow symlinks on from path */
#define MOVE_MOUNT_F_AUTOMOUNTS		0x00000002 /* Follow automounts on from path */
#define MOVE_MOUNT_F_EMPTY_PATH		0x00000004 /* Empty from path permitted */
#define MOVE_MOUNT_T_SYMLINKS		0x00000010 /* Follow symlinks on to path */
#define MOVE_MOUNT_T_AUTOMOUNTS		0x00000020 /* Follow automounts on to path */
#define MOVE_MOUNT_T_EMPTY_PATH		0x00000040 /* Empty to path permitted */
#define MOVE_MOUNT__MASK		0x00000077

/*
 * fsopen() flags.
 */
#define FSOPEN_CLOEXEC		0x00000001

/*
 * fspick() flags.
 */
#define FSPICK_CLOEXEC		0x00000001
#define FSPICK_SYMLINK_NOFOLLOW	0x00000002
#define FSPICK_NO_AUTOMOUNT	0x00000004
#define FSPICK_EMPTY_PATH	0x00000008

/*
 * The type of fsconfig() call made.
 */
enum fsconfig_command {
	FSCONFIG_SET_FLAG	= 0,	/* Set parameter, supplying no value */
	FSCONFIG_SET_STRING	= 1,	/* Set parameter, supplying a string value */
	FSCONFIG_SET_BINARY	= 2,	/* Set parameter, supplying a binary blob value */
	FSCONFIG_SET_PATH	= 3,	/* Set parameter, supplying an object by path */
	FSCONFIG_SET_PATH_EMPTY	= 4,	/* Set parameter, supplying an object by (empty) path */
	FSCONFIG_SET_FD		= 5,	/* Set parameter, supplying an object by fd */
	FSCONFIG_CMD_CREATE	= 6,	/* Invoke superblock creation */
	FSCONFIG_CMD_RECONFIGURE = 7,	/* Invoke superblock reconfiguration */
};

/*
 * fsmount() flags.
 */
#define FSMOUNT_CLOEXEC		0x00000001

/*
 * Mount attributes.
 */
#define MOUNT_ATTR_RDONLY	0x00000001 /* Mount read-only */
#define MOUNT_ATTR_NOSUID	0x00000002 /* Ignore suid and sgid bits */
#define MOUNT_ATTR_NODEV	0x00000004 /* Disallow access to device special files */
#define MOUNT_ATTR_NOEXEC	0x00000008 /* Disallow program execution */
#define MOUNT_ATTR__ATIME	0x00000070 /* Setting on how atime should be updated */
#define MOUNT_ATTR_RELATIME	0x00000000 /* - Update atime relative to mtime/ctime. */
#define MOUNT_ATTR_NOATIME	0x00000010 /* - Do not update access times. */
#define MOUNT_ATTR_STRICTATIME	0x00000020 /* - Always perform atime updates */
#define MOUNT_ATTR_NODIRATIME	0x00000080 /* Do not update directory access times */

#endif /* OPEN_TREE_CLONE */

static inline void fsopen_supported_by_kernel(void)
{
	long ret;

	if ((tst_kvercmp(5, 2, 0)) < 0) {
		/* Check if the syscall is backported on an older kernel */
		ret = syscall(__NR_fsopen, NULL, 0);
		if (ret != -1)
			SAFE_CLOSE(ret);
		else if (errno == ENOSYS)
			tst_brk(TCONF, "Test not supported on kernel version < v5.2");
	}
}

#endif /* LAPI_FSMOUNT_H__ */
