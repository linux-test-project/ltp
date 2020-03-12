// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef FSMOUNT_H__
#define FSMOUNT_H__

#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "config.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

#ifndef HAVE_FSOPEN
int fsopen(const char *fsname, unsigned int flags)
{
	return tst_syscall(__NR_fsopen, fsname, flags);
}
#endif /* HAVE_FSOPEN */

#ifndef HAVE_FSCONFIG
int fsconfig(int fd, unsigned int cmd, const char *key,
	     const void *value, int aux)
{
	return tst_syscall(__NR_fsconfig, fd, cmd, key, value, aux);
}
#endif /* HAVE_FSCONFIG */

#ifndef HAVE_FSMOUNT
int fsmount(int fd, unsigned int flags, unsigned int mount_attrs)
{
	return tst_syscall(__NR_fsmount, fd, flags, mount_attrs);
}
#endif /* HAVE_FSMOUNT */

#ifndef HAVE_FSPICK
int fspick(int dirfd, const char *pathname, unsigned int flags)
{
	return tst_syscall(__NR_fspick, dirfd, pathname, flags);
}
#endif /* HAVE_FSPICK */

#ifndef HAVE_MOVE_MOUNT
int move_mount(int from_dirfd, const char *from_pathname, int to_dirfd,
	       const char *to_pathname, unsigned int flags)
{
	return tst_syscall(__NR_move_mount, from_dirfd, from_pathname, to_dirfd,
			   to_pathname, flags);
}
#endif /* HAVE_MOVE_MOUNT */

#ifndef HAVE_OPEN_TREE
int open_tree(int dirfd, const char *pathname, unsigned int flags)
{
	return tst_syscall(__NR_open_tree, dirfd, pathname, flags);
}
#endif /* HAVE_OPEN_TREE */

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

void fsopen_supported_by_kernel(void)
{
	if ((tst_kvercmp(5, 2, 0)) < 0) {
		/* Check if the syscall is backported on an older kernel */
		TEST(syscall(__NR_fsopen, NULL, 0));
		if (TST_RET != -1)
			SAFE_CLOSE(TST_RET);
		else if (TST_ERR == ENOSYS)
			tst_brk(TCONF, "Test not supported on kernel version < v5.2");
	}
}

#endif /* FSMOUNT_H__ */
