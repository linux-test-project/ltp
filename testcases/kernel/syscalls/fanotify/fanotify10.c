// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014 SUSE.  All Rights Reserved.
 * Copyright (c) 2018 CTERA Networks.  All Rights Reserved.
 *
 * Started by Jan Kara <jack@suse.cz>
 * Forked from fanotify06.c by Amir Goldstein <amir73il@gmail.com>
 */

/*\
 * [Description]
 * Check that fanotify properly merges ignore mask of a mount mark
 * with a mask of an inode mark on the same group.  Unlike the
 * prototype test fanotify06, do not use FAN_MODIFY event for the
 * test mask, because it hides the bug.
 */

/*
 * This is a regression test for commit:
 *
 *     9bdda4e9cf2d fsnotify: fix ignore mask logic in fsnotify()
 *
 * Test case #16 is a regression test for commit:
 *
 *     2f02fd3fa13e fanotify: fix ignore mask logic for events on child...
 *
 * Test cases with 'ignored_onchild' are regression tests for commit
 * (from v5.9, unlikely to be backported thus not in .tags):
 *
 *     497b0c5a7c06 fsnotify: send event to parent and child with single...
 */

#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include "tst_test.h"
#include "tst_safe_stdio.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof(struct fanotify_event_metadata))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * EVENT_SIZE)

static unsigned int fanotify_class[] = {
	FAN_CLASS_PRE_CONTENT,
	FAN_CLASS_CONTENT,
	FAN_CLASS_NOTIF,
	/* Reporting dfid+name+fid merges events similar to reporting fd */
	FAN_REPORT_DFID_NAME_FID,
};
#define NUM_CLASSES ARRAY_SIZE(fanotify_class)
#define NUM_PRIORITIES 3

#define GROUPS_PER_PRIO 3

static int fd_notify[NUM_CLASSES][GROUPS_PER_PRIO];
static int fd_syncfs;

static char event_buf[EVENT_BUF_LEN];
static int event_buf_pos, event_buf_len;
static int exec_events_unsupported;
static int fan_report_dfid_unsupported;
static int filesystem_mark_unsupported;
static int evictable_mark_unsupported;
static int ignore_mark_unsupported;

#define MOUNT_PATH "fs_mnt"
#define MNT2_PATH "mntpoint"
#define DIR_NAME "testdir"
#define FILE_NAME "testfile"
#define FILE2_NAME "testfile2"
#define SUBDIR_NAME "testdir2"
#define TEST_APP "fanotify_child"
#define TEST_APP2 "fanotify_child2"
#define DIR_PATH MOUNT_PATH"/"DIR_NAME
#define DIR_PATH_MULTI DIR_PATH"%d"
#define FILE_PATH DIR_PATH"/"FILE_NAME
#define FILE_PATH_MULTI FILE_PATH"%d"
#define FILE_PATH_MULTIDIR DIR_PATH_MULTI"/"FILE_NAME
#define FILE2_PATH DIR_PATH"/"FILE2_NAME
#define SUBDIR_PATH DIR_PATH"/"SUBDIR_NAME
#define FILE_EXEC_PATH MOUNT_PATH"/"TEST_APP
#define FILE2_EXEC_PATH MOUNT_PATH"/"TEST_APP2
#define DIR_MNT2 MNT2_PATH"/"DIR_NAME
#define FILE_MNT2 DIR_MNT2"/"FILE_NAME
#define FILE2_MNT2 DIR_MNT2"/"FILE2_NAME
#define FILE_EXEC_PATH2 MNT2_PATH"/"TEST_APP
#define FILE2_EXEC_PATH2 MNT2_PATH"/"TEST_APP2

#define DROP_CACHES_FILE "/proc/sys/vm/drop_caches"
#define CACHE_PRESSURE_FILE "/proc/sys/vm/vfs_cache_pressure"

static int old_cache_pressure;
static pid_t child_pid;
static int bind_mount_created;
static unsigned int num_classes = NUM_CLASSES;
static int max_file_multi;

enum {
	FANOTIFY_INODE,
	FANOTIFY_PARENT,
	FANOTIFY_SUBDIR,
	FANOTIFY_MOUNT,
	FANOTIFY_FILESYSTEM,
	FANOTIFY_EVICTABLE,
};

static struct fanotify_mark_type fanotify_mark_types[] = {
	INIT_FANOTIFY_MARK_TYPE(INODE),
	INIT_FANOTIFY_MARK_TYPE(PARENT),
	INIT_FANOTIFY_MARK_TYPE(SUBDIR),
	INIT_FANOTIFY_MARK_TYPE(MOUNT),
	INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
	INIT_FANOTIFY_MARK_TYPE(EVICTABLE),
};

static struct tcase {
	const char *tname;
	int mark_path_cnt;
	const char *mark_path_fmt;
	int mark_type;
	int ignore_path_cnt;
	const char *ignore_path_fmt;
	int ignore_mark_type;
	unsigned int ignored_flags;
	int event_path_cnt;
	const char *event_path_fmt;
	unsigned long long expected_mask_with_ignore;
	unsigned long long expected_mask_without_ignore;
} tcases[] = {
	{
		.tname = "ignore mount events created on a specific file",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_MOUNT,
		.ignore_path_fmt = FILE_MNT2,
		.ignore_mark_type = FANOTIFY_INODE,
		.event_path_fmt = FILE_PATH,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "ignore exec mount events created on a specific file",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_MOUNT,
		.ignore_path_fmt = FILE_EXEC_PATH2,
		.ignore_mark_type = FANOTIFY_INODE,
		.event_path_fmt = FILE_EXEC_PATH,
		.expected_mask_with_ignore = FAN_OPEN_EXEC,
		.expected_mask_without_ignore = FAN_OPEN | FAN_OPEN_EXEC
	},
	{
		.tname = "don't ignore mount events created on another file",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_MOUNT,
		.ignore_path_fmt = FILE_PATH,
		.ignore_mark_type = FANOTIFY_INODE,
		.event_path_fmt = FILE2_PATH,
		.expected_mask_with_ignore = FAN_OPEN,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "don't ignore exec mount events created on another file",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_MOUNT,
		.ignore_path_fmt = FILE_EXEC_PATH,
		.ignore_mark_type = FANOTIFY_INODE,
		.event_path_fmt = FILE2_EXEC_PATH,
		.expected_mask_with_ignore = FAN_OPEN | FAN_OPEN_EXEC,
		.expected_mask_without_ignore = FAN_OPEN | FAN_OPEN_EXEC
	},
	{
		.tname = "ignore inode events created on a specific mount point",
		.mark_path_fmt = FILE_PATH,
		.mark_type = FANOTIFY_INODE,
		.ignore_path_fmt = MNT2_PATH,
		.ignore_mark_type = FANOTIFY_MOUNT,
		.event_path_fmt = FILE_MNT2,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "ignore exec inode events created on a specific mount point",
		.mark_path_fmt = FILE_EXEC_PATH,
		.mark_type = FANOTIFY_INODE,
		.ignore_path_fmt = MNT2_PATH,
		.ignore_mark_type = FANOTIFY_MOUNT,
		.event_path_fmt = FILE_EXEC_PATH2,
		.expected_mask_with_ignore = FAN_OPEN_EXEC,
		.expected_mask_without_ignore = FAN_OPEN | FAN_OPEN_EXEC
	},
	{
		.tname = "don't ignore inode events created on another mount point",
		.mark_path_fmt = FILE_MNT2,
		.mark_type = FANOTIFY_INODE,
		.ignore_path_fmt = MNT2_PATH,
		.ignore_mark_type = FANOTIFY_MOUNT,
		.event_path_fmt = FILE_PATH,
		.expected_mask_with_ignore = FAN_OPEN,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "don't ignore exec inode events created on another mount point",
		.mark_path_fmt = FILE_EXEC_PATH2,
		.mark_type = FANOTIFY_INODE,
		.ignore_path_fmt = MNT2_PATH,
		.ignore_mark_type = FANOTIFY_MOUNT,
		.event_path_fmt = FILE_EXEC_PATH,
		.expected_mask_with_ignore = FAN_OPEN | FAN_OPEN_EXEC,
		.expected_mask_without_ignore = FAN_OPEN | FAN_OPEN_EXEC
	},
	{
		.tname = "ignore fs events created on a specific file",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_FILESYSTEM,
		.ignore_path_fmt = FILE_PATH,
		.ignore_mark_type = FANOTIFY_INODE,
		.event_path_fmt = FILE_PATH,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "ignore exec fs events created on a specific file",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_FILESYSTEM,
		.ignore_path_fmt = FILE_EXEC_PATH,
		.ignore_mark_type = FANOTIFY_INODE,
		.event_path_fmt = FILE_EXEC_PATH,
		.expected_mask_with_ignore = FAN_OPEN_EXEC,
		.expected_mask_without_ignore = FAN_OPEN | FAN_OPEN_EXEC
	},
	{
		.tname = "don't ignore mount events created on another file",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_FILESYSTEM,
		.ignore_path_fmt = FILE_PATH,
		.ignore_mark_type = FANOTIFY_INODE,
		.event_path_fmt = FILE2_PATH,
		.expected_mask_with_ignore = FAN_OPEN,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "don't ignore exec mount events created on another file",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_FILESYSTEM,
		.ignore_path_fmt = FILE_EXEC_PATH,
		.ignore_mark_type = FANOTIFY_INODE,
		.event_path_fmt = FILE2_EXEC_PATH,
		.expected_mask_with_ignore = FAN_OPEN | FAN_OPEN_EXEC,
		.expected_mask_without_ignore = FAN_OPEN | FAN_OPEN_EXEC
	},
	{
		.tname = "ignore fs events created on a specific mount point",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_FILESYSTEM,
		.ignore_path_fmt = MNT2_PATH,
		.ignore_mark_type = FANOTIFY_MOUNT,
		.event_path_fmt = FILE_MNT2,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "ignore exec fs events created on a specific mount point",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_FILESYSTEM,
		.ignore_path_fmt = MNT2_PATH,
		.ignore_mark_type = FANOTIFY_MOUNT,
		.event_path_fmt = FILE_EXEC_PATH2,
		.expected_mask_with_ignore = FAN_OPEN_EXEC,
		.expected_mask_without_ignore = FAN_OPEN | FAN_OPEN_EXEC
	},
	{
		.tname = "don't ignore fs events created on another mount point",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_FILESYSTEM,
		.ignore_path_fmt = MNT2_PATH,
		.ignore_mark_type = FANOTIFY_MOUNT,
		.event_path_fmt = FILE_PATH,
		.expected_mask_with_ignore = FAN_OPEN,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "don't ignore exec fs events created on another mount point",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_FILESYSTEM,
		.ignore_path_fmt = MNT2_PATH,
		.ignore_mark_type = FANOTIFY_MOUNT,
		.event_path_fmt = FILE_EXEC_PATH,
		.expected_mask_with_ignore = FAN_OPEN | FAN_OPEN_EXEC,
		.expected_mask_without_ignore = FAN_OPEN | FAN_OPEN_EXEC
	},
	{
		.tname = "ignore child exec events created on a specific mount point",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_PARENT,
		.ignore_path_fmt = MOUNT_PATH,
		.ignore_mark_type = FANOTIFY_MOUNT,
		.event_path_fmt = FILE_EXEC_PATH,
		.expected_mask_with_ignore = FAN_OPEN_EXEC,
		.expected_mask_without_ignore = FAN_OPEN | FAN_OPEN_EXEC
	},
	{
		.tname = "ignore events on children of directory created on a specific file",
		.mark_path_fmt = DIR_PATH,
		.mark_type = FANOTIFY_PARENT,
		.ignore_path_fmt = DIR_PATH,
		.ignore_mark_type = FANOTIFY_PARENT,
		.ignored_flags = FAN_EVENT_ON_CHILD,
		.event_path_fmt = FILE_PATH,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "ignore events on file created inside a parent watching children",
		.mark_path_fmt = FILE_PATH,
		.mark_type = FANOTIFY_INODE,
		.ignore_path_fmt = DIR_PATH,
		.ignore_mark_type = FANOTIFY_PARENT,
		.ignored_flags = FAN_EVENT_ON_CHILD,
		.event_path_fmt = FILE_PATH,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "don't ignore events on file created inside a parent not watching children",
		.mark_path_fmt = FILE_PATH,
		.mark_type = FANOTIFY_INODE,
		.ignore_path_fmt = DIR_PATH,
		.ignore_mark_type = FANOTIFY_PARENT,
		.event_path_fmt = FILE_PATH,
		.expected_mask_with_ignore = FAN_OPEN,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "ignore mount events created inside a parent watching children",
		.mark_path_fmt = FILE_PATH,
		.mark_type = FANOTIFY_MOUNT,
		.ignore_path_fmt = DIR_PATH,
		.ignore_mark_type = FANOTIFY_PARENT,
		.ignored_flags = FAN_EVENT_ON_CHILD,
		.event_path_fmt = FILE_PATH,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "don't ignore mount events created inside a parent not watching children",
		.mark_path_fmt = FILE_PATH,
		.mark_type = FANOTIFY_MOUNT,
		.ignore_path_fmt = DIR_PATH,
		.ignore_mark_type = FANOTIFY_PARENT,
		.event_path_fmt = FILE_PATH,
		.expected_mask_with_ignore = FAN_OPEN,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "ignore fs events created inside a parent watching children",
		.mark_path_fmt = FILE_PATH,
		.mark_type = FANOTIFY_FILESYSTEM,
		.ignore_path_fmt = DIR_PATH,
		.ignore_mark_type = FANOTIFY_PARENT,
		.ignored_flags = FAN_EVENT_ON_CHILD,
		.event_path_fmt = FILE_PATH,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "don't ignore fs events created inside a parent not watching children",
		.mark_path_fmt = FILE_PATH,
		.mark_type = FANOTIFY_FILESYSTEM,
		.ignore_path_fmt = DIR_PATH,
		.ignore_mark_type = FANOTIFY_PARENT,
		.event_path_fmt = FILE_PATH,
		.expected_mask_with_ignore = FAN_OPEN,
		.expected_mask_without_ignore = FAN_OPEN
	},
	/* Evictable ignore mark test cases */
	{
		.tname = "don't ignore mount events created on file with evicted ignore mark",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_MOUNT,
		.ignore_path_cnt = 16,
		.ignore_path_fmt = FILE_PATH_MULTI,
		.ignore_mark_type = FANOTIFY_EVICTABLE,
		.event_path_cnt = 16,
		.event_path_fmt = FILE_PATH_MULTI,
		.expected_mask_with_ignore = FAN_OPEN,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "don't ignore fs events created on a file with evicted ignore mark",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_FILESYSTEM,
		.ignore_path_cnt = 16,
		.ignore_path_fmt = FILE_PATH_MULTI,
		.ignore_mark_type = FANOTIFY_EVICTABLE,
		.event_path_cnt = 16,
		.event_path_fmt = FILE_PATH_MULTI,
		.expected_mask_with_ignore = FAN_OPEN,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "don't ignore mount events created inside a parent with evicted ignore mark",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_MOUNT,
		.ignore_path_cnt = 16,
		.ignore_path_fmt = DIR_PATH_MULTI,
		.ignore_mark_type = FANOTIFY_EVICTABLE,
		.ignored_flags = FAN_EVENT_ON_CHILD,
		.event_path_cnt = 16,
		.event_path_fmt = FILE_PATH_MULTIDIR,
		.expected_mask_with_ignore = FAN_OPEN,
		.expected_mask_without_ignore = FAN_OPEN
	},
	{
		.tname = "don't ignore fs events created inside a parent with evicted ignore mark",
		.mark_path_fmt = MOUNT_PATH,
		.mark_type = FANOTIFY_FILESYSTEM,
		.ignore_path_cnt = 16,
		.ignore_path_fmt = DIR_PATH_MULTI,
		.ignore_mark_type = FANOTIFY_EVICTABLE,
		.ignored_flags = FAN_EVENT_ON_CHILD,
		.event_path_cnt = 16,
		.event_path_fmt = FILE_PATH_MULTIDIR,
		.expected_mask_with_ignore = FAN_OPEN,
		.expected_mask_without_ignore = FAN_OPEN
	},
	/* FAN_MARK_IGNORE specific test cases */
	{
		.tname = "ignore events on subdir inside a parent watching subdirs",
		.mark_path_fmt = SUBDIR_PATH,
		.mark_type = FANOTIFY_SUBDIR,
		.ignore_path_fmt = DIR_PATH,
		.ignore_mark_type = FANOTIFY_PARENT,
		.ignored_flags = FAN_EVENT_ON_CHILD | FAN_ONDIR,
		.event_path_fmt = SUBDIR_PATH,
		.expected_mask_with_ignore = 0,
		.expected_mask_without_ignore = FAN_OPEN | FAN_ONDIR
	},
	{
		.tname = "don't ignore events on subdir inside a parent not watching children",
		.mark_path_fmt = SUBDIR_PATH,
		.mark_type = FANOTIFY_SUBDIR,
		.ignore_path_fmt = DIR_PATH,
		.ignore_mark_type = FANOTIFY_PARENT,
		.ignored_flags = FAN_ONDIR,
		.event_path_fmt = SUBDIR_PATH,
		.expected_mask_with_ignore = FAN_OPEN | FAN_ONDIR,
		.expected_mask_without_ignore = FAN_OPEN | FAN_ONDIR
	},
	{
		.tname = "don't ignore events on subdir inside a parent watching non-dir children",
		.mark_path_fmt = SUBDIR_PATH,
		.mark_type = FANOTIFY_SUBDIR,
		.ignore_path_fmt = DIR_PATH,
		.ignore_mark_type = FANOTIFY_PARENT,
		.ignored_flags = FAN_EVENT_ON_CHILD,
		.event_path_fmt = SUBDIR_PATH,
		.expected_mask_with_ignore = FAN_OPEN | FAN_ONDIR,
		.expected_mask_without_ignore = FAN_OPEN | FAN_ONDIR
	},
};

static int format_path_check(char *buf, const char *fmt, int count, int i)
{
	int limit = count ? : 1;

	if (i >= limit)
		return 0;

	if (count)
		sprintf(buf, fmt, i);
	else
		strcpy(buf, fmt);
	return 1;
}

#define foreach_path(tc, buf, pname) \
	for (int piter = 0; format_path_check((buf), (tc)->pname##_fmt,	\
					(tc)->pname##_cnt, piter); piter++)

static void show_fanotify_ignore_marks(int fd, int min, int max)
{
	unsigned int mflags, mask, ignored_mask;
	char procfdinfo[100];
	char line[BUFSIZ];
	int marks = 0;
	FILE *f;

	sprintf(procfdinfo, "/proc/%d/fdinfo/%d", (int)getpid(), fd);
	f = SAFE_FOPEN(procfdinfo, "r");
	while (fgets(line, BUFSIZ, f)) {
		if (sscanf(line, "fanotify ino:%*x sdev:%*x mflags: %x mask:%x ignored_mask:%x",
			   &mflags, &mask, &ignored_mask) == 3) {
			if (ignored_mask != 0)
				marks++;
		}
	}
	if (marks < min) {
		tst_res(TFAIL, "Found %d ignore marks but at least %d expected", marks, min);
		return;
	}
	if (marks > max) {
		tst_res(TFAIL, "Found %d ignore marks but at most %d expected", marks, max);
		return;
	}
	tst_res(TPASS, "Found %d ignore marks which is in expected range %d-%d", marks, min, max);
}

static void drop_caches(void)
{
	if (syncfs(fd_syncfs) < 0)
		tst_brk(TBROK | TERRNO, "Unexpected error when syncing filesystem");

	SAFE_FILE_PRINTF(DROP_CACHES_FILE, "3");
}

static int create_fanotify_groups(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct fanotify_mark_type *mark, *ignore_mark;
	unsigned int mark_ignored, mask;
	unsigned int p, i;
	int evictable_ignored = (tc->ignore_mark_type == FANOTIFY_EVICTABLE);
	int ignore_mark_type;
	int ignored_onchild = tc->ignored_flags & FAN_EVENT_ON_CHILD;
	char path[PATH_MAX];

	mark = &fanotify_mark_types[tc->mark_type];
	ignore_mark = &fanotify_mark_types[tc->ignore_mark_type];
	ignore_mark_type = ignore_mark->flag & FAN_MARK_TYPES;

	/* Open fd for syncfs before creating groups to avoid the FAN_OPEN event */
	fd_syncfs = SAFE_OPEN(MOUNT_PATH, O_RDONLY);

	for (p = 0; p < num_classes; p++) {
		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			fd_notify[p][i] = SAFE_FANOTIFY_INIT(fanotify_class[p] |
							     FAN_NONBLOCK, O_RDONLY);

			/*
			 * Add mark for each group.
			 *
			 * FAN_EVENT_ON_CHILD has no effect on filesystem/mount
			 * or inode mark on non-directory.
			 */
			foreach_path(tc, path, mark_path)
				SAFE_FANOTIFY_MARK(fd_notify[p][i],
					    FAN_MARK_ADD | mark->flag,
					    tc->expected_mask_without_ignore |
					    FAN_EVENT_ON_CHILD | FAN_ONDIR,
					    AT_FDCWD, path);

			/* Do not add ignore mark for first priority groups */
			if (p == 0)
				continue;

			/*
			 * Run tests in two variants:
			 * 1. Legacy FAN_MARK_IGNORED_MASK
			 * 2. FAN_MARK_IGNORE
			 */
			mark_ignored = tst_variant ? FAN_MARK_IGNORE_SURV : FAN_MARK_IGNORED_SURV;
			mask = FAN_OPEN | tc->ignored_flags;
add_mark:
			foreach_path(tc, path, ignore_path)
				SAFE_FANOTIFY_MARK(fd_notify[p][i],
					    FAN_MARK_ADD | ignore_mark->flag | mark_ignored,
					    mask, AT_FDCWD, path);

			/*
			 * FAN_MARK_IGNORE respects FAN_EVENT_ON_CHILD flag, but legacy
			 * FAN_MARK_IGNORED_MASK does not. When using legacy ignore mask,
			 * if ignored mask is on a parent watching children, we need to
			 * also set the event and flag FAN_EVENT_ON_CHILD in mark mask.
			 * This is needed to indicate that parent ignored mask
			 * should be applied to events on children.
			 */
			if (ignored_onchild && mark_ignored & FAN_MARK_IGNORED_MASK) {
				mark_ignored = 0;
				goto add_mark;
			}

			/*
			 * When using FAN_MARK_IGNORE, verify that the FAN_EVENT_ON_CHILD
			 * flag in mark mask does not affect the ignore mask.
			 *
			 * If parent does not want to ignore FAN_OPEN events on children,
			 * set a mark mask to watch FAN_CLOSE_WRITE events on children
			 * to make sure we do not ignore FAN_OPEN events from children.
			 *
			 * If parent wants to ignore FAN_OPEN events on childern,
			 * set a mark mask to watch FAN_CLOSE events only on parent itself
			 * to make sure we do not get FAN_CLOSE events from children.
			 *
			 * If we had already set the FAN_EVENT_ON_CHILD in the parent
			 * mark mask (mark_type == FANOTIFY_PARENT), then FAN_CLOSE mask
			 * will apply also to childern, so we skip this verification.
			 */
			if (mark_ignored & FAN_MARK_IGNORE &&
			    tc->ignore_mark_type == FANOTIFY_PARENT) {
				if (!ignored_onchild)
					mask = FAN_CLOSE_WRITE | FAN_EVENT_ON_CHILD | FAN_ONDIR;
				else if (tc->mark_type == FANOTIFY_PARENT)
					continue;
				else if (tc->ignored_flags & FAN_ONDIR)
					mask = FAN_CLOSE | ignored_onchild;
				else
					mask = FAN_CLOSE | FAN_ONDIR;
				mark_ignored = 0;
				goto add_mark;
			}
		}
	}

	/*
	 * Verify that first priority groups have no ignore inode marks and that
	 * drop_caches evicted the evictable ignore marks of other groups.
	 */
	if (evictable_ignored)
		drop_caches();

	if (ignore_mark_type == FAN_MARK_INODE) {
		for (p = 0; p < num_classes; p++) {
			for (i = 0; i < GROUPS_PER_PRIO; i++) {
				if (fd_notify[p][i] > 0) {
					int minexp, maxexp;

					if (p == 0) {
						minexp = maxexp = 0;
					} else if (evictable_ignored) {
						minexp = 0;
						/*
						 * Check at least half the
						 * marks get evicted by reclaim
						 */
						maxexp = tc->ignore_path_cnt / 2;
					} else {
						minexp = maxexp = tc->ignore_path_cnt ? : 1;
					}
					show_fanotify_ignore_marks(fd_notify[p][i],
								   minexp, maxexp);
				}
			}
		}
	}

	return 0;
}

static void cleanup_fanotify_groups(void)
{
	unsigned int i, p;

	for (p = 0; p < num_classes; p++) {
		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			if (fd_notify[p][i] > 0)
				SAFE_CLOSE(fd_notify[p][i]);
		}
	}
	if (fd_syncfs > 0)
		SAFE_CLOSE(fd_syncfs);
}

/* Flush out all pending dirty inodes and destructing marks */
static void mount_cycle(void)
{
	if (bind_mount_created)
		SAFE_UMOUNT(MNT2_PATH);
	SAFE_UMOUNT(MOUNT_PATH);
	SAFE_MOUNT(tst_device->dev, MOUNT_PATH, tst_device->fs_type, 0, NULL);
	SAFE_MOUNT(MOUNT_PATH, MNT2_PATH, "none", MS_BIND, NULL);
	bind_mount_created = 1;
}

static int verify_event(int p, int group, struct fanotify_event_metadata *event,
			 unsigned long long expected_mask)
{
	/* Only FAN_REPORT_FID reports the FAN_ONDIR flag in events on dirs */
	if (!(fanotify_class[p] & FAN_REPORT_FID))
		expected_mask &= ~FAN_ONDIR;

	if (event->mask != expected_mask) {
		tst_res(TFAIL, "group %d (%x) got event: mask %llx (expected %llx) "
			"pid=%u fd=%u", group, fanotify_class[p],
			(unsigned long long) event->mask,
			(unsigned long long) expected_mask,
			(unsigned int)event->pid, event->fd);
		return 0;
	} else if (event->pid != child_pid) {
		tst_res(TFAIL, "group %d (%x) got event: mask %llx pid=%u "
			"(expected %u) fd=%u", group, fanotify_class[p],
			(unsigned long long)event->mask, (unsigned int)event->pid,
			(unsigned int)child_pid, event->fd);
		return 0;
	}
	return 1;
}

static int generate_event(struct tcase *tc, unsigned long long expected_mask)
{
	int fd, status;

	child_pid = SAFE_FORK();

	if (child_pid == 0) {
		char path[PATH_MAX];

		foreach_path(tc, path, event_path) {
			if (expected_mask & FAN_OPEN_EXEC) {
				SAFE_EXECL(path, path, NULL);
			} else {
				fd = SAFE_OPEN(path, O_RDONLY);

				if (fd > 0)
					SAFE_CLOSE(fd);
			}
		}

		exit(0);
	}

	SAFE_WAITPID(child_pid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return 1;
	return 0;
}

static struct fanotify_event_metadata *fetch_event(int fd, int *retp)
{
	int ret;
	struct fanotify_event_metadata *event;

	*retp = 0;
	if (event_buf_pos >= event_buf_len) {
		event_buf_pos = 0;
		ret = read(fd, event_buf, EVENT_BUF_LEN);
		if (ret < 0) {
			if (errno == EAGAIN)
				return NULL;
			tst_brk(TBROK | TERRNO, "reading fanotify events failed");
		}
		event_buf_len = ret;
	}
	if (event_buf_len - event_buf_pos < (int)FAN_EVENT_METADATA_LEN) {
		tst_brk(TBROK,
			"too short event when reading fanotify events (%d < %d)",
			event_buf_len - event_buf_pos,
			(int)FAN_EVENT_METADATA_LEN);
	}
	event = (struct fanotify_event_metadata *)(event_buf + event_buf_pos);
	event_buf_pos += event->event_len;
	return event;
}

static void test_fanotify(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct fanotify_mark_type *mark, *ignore_mark;
	int ret;
	unsigned int p, i;
	struct fanotify_event_metadata *event;
	int event_count;

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	if (exec_events_unsupported && tc->expected_mask_with_ignore & FAN_OPEN_EXEC) {
		tst_res(TCONF, "FAN_OPEN_EXEC not supported in kernel?");
		return;
	}

	if (filesystem_mark_unsupported && tc->mark_type == FANOTIFY_FILESYSTEM) {
		tst_res(TCONF, "FAN_MARK_FILESYSTEM not supported in kernel?");
		return;
	}

	if (evictable_mark_unsupported && tc->ignore_mark_type == FANOTIFY_EVICTABLE) {
		tst_res(TCONF, "FAN_MARK_EVICTABLE not supported in kernel?");
		return;
	}

	if (ignore_mark_unsupported && tst_variant) {
		tst_res(TCONF, "FAN_MARK_IGNORE not supported in kernel?");
		return;
	}

	if (tc->ignored_flags & FAN_EVENT_ON_CHILD && tst_kvercmp(5, 9, 0) < 0) {
		tst_res(TCONF, "ignored mask in combination with flag FAN_EVENT_ON_CHILD"
				" has undefined behavior on kernel < 5.9");
		return;
	}

	if (tc->ignored_flags && tc->ignore_mark_type == FANOTIFY_PARENT &&
			!tst_variant && tc->mark_type == FANOTIFY_SUBDIR) {
		tst_res(TCONF, "flags FAN_EVENT_ON_CHILD and FAN_ONDIR do not take effect"
				" with legacy FAN_MARK_IGNORED_MASK");
		return;
	}

	if (create_fanotify_groups(n) != 0)
		goto cleanup;

	mark = &fanotify_mark_types[tc->mark_type];
	ignore_mark = &fanotify_mark_types[tc->ignore_mark_type];

	/* Generate event in child process */
	if (!generate_event(tc, tc->expected_mask_with_ignore))
		tst_brk(TBROK, "Child process terminated incorrectly");

	/* First verify all groups without matching ignore mask got the event */
	for (p = 0; p < num_classes; p++) {
		if (p > 0 && !tc->expected_mask_with_ignore)
			break;

		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			event_count = 0;
			event_buf_pos = event_buf_len = 0;
			while ((event = fetch_event(fd_notify[p][i], &ret))) {
				event_count++;
				if (!verify_event(p, i, event, p == 0 ?
						tc->expected_mask_without_ignore :
						tc->expected_mask_with_ignore))
					break;
				if (event->fd != FAN_NOFD)
					SAFE_CLOSE(event->fd);
			}
			if (ret < 0)
				continue;
			if (event_count != (tc->event_path_cnt ? : 1)) {
				tst_res(TFAIL, "group %d (%x) with %s "
					"got unexpected number of events (%d != %d)",
					i, fanotify_class[p], mark->name,
					event_count, tc->event_path_cnt);
			} else {
				tst_res(TPASS, "group %d (%x) got %d events: mask %llx pid=%u",
					i, fanotify_class[p], event_count,
					(unsigned long long)(p == 0 ?
					tc->expected_mask_without_ignore :
					tc->expected_mask_with_ignore),
					(unsigned int)child_pid);
			}
		}
	}
	/* Then verify all groups with matching ignore mask did got the event */
	for (p = 1; p < num_classes && !tc->expected_mask_with_ignore; p++) {
		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			event_count = 0;
			event_buf_pos = event_buf_len = 0;
			while ((event = fetch_event(fd_notify[p][i], &ret))) {
				event_count++;
				if (event->fd != FAN_NOFD)
					SAFE_CLOSE(event->fd);
			}
			if (ret < 0)
				continue;
			if (event_count > tc->event_path_cnt / 2)
				tst_res(TFAIL, "group %d (%x) with %s and "
					"%s ignore mask got unexpectedly many events (%d > %d)",
					i, fanotify_class[p], mark->name,
					ignore_mark->name, event_count,
					tc->event_path_cnt / 2);
		}
	}
cleanup:
	cleanup_fanotify_groups();
	mount_cycle();
}

static void setup(void)
{
	int i;

	exec_events_unsupported = fanotify_flags_supported_on_fs(FAN_CLASS_CONTENT,
					0, FAN_OPEN_EXEC, MOUNT_PATH);
	filesystem_mark_unsupported = fanotify_mark_supported_on_fs(FAN_MARK_FILESYSTEM,
								    MOUNT_PATH);
	evictable_mark_unsupported = fanotify_mark_supported_on_fs(FAN_MARK_EVICTABLE,
								   MOUNT_PATH);
	ignore_mark_unsupported = fanotify_mark_supported_on_fs(FAN_MARK_IGNORE_SURV,
								MOUNT_PATH);
	fan_report_dfid_unsupported = fanotify_flags_supported_on_fs(FAN_REPORT_DFID_NAME,
								     FAN_MARK_MOUNT,
								     FAN_OPEN, MOUNT_PATH);
	if (fan_report_dfid_unsupported) {
		FANOTIFY_INIT_FLAGS_ERR_MSG(FAN_REPORT_DFID_NAME, fan_report_dfid_unsupported);
		/* Limit tests to legacy priority classes */
		num_classes = NUM_PRIORITIES;
	}

	SAFE_MKDIR(DIR_PATH, 0755);
	SAFE_MKDIR(SUBDIR_PATH, 0755);
	SAFE_FILE_PRINTF(FILE_PATH, "1");
	for (i = 0; i < (int)ARRAY_SIZE(tcases); i++) {
		if (tcases[i].mark_path_cnt > max_file_multi)
			max_file_multi = tcases[i].mark_path_cnt;
		if (tcases[i].ignore_path_cnt > max_file_multi)
			max_file_multi = tcases[i].ignore_path_cnt;
		if (tcases[i].event_path_cnt > max_file_multi)
			max_file_multi = tcases[i].event_path_cnt;
	}
	for (i = 0; i < max_file_multi; i++) {
		char path[PATH_MAX];

		sprintf(path, FILE_PATH_MULTI, i);
		SAFE_FILE_PRINTF(path, "1");
		sprintf(path, DIR_PATH_MULTI, i);
		SAFE_MKDIR(path, 0755);
		sprintf(path, FILE_PATH_MULTIDIR, i);
		SAFE_FILE_PRINTF(path, "1");
	}

	SAFE_CP(TEST_APP, FILE_EXEC_PATH);
	SAFE_CP(TEST_APP, FILE2_EXEC_PATH);

	/* Create another bind mount at another path for generating events */
	SAFE_MKDIR(MNT2_PATH, 0755);
	mount_cycle();

	SAFE_FILE_SCANF(CACHE_PRESSURE_FILE, "%d", &old_cache_pressure);
	/* Set high priority for evicting inodes */
	SAFE_FILE_PRINTF(CACHE_PRESSURE_FILE, "500");
}

static void cleanup(void)
{
	int i;

	cleanup_fanotify_groups();

	if (bind_mount_created)
		SAFE_UMOUNT(MNT2_PATH);

	SAFE_FILE_PRINTF(CACHE_PRESSURE_FILE, "%d", old_cache_pressure);

	for (i = 0; i < max_file_multi; i++) {
		char path[PATH_MAX];

		sprintf(path, FILE_PATH_MULTIDIR, i);
		SAFE_UNLINK(path);
		sprintf(path, DIR_PATH_MULTI, i);
		SAFE_RMDIR(path);
		sprintf(path, FILE_PATH_MULTI, i);
		SAFE_UNLINK(path);
	}
	SAFE_UNLINK(FILE_PATH);
	SAFE_RMDIR(SUBDIR_PATH);
	SAFE_RMDIR(DIR_PATH);
	SAFE_RMDIR(MNT2_PATH);
}

static struct tst_test test = {
	.test = test_fanotify,
	.tcnt = ARRAY_SIZE(tcases),
	.test_variants = 2,
	.setup = setup,
	.cleanup = cleanup,
	.mount_device = 1,
	.mntpoint = MOUNT_PATH,
	.needs_root = 1,
	.forks_child = 1,
	.resource_files = (const char *const []) {
		TEST_APP,
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "9bdda4e9cf2d"},
		{"linux-git", "2f02fd3fa13e"},
		{}
	}
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
