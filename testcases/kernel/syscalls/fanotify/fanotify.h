// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2012 Linux Test Project.  All Rights Reserved.
 * Author: Jan Kara, November 2013
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
#ifndef FAN_REPORT_FID
#define FAN_REPORT_FID		0x00000200
#endif

#ifndef FAN_MARK_INODE
#define FAN_MARK_INODE		0
#endif
#ifndef FAN_MARK_FILESYSTEM
#define FAN_MARK_FILESYSTEM	0x00000100
#endif
/* New dirent event masks */
#ifndef FAN_ATTRIB
#define FAN_ATTRIB		0x00000004
#endif
#ifndef FAN_MOVED_FROM
#define FAN_MOVED_FROM		0x00000040
#endif
#ifndef FAN_MOVED_TO
#define FAN_MOVED_TO		0x00000080
#endif
#ifndef FAN_CREATE
#define FAN_CREATE		0x00000100
#endif
#ifndef FAN_DELETE
#define FAN_DELETE		0x00000200
#endif
#ifndef FAN_DELETE_SELF
#define FAN_DELETE_SELF		0x00000400
#endif
#ifndef FAN_MOVE_SELF
#define FAN_MOVE_SELF		0x00000800
#endif
#ifndef FAN_MOVE
#define FAN_MOVE		(FAN_MOVED_FROM | FAN_MOVED_TO)
#endif
#ifndef FAN_OPEN_EXEC
#define FAN_OPEN_EXEC		0x00001000
#endif
#ifndef FAN_OPEN_EXEC_PERM
#define FAN_OPEN_EXEC_PERM	0x00040000
#endif
#ifndef FAN_DIR_MODIFY
#define FAN_DIR_MODIFY		0x00080000
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

#ifndef __kernel_fsid_t
typedef struct {
	int	val[2];
} lapi_fsid_t;
#define __kernel_fsid_t lapi_fsid_t
#endif /* __kernel_fsid_t */

#ifndef FAN_EVENT_INFO_TYPE_FID
#define FAN_EVENT_INFO_TYPE_FID		1
#endif
#ifndef FAN_EVENT_INFO_TYPE_DFID_NAME
#define FAN_EVENT_INFO_TYPE_DFID_NAME	2
#endif

#ifndef HAVE_STRUCT_FANOTIFY_EVENT_INFO_HEADER
struct fanotify_event_info_header {
	uint8_t info_type;
	uint8_t pad;
	uint16_t len;
};
#endif /* HAVE_STRUCT_FANOTIFY_EVENT_INFO_HEADER */

#ifndef HAVE_STRUCT_FANOTIFY_EVENT_INFO_FID
struct fanotify_event_info_fid {
	struct fanotify_event_info_header hdr;
	__kernel_fsid_t fsid;
	unsigned char handle[0];
};
#endif /* HAVE_STRUCT_FANOTIFY_EVENT_INFO_FID */

/* NOTE: only for struct fanotify_event_info_fid */
#ifdef HAVE_STRUCT_FANOTIFY_EVENT_INFO_FID_FSID___VAL
# define FSID_VAL_MEMBER(fsid, i) (fsid.__val[i])
#else
# define FSID_VAL_MEMBER(fsid, i) (fsid.val[i])
#endif /* HAVE_STRUCT_FANOTIFY_EVENT_INFO_FID_FSID___VAL */

#ifdef HAVE_NAME_TO_HANDLE_AT
/*
 * Helper function used to obtain fsid and file_handle for a given path.
 * Used by test files correlated to FAN_REPORT_FID functionality.
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

	if (name_to_handle_at(AT_FDCWD, path, handle, &mount_id, 0) == -1) {
		if (errno == EOPNOTSUPP) {
			tst_brk(TCONF,
				"filesystem %s does not support file handles",
				tst_device->fs_type);
		}
		tst_brk(TBROK | TERRNO,
			"name_to_handle_at(AT_FDCWD, %s, ...) failed", path);
	}
}

struct fanotify_fid_t {
	__kernel_fsid_t fsid;
	struct file_handle handle;
	char buf[MAX_HANDLE_SZ];
};

static inline void fanotify_save_fid(const char *path,
				     struct fanotify_fid_t *fid)
{
	int *fh = (int *)(fid->handle.f_handle);

	fh[0] = fh[1] = fh[2] = 0;
	fid->handle.handle_bytes = MAX_HANDLE_SZ;
	fanotify_get_fid(path, &fid->fsid, &fid->handle);

	tst_res(TINFO,
		"fid(%s) = %x.%x.%x.%x.%x...", path, fid->fsid.val[0],
		fid->fsid.val[1], fh[0], fh[1], fh[2]);
}
#endif /* HAVE_NAME_TO_HANDLE_AT */

#define INIT_FANOTIFY_MARK_TYPE(t) \
	{ FAN_MARK_ ## t, "FAN_MARK_" #t }

#endif /* __FANOTIFY_H__ */
