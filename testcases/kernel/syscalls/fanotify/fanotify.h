/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2012-2020 Linux Test Project.  All Rights Reserved.
 * Author: Jan Kara, November 2013
 */

#ifndef	__FANOTIFY_H__
#define	__FANOTIFY_H__

#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "lapi/fanotify.h"
#include "lapi/fcntl.h"

static inline int safe_fanotify_init(const char *file, const int lineno,
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
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "fanotify_mark(%d, 0x%x, 0x%lx, ..., %s) failed",
			 fd, flags, mask, pathname);
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

#ifdef HAVE_NAME_TO_HANDLE_AT

/*
 * Helper function used to obtain fsid and file_handle for a given path.
 * Used by test files correlated to FAN_REPORT_FID functionality.
 *
 * Returns 0 if normal NFS file handles are supported.
 * Returns AT_HANDLE_FID, if only non-decodeable file handles are supported.
 */
static inline int fanotify_get_fid(const char *path, __kernel_fsid_t *fsid,
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
			/* Try to request non-decodeable fid instead */
			if (name_to_handle_at(AT_FDCWD, path, handle, &mount_id,
					      AT_HANDLE_FID) == 0)
				return AT_HANDLE_FID;

			tst_brk(TCONF,
				"filesystem %s does not support file handles",
				tst_device->fs_type);
		}
		tst_brk(TBROK | TERRNO,
			"name_to_handle_at(AT_FDCWD, %s, ...) failed", path);
	}
	return 0;
}

struct fanotify_fid_t {
	__kernel_fsid_t fsid;
	struct file_handle handle;
	char buf[MAX_HANDLE_SZ];
};

static inline int fanotify_save_fid(const char *path,
				     struct fanotify_fid_t *fid)
{
	int *fh = (int *)(fid->handle.f_handle);
	int ret;

	fh[0] = fh[1] = fh[2] = 0;
	fid->handle.handle_bytes = MAX_HANDLE_SZ;
	ret = fanotify_get_fid(path, &fid->fsid, &fid->handle);

	tst_res(TINFO,
		"fid(%s) = %x.%x.%x.%x.%x...", path, fid->fsid.val[0],
		fid->fsid.val[1], fh[0], fh[1], fh[2]);

	return ret;
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

/*
 * @return  0: fanotify flags supported both in kernel and on tested filesystem
 * @return -1: @init_flags not supported in kernel
 * @return -2: @mark_flags not supported on tested filesystem (tested if @fname is not NULL)
 * @return -3: @mark_flags not supported on overlayfs (tested if @fname == OVL_MNT)
 */
static inline int fanotify_flags_supported_on_fs(unsigned int init_flags,
						 unsigned int mark_flags,
						 uint64_t event_flags,
						 const char *fname)
{
	int fd;
	int rval = 0;

	fd = fanotify_init(init_flags, O_RDONLY);

	if (fd < 0) {
		if (errno == ENOSYS)
			tst_brk(TCONF, "fanotify not configured in kernel");
		if (errno != EINVAL)
			tst_brk(TBROK | TERRNO,
				"fanotify_init(%x, O_RDONLY) failed",
				init_flags);
		return -1;
	}

	if (fname && fanotify_mark(fd, FAN_MARK_ADD | mark_flags, event_flags, AT_FDCWD, fname) < 0) {
		if (errno == ENODEV || errno == EOPNOTSUPP || errno == EXDEV) {
			rval = strcmp(fname, OVL_MNT) ? -2 : -3;
		} else if (errno != EINVAL) {
			tst_brk(TBROK | TERRNO,
				"fanotify_mark (%d, FAN_MARK_ADD | %x, %llx, AT_FDCWD, %s) failed",
				fd, mark_flags, (unsigned long long)event_flags,
				fname);
		} else {
			rval = -1;
		}
	}

	SAFE_CLOSE(fd);

	return rval;
}

static inline int fanotify_init_flags_supported_on_fs(unsigned int flags, const char *fname)
{
	return fanotify_flags_supported_on_fs(flags, FAN_MARK_INODE, FAN_ACCESS, fname);
}

static inline int fanotify_init_flags_supported_by_kernel(unsigned int flags)
{
	return fanotify_init_flags_supported_on_fs(flags, NULL);
}

static inline int fanotify_mark_supported_on_fs(uint64_t flag, const char *fname)
{
	return fanotify_flags_supported_on_fs(FAN_CLASS_NOTIF, flag, FAN_ACCESS, fname);
}

#define TST_FANOTIFY_INIT_KNOWN_FLAGS                                      \
	(FAN_REPORT_DFID_NAME_TARGET | FAN_REPORT_TID | FAN_REPORT_PIDFD | \
	FAN_CLASS_NOTIF | FAN_CLASS_CONTENT | FAN_CLASS_PRE_CONTENT)

/*
 * Check support of given init flags one by one and return those which are
 * supported.
 */
static inline unsigned int fanotify_get_supported_init_flags(unsigned int flags,
	const char *fname)
{
	unsigned int i, flg, arg, ret = 0;
	static const struct { unsigned int flag, deps; } deplist[] = {
		{FAN_REPORT_NAME, FAN_REPORT_DIR_FID},
		{FAN_REPORT_TARGET_FID, FAN_REPORT_DFID_NAME_FID},
		{0, 0}
	};

	if (flags & ~TST_FANOTIFY_INIT_KNOWN_FLAGS) {
		tst_brk(TBROK, "fanotify_init() feature check called with unknown flags %x, please update flag dependency table if needed",
			flags & ~TST_FANOTIFY_INIT_KNOWN_FLAGS);
	}

	for (flg = 1; flg; flg <<= 1) {
		if (!(flags & flg))
			continue;

		arg = flg;

		for (i = 0; deplist[i].flag; i++) {
			if (deplist[i].flag == flg) {
				arg |= deplist[i].deps;
				break;
			}
		}

		if (!fanotify_init_flags_supported_on_fs(arg, fname))
			ret |= flg;
	}

	return ret;
}

typedef void (*tst_res_func_t)(const char *file, const int lineno,
			       int ttype, const char *fmt, ...);

static inline void fanotify_flags_err_msg(const char *flags_str,
	const char *file, const int lineno, tst_res_func_t res_func, int fail)
{
	if (fail == -1)
		res_func(file, lineno, TCONF,
			 "%s not supported in kernel?", flags_str);
	if (fail == -2 || fail == -3)
		res_func(file, lineno, TCONF,
			 "%s not supported on %s%s filesystem",
			 flags_str, fail == -3 ? "overlayfs over " : "",
			 tst_device->fs_type);
}

#define FANOTIFY_INIT_FLAGS_ERR_MSG(flags, fail) \
	fanotify_flags_err_msg(#flags, __FILE__, __LINE__, tst_res_, (fail))

#define FANOTIFY_MARK_FLAGS_ERR_MSG(mark, fail) \
	fanotify_flags_err_msg((mark)->name, __FILE__, __LINE__, tst_res_, (fail))

#define REQUIRE_FANOTIFY_INIT_FLAGS_SUPPORTED_ON_FS(flags, fname) \
	fanotify_flags_err_msg(#flags, __FILE__, __LINE__, tst_brk_, \
		fanotify_init_flags_supported_on_fs(flags, fname))

#define REQUIRE_FANOTIFY_INIT_FLAGS_SUPPORTED_BY_KERNEL(flags) \
	fanotify_flags_err_msg(#flags, __FILE__, __LINE__, tst_brk_, \
		fanotify_init_flags_supported_by_kernel(flags))

static inline int fanotify_handle_supported_by_kernel(int flag)
{
	/*
	 * On Kernel that does not support AT_HANDLE_FID this will result
	 * with EINVAL. On older kernels, this will result in EBADF.
	 */
	if (name_to_handle_at(-1, "", NULL, NULL, AT_EMPTY_PATH | flag)) {
		if (errno == EINVAL)
			return -1;
	}
	return 0;
}

#define REQUIRE_MARK_TYPE_SUPPORTED_ON_FS(mark_type, fname) \
	fanotify_flags_err_msg(#mark_type, __FILE__, __LINE__, tst_brk_, \
		fanotify_mark_supported_on_fs(mark_type, fname))

#define REQUIRE_HANDLE_TYPE_SUPPORTED_BY_KERNEL(handle_type) \
	fanotify_flags_err_msg(#handle_type, __FILE__, __LINE__, tst_brk_, \
		fanotify_handle_supported_by_kernel(handle_type))

#define REQUIRE_FANOTIFY_EVENTS_SUPPORTED_ON_FS(init_flags, mark_type, mask, fname) do { \
	if (mark_type)							\
		REQUIRE_MARK_TYPE_SUPPORTED_ON_FS(mark_type, fname);	\
	if (init_flags)							\
		REQUIRE_FANOTIFY_INIT_FLAGS_SUPPORTED_ON_FS(init_flags, fname); \
	fanotify_flags_err_msg(#mask, __FILE__, __LINE__, tst_brk_, \
		fanotify_flags_supported_on_fs(init_flags, mark_type, mask, fname)); \
} while (0)

static inline struct fanotify_event_info_header *get_event_info(
					struct fanotify_event_metadata *event,
					int info_type)
{
	struct fanotify_event_info_header *hdr = NULL;
	char *start = (char *) event;
	int off;

	for (off = event->metadata_len; (off+sizeof(*hdr)) < event->event_len;
	     off += hdr->len) {
		hdr = (struct fanotify_event_info_header *) &(start[off]);
		if (hdr->info_type == info_type)
			return hdr;
	}
	return NULL;
}

#define get_event_info_error(event)					\
	((struct fanotify_event_info_error *)				\
	 get_event_info((event), FAN_EVENT_INFO_TYPE_ERROR))

#define get_event_info_fid(event)					\
	((struct fanotify_event_info_fid *)				\
	 get_event_info((event), FAN_EVENT_INFO_TYPE_FID))

#endif /* __FANOTIFY_H__ */
