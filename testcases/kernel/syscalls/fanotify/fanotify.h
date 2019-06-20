/*
 * fanotify testcase common definitions.
 *
 * Copyright (c) 2012 Linux Test Project.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Jan Kara, November 2013
 */

#ifndef	__FANOTIFY_H__
#define	__FANOTIFY_H__

#include "config.h"
#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#if defined(HAVE_SYS_FANOTIFY_H)

#include <sys/fanotify.h>

#else /* HAVE_SYS_FANOTIFY_H */

/* fanotify(7) wrappers */

#include <stdint.h>
#include "lapi/syscalls.h"

static int fanotify_init(unsigned int flags, unsigned int event_f_flags)
{
	return syscall(__NR_fanotify_init, flags, event_f_flags);
}

static long fanotify_mark(int fd, unsigned int flags, uint64_t mask,
                     int dfd, const char *pathname)
{
	return syscall(__NR_fanotify_mark, fd, flags, mask, dfd, pathname);
}

#endif /* HAVE_SYS_FANOTIFY_H */

#ifndef FAN_REPORT_TID
#define FAN_REPORT_TID		0x00000100
#endif

#ifndef FAN_MARK_INODE
#define FAN_MARK_INODE		0
#endif
#ifndef FAN_MARK_FILESYSTEM
#define FAN_MARK_FILESYSTEM	0x00000100
#endif
#ifndef FAN_OPEN_EXEC
#define FAN_OPEN_EXEC		0x00001000
#endif
#ifndef FAN_OPEN_EXEC_PERM
#define FAN_OPEN_EXEC_PERM	0x00040000
#endif

/*
 * FAN_ALL_PERM_EVENTS has been deprecated, so any new permission events
 * are not to be added to it. To cover the instance where a new permission
 * event is defined, we create a new macro that is to include all
 * permission events. Any new permission events should be added to this
 * macro.
 */
#define LTP_ALL_PERM_EVENTS	(FAN_OPEN_PERM | FAN_OPEN_EXEC_PERM | \
				 FAN_ACCESS_PERM)

struct fanotify_mark_type {
	unsigned int flag;
	const char * name;
};

#ifndef FAN_REPORT_FID
#define FAN_REPORT_FID		0x00000200

struct fanotify_event_info_header {
	uint8_t info_type;
	uint8_t pad;
	uint16_t len;
};

struct fanotify_event_info_fid {
	struct fanotify_event_info_header hdr;
	__kernel_fsid_t fsid;
	unsigned char handle[0];
};

#endif

/*
 * Helper function used to obtain __kernel_fsid_t and file_handle objects
 * for a given path. Used by test files correlated to FAN_REPORT_FID
 * functionality.
 */
static inline void fanotify_get_fid(const char *path, __kernel_fsid_t *fsid,
			struct file_handle *handle)
{
	int mount_id;
	struct statfs stats;

	if (statfs(path, &stats) == -1)
		tst_brk(TBROK | TERRNO,
			"statfs(%s, ...) failed", path);
	memcpy(fsid, &stats.f_fsid, sizeof(stats.f_fsid));

#ifdef HAVE_NAME_TO_HANDLE_AT
	if (name_to_handle_at(AT_FDCWD, path, handle, &mount_id, 0) == -1) {
		if (errno == EOPNOTSUPP) {
			tst_brk(TCONF,
				"filesystem %s does not support file handles",
				tst_device->fs_type);
		}
		tst_brk(TBROK | TERRNO,
			"name_to_handle_at(AT_FDCWD, %s, ...) failed", path);
	}
#else
	tst_brk(TCONF, "name_to_handle_at() is not implmented");
#endif /* HAVE_NAME_TO_HANDLE_AT */
}

#define INIT_FANOTIFY_MARK_TYPE(t) \
	{ FAN_MARK_ ## t, "FAN_MARK_" #t }

#endif /* __FANOTIFY_H__ */
