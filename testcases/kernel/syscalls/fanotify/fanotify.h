// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2012-2020 Linux Test Project.  All Rights Reserved.
 * Author: Jan Kara, November 2013
 */

#ifndef	__FANOTIFY_H__
#define	__FANOTIFY_H__

#include "config.h"
#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/fanotify.h>
#include "lapi/fcntl.h"

int safe_fanotify_init(const char *file, const int lineno,
	unsigned int flags, unsigned int event_f_flags)
{
	int rval;

	rval = fanotify_init(flags, event_f_flags);

	if (rval == -1) {
		if (errno == ENOSYS) {
			tst_brk_(file, lineno, TCONF,
				"fanotify is not configured in this kernel");
		}
		tst_brk_(file, lineno, TBROK | TERRNO,
			"%s:%d: fanotify_init() failed", file, lineno);
	}

	if (rval < -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "invalid fanotify_init() return %d", rval);
	}

	return rval;
}

static inline int safe_fanotify_mark(const char *file, const int lineno,
			int fd, unsigned int flags, uint64_t mask,
			int dfd, const char *pathname)
{
	int rval;

	rval = fanotify_mark(fd, flags, mask, dfd, pathname);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO, "fanotify_mark() failed");
	}

	if (rval < -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "invalid fanotify_mark() return %d", rval);
	}

	return rval;
}

#define SAFE_FANOTIFY_MARK(fd, flags, mask, dfd, pathname)  \
	safe_fanotify_mark(__FILE__, __LINE__, (fd), (flags), (mask), (dfd), (pathname))

#define SAFE_FANOTIFY_INIT(fan, mode)  \
	safe_fanotify_init(__FILE__, __LINE__, (fan), (mode))

#ifndef FAN_REPORT_TID
#define FAN_REPORT_TID		0x00000100
#endif
#ifndef FAN_REPORT_FID
#define FAN_REPORT_FID		0x00000200
#endif
#ifndef FAN_REPORT_DIR_FID
#define FAN_REPORT_DIR_FID	0x00000400
#endif
#ifndef FAN_REPORT_NAME
#define FAN_REPORT_NAME		0x00000800
#define FAN_REPORT_DFID_NAME     (FAN_REPORT_DIR_FID | FAN_REPORT_NAME)
#endif
#ifndef FAN_REPORT_PIDFD
#define FAN_REPORT_PIDFD	0x00000080
#endif

/* Non-uapi convenience macros */
#ifndef FAN_REPORT_DFID_NAME_FID
#define FAN_REPORT_DFID_NAME_FID (FAN_REPORT_DFID_NAME | FAN_REPORT_FID)
#endif
#ifndef FAN_REPORT_DFID_FID
#define FAN_REPORT_DFID_FID      (FAN_REPORT_DIR_FID | FAN_REPORT_FID)
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

/* Additional error status codes that can be returned to userspace */
#ifndef FAN_NOPIDFD
#define FAN_NOPIDFD		-1
#endif
#ifndef FAN_EPIDFD
#define FAN_EPIDFD		-2
#endif

/* Flags required for unprivileged user group */
#define FANOTIFY_REQUIRED_USER_INIT_FLAGS    (FAN_REPORT_FID)

/*
 * FAN_ALL_PERM_EVENTS has been deprecated, so any new permission events
 * are not to be added to it. To cover the instance where a new permission
 * event is defined, we create a new macro that is to include all
 * permission events. Any new permission events should be added to this
 * macro.
 */
#define LTP_ALL_PERM_EVENTS	(FAN_OPEN_PERM | FAN_OPEN_EXEC_PERM | \
				 FAN_ACCESS_PERM)

struct fanotify_group_type {
	unsigned int flag;
	const char * name;
};

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
#ifndef FAN_EVENT_INFO_TYPE_DFID
#define FAN_EVENT_INFO_TYPE_DFID	3
#endif
#ifndef FAN_EVENT_INFO_TYPE_PIDFD
#define FAN_EVENT_INFO_TYPE_PIDFD	4
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

#ifndef HAVE_STRUCT_FANOTIFY_EVENT_INFO_PIDFD
struct fanotify_event_info_pidfd {
	struct fanotify_event_info_header hdr;
	int32_t pidfd;
};
#endif /* HAVE_STRUCT_FANOTIFY_EVENT_INFO_PIDFD */

/* NOTE: only for struct fanotify_event_info_fid */
#ifdef HAVE_STRUCT_FANOTIFY_EVENT_INFO_FID_FSID___VAL
# define FSID_VAL_MEMBER(fsid, i) (fsid.__val[i])
#else
# define FSID_VAL_MEMBER(fsid, i) (fsid.val[i])
#endif /* HAVE_STRUCT_FANOTIFY_EVENT_INFO_FID_FSID___VAL */

#ifdef HAVE_NAME_TO_HANDLE_AT

#ifndef MAX_HANDLE_SZ
#define MAX_HANDLE_SZ		128
#endif

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

#define INIT_FANOTIFY_GROUP_TYPE(t) \
	{ FAN_ ## t, "FAN_" #t }

#define INIT_FANOTIFY_MARK_TYPE(t) \
	{ FAN_MARK_ ## t, "FAN_MARK_" #t }

static inline void require_fanotify_access_permissions_supported_by_kernel(void)
{
	int fd;

	fd = SAFE_FANOTIFY_INIT(FAN_CLASS_CONTENT, O_RDONLY);

	if (fanotify_mark(fd, FAN_MARK_ADD, FAN_ACCESS_PERM, AT_FDCWD, ".") < 0) {
		if (errno == EINVAL) {
			tst_brk(TCONF | TERRNO,
				"CONFIG_FANOTIFY_ACCESS_PERMISSIONS not configured in kernel?");
		} else {
			tst_brk(TBROK | TERRNO,
				"fanotify_mark (%d, FAN_MARK_ADD, FAN_ACCESS_PERM, AT_FDCWD, \".\") failed", fd);
		}
	}

	SAFE_CLOSE(fd);
}

static inline int fanotify_events_supported_by_kernel(uint64_t mask)
{
	int fd;
	int rval = 0;

	fd = SAFE_FANOTIFY_INIT(FAN_CLASS_CONTENT, O_RDONLY);

	if (fanotify_mark(fd, FAN_MARK_ADD, mask, AT_FDCWD, ".") < 0) {
		if (errno == EINVAL) {
			rval = -1;
		} else {
			tst_brk(TBROK | TERRNO,
				"fanotify_mark (%d, FAN_MARK_ADD, ..., AT_FDCWD, \".\") failed", fd);
		}
	}

	SAFE_CLOSE(fd);

	return rval;
}

/*
 * @return  0: fanotify supported both in kernel and on tested filesystem
 * @return -1: @flags not supported in kernel
 * @return -2: @flags not supported on tested filesystem (tested if @fname is not NULL)
 */
static inline int fanotify_init_flags_supported_on_fs(unsigned int flags, const char *fname)
{
	int fd;
	int rval = 0;

	fd = fanotify_init(flags, O_RDONLY);

	if (fd < 0) {
		if (errno == ENOSYS)
			tst_brk(TCONF, "fanotify not configured in kernel");

		if (errno == EINVAL)
			return -1;

		tst_brk(TBROK | TERRNO, "fanotify_init() failed");
	}

	if (fname && fanotify_mark(fd, FAN_MARK_ADD, FAN_ACCESS, AT_FDCWD, fname) < 0) {
		if (errno == ENODEV || errno == EOPNOTSUPP || errno == EXDEV) {
			rval = -2;
		} else {
			tst_brk(TBROK | TERRNO,
				"fanotify_mark (%d, FAN_MARK_ADD, ..., AT_FDCWD, %s) failed",
				fd, fname);
		}
	}

	SAFE_CLOSE(fd);

	return rval;
}

static inline int fanotify_init_flags_supported_by_kernel(unsigned int flags)
{
	return fanotify_init_flags_supported_on_fs(flags, NULL);
}

typedef void (*tst_res_func_t)(const char *file, const int lineno,
			       int ttype, const char *fmt, ...);

static inline void fanotify_init_flags_err_msg(const char *flags_str,
	const char *file, const int lineno, tst_res_func_t res_func, int fail)
{
	if (fail == -1)
		res_func(file, lineno, TCONF,
			 "%s not supported in kernel?", flags_str);
	if (fail == -2)
		res_func(file, lineno, TCONF,
			 "%s not supported on %s filesystem",
			 flags_str, tst_device->fs_type);
}

#define FANOTIFY_INIT_FLAGS_ERR_MSG(flags, fail) \
	fanotify_init_flags_err_msg(#flags, __FILE__, __LINE__, tst_res_, (fail))

#define REQUIRE_FANOTIFY_INIT_FLAGS_SUPPORTED_ON_FS(flags, fname) do { \
	fanotify_init_flags_err_msg(#flags, __FILE__, __LINE__, tst_brk_, \
		fanotify_init_flags_supported_on_fs(flags, fname)); \
	} while (0)

#define REQUIRE_FANOTIFY_INIT_FLAGS_SUPPORTED_BY_KERNEL(flags) do { \
	fanotify_init_flags_err_msg(#flags, __FILE__, __LINE__, tst_brk_, \
		fanotify_init_flags_supported_by_kernel(flags)); \
	} while (0)

static inline int fanotify_mark_supported_by_kernel(uint64_t flag)
{
	int fd;
	int rval = 0;

	fd = SAFE_FANOTIFY_INIT(FAN_CLASS_CONTENT, O_RDONLY);

	if (fanotify_mark(fd, FAN_MARK_ADD | flag, FAN_ACCESS, AT_FDCWD, ".") < 0) {
		if (errno == EINVAL) {
			rval = -1;
		} else {
			tst_brk(TBROK | TERRNO,
				"fanotify_mark (%d, FAN_MARK_ADD, ..., FAN_ACCESS, AT_FDCWD, \".\") failed", fd);
		}
	}

	SAFE_CLOSE(fd);

	return rval;
}

#endif /* __FANOTIFY_H__ */
