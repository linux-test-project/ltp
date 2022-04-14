// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 CTERA Networks. All Rights Reserved.
 *
 * Started by Amir Goldstein <amir73il@gmail.com>
 */

/*\
 * [Description]
 * Check fanotify directory entry modification events, events on child and
 * on self with group init flags:
 *
 * - FAN_REPORT_DFID_NAME (dir fid + name)
 * - FAN_REPORT_DIR_FID   (dir fid)
 * - FAN_REPORT_DIR_FID | FAN_REPORT_FID   (dir fid + child fid)
 * - FAN_REPORT_DFID_NAME | FAN_REPORT_FID (dir fid + name + child fid)
 * - FAN_REPORT_DFID_NAME_TARGET (dir fid + name + created/deleted file fid)
 */

#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include "tst_test.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define EVENT_MAX 20

/* Size of the event structure, not including file handle */
#define EVENT_SIZE (sizeof(struct fanotify_event_metadata) + \
		    sizeof(struct fanotify_event_info_fid))

/* Tripple events buffer size to account for file handles and names */
#define EVENT_BUF_LEN (EVENT_MAX * EVENT_SIZE * 3)

#define BUF_SIZE 256

#ifdef HAVE_NAME_TO_HANDLE_AT
struct event_t {
	unsigned long long mask;
	struct fanotify_fid_t *fid;
	struct fanotify_fid_t *child_fid;
	char name[BUF_SIZE];
	char name2[BUF_SIZE];
	char *old_name;
	char *new_name;
};

static char fname1[BUF_SIZE + 11], fname2[BUF_SIZE + 11];
static char dname1[BUF_SIZE], dname2[BUF_SIZE], tmpdir[BUF_SIZE];
static int fd_notify;

static struct event_t event_set[EVENT_MAX];

static char event_buf[EVENT_BUF_LEN];

#define DIR_NAME1 "test_dir1"
#define DIR_NAME2 "test_dir2"
#define FILE_NAME1 "test_file1"
#define FILE_NAME2 "test_file2"
#define MOUNT_PATH "fs_mnt"
#define TEMP_DIR MOUNT_PATH "/temp_dir"

static int fan_report_target_fid_unsupported;
static int rename_events_unsupported;

static struct test_case_t {
	const char *tname;
	struct fanotify_group_type group;
	struct fanotify_mark_type mark;
	unsigned long mask;
	struct fanotify_mark_type sub_mark;
	unsigned long sub_mask;
	unsigned long tmpdir_ignored_mask;
} test_cases[] = {
	{
		"FAN_REPORT_DFID_NAME monitor filesystem for create/delete/move/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_NAME),
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR,
		/* Mount watch for events possible on children */
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_OPEN | FAN_CLOSE | FAN_ONDIR,
		0,
	},
	{
		"FAN_REPORT_DFID_NAME monitor directories for create/delete/move/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_NAME),
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_ONDIR,
		/* Watches for self events on subdir and events on subdir's children */
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR |
		FAN_OPEN | FAN_CLOSE | FAN_EVENT_ON_CHILD,
		0,
	},
	{
		"FAN_REPORT_DIR_FID monitor filesystem for create/delete/move/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DIR_FID),
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR,
		/* Mount watch for events possible on children */
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_OPEN | FAN_CLOSE | FAN_ONDIR,
		0,
	},
	{
		"FAN_REPORT_DIR_FID monitor directories for create/delete/move/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DIR_FID),
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_ONDIR,
		/* Watches for self events on subdir and events on subdir's children */
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR |
		FAN_OPEN | FAN_CLOSE | FAN_EVENT_ON_CHILD,
		0,
	},
	{
		"FAN_REPORT_DFID_FID monitor filesystem for create/delete/move/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_FID),
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR,
		/* Mount watch for events possible on children */
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_OPEN | FAN_CLOSE | FAN_ONDIR,
		0,
	},
	{
		"FAN_REPORT_DFID_FID monitor directories for create/delete/move/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_FID),
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_ONDIR,
		/* Watches for self events on subdir and events on subdir's children */
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR |
		FAN_OPEN | FAN_CLOSE | FAN_EVENT_ON_CHILD,
		0,
	},
	{
		"FAN_REPORT_DFID_NAME_FID monitor filesystem for create/delete/move/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_NAME_FID),
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR,
		/* Mount watch for events possible on children */
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_OPEN | FAN_CLOSE | FAN_ONDIR,
		0,
	},
	{
		"FAN_REPORT_DFID_NAME_FID monitor directories for create/delete/move/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_NAME_FID),
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_ONDIR,
		/* Watches for self events on subdir and events on subdir's children */
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR |
		FAN_OPEN | FAN_CLOSE | FAN_EVENT_ON_CHILD,
		0,
	},
	{
		"FAN_REPORT_DFID_NAME_TARGET monitor filesystem for create/delete/move/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_NAME_TARGET),
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR,
		/* Mount watch for events possible on children */
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_OPEN | FAN_CLOSE | FAN_ONDIR,
		0,
	},
	{
		"FAN_REPORT_DFID_NAME_TARGET monitor directories for create/delete/move/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_NAME_TARGET),
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_ONDIR,
		/* Watches for self events on subdir and events on subdir's children */
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR |
		FAN_OPEN | FAN_CLOSE | FAN_EVENT_ON_CHILD,
		0,
	},
	{
		"FAN_REPORT_DFID_NAME_FID monitor filesystem for create/delete/move/rename/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_NAME_FID),
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_RENAME | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR,
		/* Mount watch for events possible on children */
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_OPEN | FAN_CLOSE | FAN_ONDIR,
		0,
	},
	{
		"FAN_REPORT_DFID_NAME_FID monitor directories for create/delete/move/rename/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_NAME_FID),
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_RENAME | FAN_ONDIR,
		/* Watches for self events on subdir and events on subdir's children */
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_RENAME | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR |
		FAN_OPEN | FAN_CLOSE | FAN_EVENT_ON_CHILD,
		0,
	},
	{
		"FAN_REPORT_DFID_NAME_TARGET monitor filesystem for create/delete/move/rename/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_NAME_TARGET),
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_RENAME | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR,
		/* Mount watch for events possible on children */
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_OPEN | FAN_CLOSE | FAN_ONDIR,
		0,
	},
	{
		"FAN_REPORT_DFID_NAME_TARGET monitor directories for create/delete/move/rename/open/close",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_NAME_TARGET),
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_RENAME | FAN_ONDIR,
		/* Watches for self events on subdir and events on subdir's children */
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_RENAME | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR |
		FAN_OPEN | FAN_CLOSE | FAN_EVENT_ON_CHILD,
		0,
	},
	{
		"FAN_REPORT_DFID_NAME_FID monitor directories and ignore FAN_RENAME events to/from temp directory",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_NAME_FID),
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_RENAME | FAN_ONDIR,
		/* Watches for self events on subdir and events on subdir's children */
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_RENAME | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR |
		FAN_OPEN | FAN_CLOSE | FAN_EVENT_ON_CHILD,
		/* Ignore FAN_RENAME to/from tmpdir */
		FAN_MOVE | FAN_RENAME,
	},
	{
		"FAN_REPORT_DFID_NAME_FID monitor filesystem and ignore FAN_RENAME events to/from temp directory",
		INIT_FANOTIFY_GROUP_TYPE(REPORT_DFID_NAME_FID),
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_CREATE | FAN_DELETE | FAN_MOVE | FAN_RENAME | FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR,
		/* Mount watch for events possible on children */
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_OPEN | FAN_CLOSE | FAN_ONDIR,
		/* Ignore FAN_RENAME to/from tmpdir */
		FAN_MOVE | FAN_RENAME,
	},
};

static void do_test(unsigned int number)
{
	int fd, dirfd, len = 0, i = 0, test_num = 0, tst_count = 0;
	struct test_case_t *tc = &test_cases[number];
	struct fanotify_group_type *group = &tc->group;
	struct fanotify_mark_type *mark = &tc->mark;
	struct fanotify_mark_type *sub_mark = &tc->sub_mark;
	struct fanotify_fid_t root_fid, dir_fid, file_fid;
	struct fanotify_fid_t *child_fid = NULL, *subdir_fid = NULL;
	int report_name = (group->flag & FAN_REPORT_NAME);
	int report_target_fid = (group->flag & FAN_REPORT_TARGET_FID);
	int report_rename = (tc->mask & FAN_RENAME);
	int fs_mark = (mark->flag == FAN_MARK_FILESYSTEM);
	int rename_ignored = (tc->tmpdir_ignored_mask & FAN_RENAME);

	tst_res(TINFO, "Test #%d: %s", number, tc->tname);

	if (report_rename && rename_events_unsupported) {
		tst_res(TCONF, "FAN_RENAME not supported in kernel?");
		return;
	}

	if (fan_report_target_fid_unsupported && report_target_fid) {
		FANOTIFY_INIT_FLAGS_ERR_MSG(FAN_REPORT_TARGET_FID,
					    fan_report_target_fid_unsupported);
		return;
	}

	fd_notify = SAFE_FANOTIFY_INIT(group->flag, 0);

	/*
	 * Watch dir modify events with name in filesystem/dir
	 */
	SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD | mark->flag, tc->mask,
			   AT_FDCWD, MOUNT_PATH);

	/* Save the mount root fid */
	fanotify_save_fid(MOUNT_PATH, &root_fid);

	/*
	 * Create subdir and watch open events "on children" with name.
	 * Make it a mount root.
	 */
	SAFE_MKDIR(dname1, 0755);
	SAFE_MOUNT(dname1, dname1, "none", MS_BIND, NULL);

	/* Save the subdir fid */
	fanotify_save_fid(dname1, &dir_fid);
	/* With FAN_REPORT_TARGET_FID, report subdir fid also for dirent events */
	if (report_target_fid)
		subdir_fid = &dir_fid;

	if (tc->sub_mask)
		SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD | sub_mark->flag,
				   tc->sub_mask, AT_FDCWD, dname1);
	/*
	 * ignore FAN_RENAME to/from tmpdir, so we won't get the FAN_RENAME events
	 * when subdir is moved via tmpdir.
	 * FAN_MOVE is also set in ignored mark of tmpdir, but it will have no effect
	 * and the MOVED_FROM/TO events will still be reported.
	 */
	if (tc->tmpdir_ignored_mask)
		SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD |
				   FAN_MARK_IGNORED_MASK |
				   FAN_MARK_IGNORED_SURV_MODIFY,
				   tc->tmpdir_ignored_mask, AT_FDCWD, TEMP_DIR);

	memset(event_set, 0, sizeof(event_set));
	event_set[tst_count].mask = FAN_CREATE | FAN_ONDIR;
	event_set[tst_count].fid = &root_fid;
	event_set[tst_count].child_fid = subdir_fid;
	strcpy(event_set[tst_count].name, DIR_NAME1);
	tst_count++;

	/* Generate modify events "on child" */
	fd = SAFE_CREAT(fname1, 0755);

	/* Save the file fid */
	fanotify_save_fid(fname1, &file_fid);
	/* With FAN_REPORT_TARGET_FID, report child fid also for dirent events */
	if (report_target_fid)
		child_fid = &file_fid;

	SAFE_WRITE(1, fd, "1", 1);
	SAFE_RENAME(fname1, fname2);

	SAFE_CLOSE(fd);

	/* Generate delete events with fname2 */
	SAFE_UNLINK(fname2);

	/* Read events on files in subdir */
	len += SAFE_READ(0, fd_notify, event_buf + len, EVENT_BUF_LEN - len);

	/*
	 * FAN_CREATE|FAN_DELETE|FAN_MOVE events with the same name are merged.
	 */
	event_set[tst_count].mask = FAN_CREATE | FAN_MOVED_FROM;
	event_set[tst_count].fid = &dir_fid;
	event_set[tst_count].child_fid = child_fid;
	strcpy(event_set[tst_count].name, FILE_NAME1);
	tst_count++;
	/*
	 * Event on non-dir child with the same name may be merged with the
	 * directory entry modification events above, unless FAN_REPORT_FID is
	 * set and child fid is reported. If FAN_REPORT_FID is set but
	 * FAN_REPORT_NAME is not set, then FAN_CREATE above is merged with
	 * FAN_DELETE below and FAN_OPEN will be merged with FAN_CLOSE.
	 */
	if (report_name) {
		event_set[tst_count].mask = FAN_OPEN;
		event_set[tst_count].fid = &dir_fid;
		event_set[tst_count].child_fid = &file_fid;
		strcpy(event_set[tst_count].name, FILE_NAME1);
		tst_count++;
	}
	/*
	 * FAN_RENAME event is independent of MOVED_FROM/MOVED_TO and not merged
	 * with any other event because it has different info records.
	 */
	if (report_rename) {
		event_set[tst_count].mask = FAN_RENAME;
		event_set[tst_count].fid = &dir_fid;
		event_set[tst_count].child_fid = child_fid;
		strcpy(event_set[tst_count].name, FILE_NAME1);
		strcpy(event_set[tst_count].name2, FILE_NAME2);
		event_set[tst_count].old_name = event_set[tst_count].name;
		event_set[tst_count].new_name = event_set[tst_count].name2;
		tst_count++;
	}

	event_set[tst_count].mask = FAN_DELETE | FAN_MOVED_TO;
	/*
	 * With FAN_REPORT_TARGET_FID, close of FILE_NAME2 is merged with
	 * moved_to and delete events, because they all have parent and
	 * child fid records.
	 */
	if (report_target_fid)
		event_set[tst_count].mask |= FAN_CLOSE_WRITE;
	event_set[tst_count].fid = &dir_fid;
	event_set[tst_count].child_fid = child_fid;
	strcpy(event_set[tst_count].name, FILE_NAME2);
	tst_count++;
	/*
	 * When not reporting name, open of FILE_NAME1 is merged
	 * with close of FILE_NAME2.
	 */
	if (!report_name) {
		event_set[tst_count].mask = FAN_OPEN | FAN_CLOSE_WRITE;
		event_set[tst_count].fid = &dir_fid;
		event_set[tst_count].child_fid = &file_fid;
		strcpy(event_set[tst_count].name, "");
		tst_count++;
	}
	/*
	 * Directory watch does not get self events on children.
	 * Filesystem watch gets self event w/o name info if FAN_REPORT_FID
	 * is set.
	 */
	if (fs_mark && (group->flag & FAN_REPORT_FID)) {
		event_set[tst_count].mask = FAN_DELETE_SELF | FAN_MOVE_SELF;
		event_set[tst_count].fid = &file_fid;
		event_set[tst_count].child_fid = NULL;
		strcpy(event_set[tst_count].name, "");
		tst_count++;
	}
	/*
	 * Without FAN_REPORT_TARGET_FID, close of FILE_NAME2 is not merged with
	 * open of FILE_NAME1 and it is received after the merged self events.
	 */
	if (report_name && !report_target_fid) {
		event_set[tst_count].mask = FAN_CLOSE_WRITE;
		event_set[tst_count].fid = &dir_fid;
		event_set[tst_count].child_fid = &file_fid;
		strcpy(event_set[tst_count].name, FILE_NAME2);
		tst_count++;
	}

	dirfd = SAFE_OPEN(dname1, O_RDONLY | O_DIRECTORY);
	SAFE_CLOSE(dirfd);

	SAFE_UMOUNT(dname1);

	/*
	 * Directory watch gets open/close events on itself and on its subdirs.
	 * Filesystem watch gets open/close event on all directories with name ".".
	 */
	event_set[tst_count].mask = FAN_OPEN | FAN_CLOSE_NOWRITE | FAN_ONDIR;
	event_set[tst_count].fid = &dir_fid;
	event_set[tst_count].child_fid = NULL;
	strcpy(event_set[tst_count].name, ".");
	tst_count++;
	/*
	 * Directory watch gets self event on itself and filesystem watch gets
	 * self event on all directories with name ".".
	 */
	event_set[tst_count].mask = FAN_DELETE_SELF | FAN_MOVE_SELF | FAN_ONDIR;
	event_set[tst_count].fid = &dir_fid;
	event_set[tst_count].child_fid = NULL;
	strcpy(event_set[tst_count].name, ".");
	tst_count++;

	/*
	 * If only root dir and subdir are watched, a rename via an unwatched tmpdir
	 * will observe the same MOVED_FROM/MOVED_TO events as a direct rename,
	 * but will observe 2 FAN_RENAME events with 1 info dir+name record each
	 * instead of 1 FAN_RENAME event with 2 dir+name info records.
	 *
	 * If tmpdir is ignoring FAN_RENAME, we will get the MOVED_FROM/MOVED_TO
	 * events and will not get the FAN_RENAME event for rename via tmpdir.
	 */
	if (!fs_mark || rename_ignored) {
		SAFE_RENAME(dname1, tmpdir);
		SAFE_RENAME(tmpdir, dname2);
	} else {
		SAFE_RENAME(dname1, dname2);
	}
	SAFE_RMDIR(dname2);

	/* Read more events on dirs */
	len += SAFE_READ(0, fd_notify, event_buf + len, EVENT_BUF_LEN - len);

	/*
	 * FAN_RENAME event is independent of MOVED_FROM/MOVED_TO and not merged
	 * with any other event because it has different info records.
	 * When renamed via an unwatched tmpdir, the 1st FAN_RENAME event has the
	 * info record of root_fid+DIR_NAME1 and the 2nd FAN_RENAME event has the
	 * info record of root_fid+DIR_NAME2.
	 * If tmpdir is ignoring FAN_RENAME, we get no FAN_RENAME events at all.
	 */
	if (report_rename && !rename_ignored) {
		event_set[tst_count].mask = FAN_RENAME | FAN_ONDIR;
		event_set[tst_count].fid = &root_fid;
		event_set[tst_count].child_fid = subdir_fid;
		strcpy(event_set[tst_count].name, DIR_NAME1);
		event_set[tst_count].old_name = event_set[tst_count].name;
		if (fs_mark) {
			strcpy(event_set[tst_count].name2, DIR_NAME2);
			event_set[tst_count].new_name = event_set[tst_count].name2;
		}
		tst_count++;
	}
	event_set[tst_count].mask = FAN_MOVED_FROM | FAN_ONDIR;
	event_set[tst_count].fid = &root_fid;
	event_set[tst_count].child_fid = subdir_fid;
	strcpy(event_set[tst_count].name, DIR_NAME1);
	tst_count++;
	if (report_rename && !fs_mark && !rename_ignored) {
		event_set[tst_count].mask = FAN_RENAME | FAN_ONDIR;
		event_set[tst_count].fid = &root_fid;
		event_set[tst_count].child_fid = subdir_fid;
		strcpy(event_set[tst_count].name, DIR_NAME2);
		event_set[tst_count].new_name = event_set[tst_count].name;
		tst_count++;
	}
	event_set[tst_count].mask = FAN_DELETE | FAN_MOVED_TO | FAN_ONDIR;
	event_set[tst_count].fid = &root_fid;
	event_set[tst_count].child_fid = subdir_fid;
	strcpy(event_set[tst_count].name, DIR_NAME2);
	tst_count++;
	/* Expect no more events */
	event_set[tst_count].mask = 0;

	/*
	 * Cleanup the marks
	 */
	SAFE_CLOSE(fd_notify);
	fd_notify = -1;

	while (i < len) {
		struct event_t *expected = &event_set[test_num];
		struct fanotify_event_metadata *event;
		struct fanotify_event_info_fid *event_fid;
		struct fanotify_event_info_fid *child_fid;
		struct fanotify_fid_t *expected_fid = expected->fid;
		struct fanotify_fid_t *expected_child_fid = expected->child_fid;
		struct file_handle *file_handle;
		unsigned int fhlen;
		const char *filename;
		int namelen, info_type, mask_match, info_id = 0;

		event = (struct fanotify_event_metadata *)&event_buf[i];
		event_fid = (struct fanotify_event_info_fid *)(event + 1);
		file_handle = (struct file_handle *)event_fid->handle;
		fhlen = file_handle->handle_bytes;
		filename = (char *)file_handle->f_handle + fhlen;
		child_fid = (void *)((char *)event_fid + event_fid->hdr.len);
		namelen = (char *)child_fid - (char *)filename;
		/* End of event_fid could have name, zero padding, both or none */
		if (namelen > 0) {
			namelen = strlen(filename);
		} else {
			filename = "";
			namelen = 0;
		}
		/* Is there a child fid after first fid record? */
		if (((char *)child_fid - (char *)event) >= event->event_len)
			child_fid = NULL;

		if (!(group->flag & FAN_REPORT_FID))
			expected_child_fid = NULL;

		if (!report_name)
			expected->name[0] = 0;

		if (expected->mask & FAN_RENAME) {
			/* If old name is not reported, first record is new name */
			info_type = expected->old_name ?
				FAN_EVENT_INFO_TYPE_OLD_DFID_NAME :
				FAN_EVENT_INFO_TYPE_NEW_DFID_NAME;
			/* The 2nd fid is same as 1st becaue we rename in same parent */
			if (expected->name2[0])
				expected_child_fid = expected_fid;
		} else if (expected->name[0]) {
			info_type = FAN_EVENT_INFO_TYPE_DFID_NAME;
		} else if (expected->mask & FAN_ONDIR) {
			info_type = FAN_EVENT_INFO_TYPE_DFID;
		} else if (expected->mask & (FAN_DELETE_SELF | FAN_MOVE_SELF)) {
			/* Self event on non-dir has only child fid */
			info_type = FAN_EVENT_INFO_TYPE_FID;
		} else {
			info_type = FAN_EVENT_INFO_TYPE_DFID;
		}

		/*
		 * Event may contain more than the expected mask, but it must
		 * have all the bits in expected mask.
		 * Expected event on dir must not get event on non dir and the
		 * other way around.
		 */
		mask_match = ((event->mask & expected->mask) &&
			      !(expected->mask & ~event->mask) &&
			      !((event->mask ^ expected->mask) & FAN_ONDIR));

check_match:
		if (test_num >= tst_count) {
			tst_res(TFAIL,
				"got unnecessary event: mask=%llx "
				"pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd, filename,
				event->event_len, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		} else if (!fhlen || namelen < 0) {
			tst_res(TFAIL,
				"got event without fid: mask=%llx pid=%u fd=%d, "
				"len=%d info_type=%d info_len=%d fh_len=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd,
				event->event_len, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		} else if (!mask_match) {
			tst_res(TFAIL,
				"got event: mask=%llx (expected %llx) "
				"pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d",
				(unsigned long long)event->mask, expected->mask,
				(unsigned int)event->pid, event->fd, filename,
				event->event_len, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		} else if (info_type != event_fid->hdr.info_type) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u fd=%d, "
				"len=%d info_type=%d expected(%d) info_len=%d fh_len=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd,
				event->event_len, event_fid->hdr.info_type,
				info_type, event_fid->hdr.len, fhlen);
		} else if (fhlen != expected_fid->handle.handle_bytes) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d expected(%d) "
				"fh_type=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd, filename,
				event->event_len, info_type,
				event_fid->hdr.len, fhlen,
				expected_fid->handle.handle_bytes,
				file_handle->handle_type);
		} else if (file_handle->handle_type !=
			   expected_fid->handle.handle_type) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d "
				"fh_type=%d expected(%x)",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd, filename,
				event->event_len, info_type,
				event_fid->hdr.len, fhlen,
				file_handle->handle_type,
				expected_fid->handle.handle_type);
		} else if (memcmp(file_handle->f_handle,
				  expected_fid->handle.f_handle, fhlen)) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d "
				"fh_type=%d unexpected file handle (%x...)",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd, filename,
				event->event_len, info_type,
				event_fid->hdr.len, fhlen,
				file_handle->handle_type,
				*(int *)(file_handle->f_handle));
		} else if (memcmp(&event_fid->fsid, &expected_fid->fsid,
				  sizeof(event_fid->fsid)) != 0) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d "
				"fsid=%x.%x (expected %x.%x)",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd, filename,
				event->event_len, info_type,
				event_fid->hdr.len, fhlen,
				FSID_VAL_MEMBER(event_fid->fsid, 0),
				FSID_VAL_MEMBER(event_fid->fsid, 1),
				expected_fid->fsid.val[0],
				expected_fid->fsid.val[1]);
		} else if (strcmp(expected->name, filename)) {
			tst_res(TFAIL,
				"got event: mask=%llx "
				"pid=%u fd=%d name='%s' expected('%s') "
				"len=%d info_type=%d info_len=%d fh_len=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd,
				filename, expected->name,
				event->event_len, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		} else if (event->pid != getpid()) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u "
				"(expected %u) fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid,
				(unsigned int)getpid(),
				event->fd, filename,
				event->event_len, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		} else if (!!child_fid != !!expected_child_fid) {
			tst_res(TFAIL,
				"got event: mask=%llx "
				"pid=%u fd=%d name='%s' num_info=%d (expected %d) "
				"len=%d info_type=%d info_len=%d fh_len=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd,
				filename, 1 + !!child_fid, 1 + !!expected_child_fid,
				event->event_len, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		} else if (child_fid) {
			tst_res(TINFO,
				"got event #%d: info #%d: info_type=%d info_len=%d fh_len=%d",
				test_num, info_id, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);

			/* Recheck event_fid match with child_fid */
			event_fid = child_fid;
			expected_fid = expected->child_fid;
			info_id = 1;
			info_type = FAN_EVENT_INFO_TYPE_FID;
			/*
			 * With FAN_RENAME event, expect a second record of
			 * type NEW_DFID_NAME, which in our case
			 * has the same fid as the source dir in 1st record.
			 * TODO: check the 2nd name and the 3rd child fid record.
			 */
			if (event->mask & FAN_RENAME && expected->name2[0]) {
				info_type = FAN_EVENT_INFO_TYPE_NEW_DFID_NAME;
				expected_fid = expected->fid;
			}
			file_handle = (struct file_handle *)event_fid->handle;
			fhlen = file_handle->handle_bytes;
			child_fid = NULL;
			expected_child_fid = NULL;
			goto check_match;
		} else {
			tst_res(TPASS,
				"got event #%d: mask=%llx pid=%u fd=%d name='%s' "
				"len=%d; info #%d: info_type=%d info_len=%d fh_len=%d",
				test_num, (unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd, filename,
				event->event_len, info_id, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		}

		if (test_num < tst_count)
			test_num++;

		if (mask_match) {
			/* In case of merged event match next expected mask */
			event->mask &= ~expected->mask | FAN_ONDIR;
			if (event->mask & ~FAN_ONDIR)
				continue;
		}

		i += event->event_len;
		if (event->fd > 0)
			SAFE_CLOSE(event->fd);
	}

	for (; test_num < tst_count; test_num++) {
		tst_res(TFAIL, "didn't get event: mask=%llx, name='%s'",
			 event_set[test_num].mask, event_set[test_num].name);

	}
}

static void setup(void)
{
	REQUIRE_FANOTIFY_INIT_FLAGS_SUPPORTED_ON_FS(FAN_REPORT_DIR_FID, MOUNT_PATH);
	fan_report_target_fid_unsupported =
		fanotify_init_flags_supported_on_fs(FAN_REPORT_DFID_NAME_TARGET, MOUNT_PATH);
	rename_events_unsupported =
		fanotify_events_supported_by_kernel(FAN_RENAME, FAN_REPORT_DFID_NAME, 0);

	SAFE_MKDIR(TEMP_DIR, 0755);
	sprintf(dname1, "%s/%s", MOUNT_PATH, DIR_NAME1);
	sprintf(dname2, "%s/%s", MOUNT_PATH, DIR_NAME2);
	sprintf(tmpdir, "%s/%s", TEMP_DIR, DIR_NAME2);
	sprintf(fname1, "%s/%s", dname1, FILE_NAME1);
	sprintf(fname2, "%s/%s", dname1, FILE_NAME2);
}

static void cleanup(void)
{
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(test_cases),
	.setup = setup,
	.cleanup = cleanup,
	.mount_device = 1,
	.mntpoint = MOUNT_PATH,
	.all_filesystems = 1,
	.needs_root = 1
};

#else
	TST_TEST_TCONF("system does not have required name_to_handle_at() support");
#endif
#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
