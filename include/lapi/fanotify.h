/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2012-2022 Linux Test Project.  All Rights Reserved.
 * Author: Jan Kara, November 2013
 */

#ifndef	LAPI_FANOTIFY_H__
#define	LAPI_FANOTIFY_H__

#include "config.h"
#include <sys/fanotify.h>
#include <linux/types.h>

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
#ifndef FAN_REPORT_TARGET_FID
#define FAN_REPORT_TARGET_FID	0x00001000
#define FAN_REPORT_DFID_NAME_TARGET (FAN_REPORT_DFID_NAME | \
				     FAN_REPORT_FID | FAN_REPORT_TARGET_FID)
#endif
#ifndef FAN_REPORT_FD_ERROR
#define FAN_REPORT_FD_ERROR	0x00002000
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
#ifndef FAN_MARK_EVICTABLE
#define FAN_MARK_EVICTABLE	0x00000200
#endif
#ifndef FAN_MARK_IGNORE
#define FAN_MARK_IGNORE		0x00000400
#endif
#ifndef FAN_MARK_IGNORE_SURV
#define FAN_MARK_IGNORE_SURV	(FAN_MARK_IGNORE | FAN_MARK_IGNORED_SURV_MODIFY)
#endif
/* Non-uapi convenience macros */
#ifndef FAN_MARK_IGNORED_SURV
#define FAN_MARK_IGNORED_SURV	(FAN_MARK_IGNORED_MASK | \
				FAN_MARK_IGNORED_SURV_MODIFY)
#endif
#ifndef FAN_MARK_PARENT
#define FAN_MARK_PARENT		FAN_MARK_ONLYDIR
#endif
#ifndef FAN_MARK_SUBDIR
#define FAN_MARK_SUBDIR		FAN_MARK_ONLYDIR
#endif
#ifndef FAN_MARK_TYPES
#define FAN_MARK_TYPES (FAN_MARK_INODE | FAN_MARK_MOUNT | FAN_MARK_FILESYSTEM)
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
#ifndef FAN_FS_ERROR
#define FAN_FS_ERROR		0x00008000
#endif
#ifndef FAN_PRE_ACCESS
#define FAN_PRE_ACCESS		0x00100000
#endif
#ifndef FAN_RENAME
#define FAN_RENAME		0x10000000
#endif

/* Additional error status codes that can be returned to userspace */
#ifndef FAN_NOPIDFD
#define FAN_NOPIDFD		-1
#endif
#ifndef FAN_EPIDFD
#define FAN_EPIDFD		-2
#endif

/* errno other than EPERM can specified in upper byte of deny response */
#ifndef FAN_DENY_ERRNO
#define FAN_ERRNO(err) (((((__u32)(err)) & 0xff) << 24))
#define FAN_DENY_ERRNO(err) (FAN_DENY | FAN_ERRNO(err))
#endif

#ifndef FAN_RESPONSE_ERRNO
#define FAN_RESPONSE_ERRNO(res) ((int)((res) >> 24))
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

#define LTP_PRE_CONTENT_EVENTS	(FAN_PRE_ACCESS)

struct fanotify_group_type {
	unsigned int flag;
	const char *name;
};

struct fanotify_mark_type {
	unsigned int flag;
	const char *name;
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
#ifndef FAN_EVENT_INFO_TYPE_ERROR
#define FAN_EVENT_INFO_TYPE_ERROR	5
#endif
#ifndef FAN_EVENT_INFO_TYPE_RANGE
#define FAN_EVENT_INFO_TYPE_RANGE	6
#endif

#ifndef FAN_EVENT_INFO_TYPE_OLD_DFID_NAME
#define FAN_EVENT_INFO_TYPE_OLD_DFID_NAME	10
#endif
#ifndef FAN_EVENT_INFO_TYPE_NEW_DFID_NAME
#define FAN_EVENT_INFO_TYPE_NEW_DFID_NAME	12
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

#ifndef HAVE_STRUCT_FANOTIFY_EVENT_INFO_ERROR
struct fanotify_event_info_error {
	struct fanotify_event_info_header hdr;
	__s32 error;
	__u32 error_count;
};
#endif /* HAVE_STRUCT_FANOTIFY_EVENT_INFO_ERROR */

#ifndef HAVE_STRUCT_FANOTIFY_EVENT_INFO_RANGE
struct fanotify_event_info_range {
	struct fanotify_event_info_header hdr;
	__u32 pad;
	__u64 offset;
	__u64 count;
};
#endif /* HAVE_STRUCT_FANOTIFY_EVENT_INFO_RANGE */

/* NOTE: only for struct fanotify_event_info_fid */
#ifdef HAVE_STRUCT_FANOTIFY_EVENT_INFO_FID_FSID___VAL
# define FSID_VAL_MEMBER(fsid, i) (fsid.__val[i])
#else
# define FSID_VAL_MEMBER(fsid, i) (fsid.val[i])
#endif /* HAVE_STRUCT_FANOTIFY_EVENT_INFO_FID_FSID___VAL */

/* linux/exportfs.h */
#ifndef FILEID_INVALID
# define FILEID_INVALID		0xff
#endif

#endif /* LAPI_FANOTIFY_H__ */
