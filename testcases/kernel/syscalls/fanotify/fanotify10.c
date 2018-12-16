// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014 SUSE.  All Rights Reserved.
 * Copyright (c) 2018 CTERA Networks.  All Rights Reserved.
 *
 * Started by Jan Kara <jack@suse.cz>
 * Forked from fanotify06.c by Amir Goldstein <amir73il@gmail.com>
 *
 * DESCRIPTION
 *     Check that fanotify properly merges ignore mask of a mount mark
 *     with a mask of an inode mark on the same group.  Unlike the
 *     prototype test fanotify06, do not use FAN_MODIFY event for the
 *     test mask, because it hides the bug.
 *
 * This is a regression test for commit:
 *
 *     9bdda4e9cf2d fsnotify: fix ignore mask logic in fsnotify()
 */
#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include "tst_test.h"
#include "fanotify.h"

#if defined(HAVE_SYS_FANOTIFY_H)
#include <sys/fanotify.h>

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct fanotify_event_metadata))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * EVENT_SIZE)

static unsigned int fanotify_prio[] = {
	FAN_CLASS_PRE_CONTENT,
	FAN_CLASS_CONTENT,
	FAN_CLASS_NOTIF
};
#define FANOTIFY_PRIORITIES ARRAY_SIZE(fanotify_prio)

#define GROUPS_PER_PRIO 3

static int fd_notify[FANOTIFY_PRIORITIES][GROUPS_PER_PRIO];

static char event_buf[EVENT_BUF_LEN];

#define MOUNT_PATH "fs_mnt"
#define MNT2_PATH "mntpoint"
#define FILE_NAME "testfile"
#define FILE2_NAME "testfile2"
#define FILE_PATH MOUNT_PATH"/"FILE_NAME
#define FILE2_PATH MOUNT_PATH"/"FILE2_NAME
#define FILE_MNT2 MNT2_PATH"/"FILE_NAME
#define FILE2_MNT2 MNT2_PATH"/"FILE2_NAME

static int bind_mount_created;

enum {
	FANOTIFY_INODE,
	FANOTIFY_MOUNT,
	FANOTIFY_FILESYSTEM,
};

static struct fanotify_mark_type fanotify_mark_types[] = {
	INIT_FANOTIFY_MARK_TYPE(INODE),
	INIT_FANOTIFY_MARK_TYPE(MOUNT),
	INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
};

static struct tcase {
	const char *tname;
	const char *mark_path;
	int mark_type;
	const char *ignore_path;
	int ignore_mark_type;
	const char *event_path;
	int expect_event;
} tcases[] = {
	{
		"ignore mount events created on a specific file",
		MOUNT_PATH, FANOTIFY_MOUNT,
		FILE_MNT2, FANOTIFY_INODE,
		FILE_PATH, 0
	},
	{
		"don't ignore mount events created on another file",
		MOUNT_PATH, FANOTIFY_MOUNT,
		FILE_PATH, FANOTIFY_INODE,
		FILE2_PATH, 1
	},
	{
		"ignore inode events created on a specific mount point",
		FILE_PATH, FANOTIFY_INODE,
		MNT2_PATH, FANOTIFY_MOUNT,
		FILE_MNT2, 0
	},
	{
		"don't ignore inode events created on another mount point",
		FILE_MNT2, FANOTIFY_INODE,
		MNT2_PATH, FANOTIFY_MOUNT,
		FILE_PATH, 1
	},
	{
		"ignore fs events created on a specific file",
		MOUNT_PATH, FANOTIFY_FILESYSTEM,
		FILE_PATH, FANOTIFY_INODE,
		FILE_PATH, 0
	},
	{
		"don't ignore mount events created on another file",
		MOUNT_PATH, FANOTIFY_FILESYSTEM,
		FILE_PATH, FANOTIFY_INODE,
		FILE2_PATH, 1
	},
	{
		"ignore fs events created on a specific mount point",
		MOUNT_PATH, FANOTIFY_FILESYSTEM,
		MNT2_PATH, FANOTIFY_MOUNT,
		FILE_MNT2, 0
	},
	{
		"don't ignore fs events created on another mount point",
		MOUNT_PATH, FANOTIFY_FILESYSTEM,
		MNT2_PATH, FANOTIFY_MOUNT,
		FILE_PATH, 1
	},
};

static int create_fanotify_groups(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct fanotify_mark_type *mark, *ignore_mark;
	unsigned int p, i;
	int ret;

	mark = &fanotify_mark_types[tc->mark_type];
	ignore_mark = &fanotify_mark_types[tc->ignore_mark_type];

	for (p = 0; p < FANOTIFY_PRIORITIES; p++) {
		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			fd_notify[p][i] = SAFE_FANOTIFY_INIT(fanotify_prio[p] |
							     FAN_NONBLOCK,
							     O_RDONLY);

			/* Add mark for each group */
			ret = fanotify_mark(fd_notify[p][i],
					    FAN_MARK_ADD | mark->flag,
					    FAN_OPEN, AT_FDCWD, tc->mark_path);
			if (ret < 0) {
				if (errno == EINVAL &&
				    tc->mark_type == FANOTIFY_FILESYSTEM) {
					tst_res(TCONF,
						"FAN_MARK_FILESYSTEM not "
						"supported in kernel?");
					return -1;
				}
				tst_brk(TBROK | TERRNO,
					"fanotify_mark(%d, FAN_MARK_ADD | %s,"
					"FAN_OPEN, AT_FDCWD, %s) failed",
					fd_notify[p][i], mark->name,
					tc->mark_path);
			}
			/* Add ignore mark for groups with higher priority */
			if (p == 0)
				continue;
			ret = fanotify_mark(fd_notify[p][i],
					    FAN_MARK_ADD | ignore_mark->flag |
					    FAN_MARK_IGNORED_MASK |
					    FAN_MARK_IGNORED_SURV_MODIFY,
					    FAN_OPEN, AT_FDCWD,
					    tc->ignore_path);
			if (ret < 0) {
				tst_brk(TBROK | TERRNO,
					"fanotify_mark(%d, FAN_MARK_ADD | %s | "
					"FAN_MARK_IGNORED_MASK | "
					"FAN_MARK_IGNORED_SURV_MODIFY, "
					"FAN_OPEN, AT_FDCWD, %s) failed",
					fd_notify[p][i], ignore_mark->name,
					tc->ignore_path);
			}
		}
	}
	return 0;
}

static void cleanup_fanotify_groups(void)
{
	unsigned int i, p;

	for (p = 0; p < FANOTIFY_PRIORITIES; p++) {
		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			if (fd_notify[p][i] > 0)
				SAFE_CLOSE(fd_notify[p][i]);
		}
	}
}

static void verify_event(int group, struct fanotify_event_metadata *event)
{
	if (event->mask != FAN_OPEN) {
		tst_res(TFAIL, "group %d got event: mask %llx (expected %llx) "
			"pid=%u fd=%u", group, (unsigned long long)event->mask,
			(unsigned long long)FAN_OPEN,
			(unsigned)event->pid, event->fd);
	} else if (event->pid != getpid()) {
		tst_res(TFAIL, "group %d got event: mask %llx pid=%u "
			"(expected %u) fd=%u", group,
			(unsigned long long)event->mask, (unsigned)event->pid,
			(unsigned)getpid(), event->fd);
	} else {
		tst_res(TPASS, "group %d got event: mask %llx pid=%u fd=%u",
			group, (unsigned long long)event->mask,
			(unsigned)event->pid, event->fd);
	}
}

static void test_fanotify(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct fanotify_mark_type *mark, *ignore_mark;
	int ret, fd;
	unsigned int p, i;
	struct fanotify_event_metadata *event;

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	if (create_fanotify_groups(n) != 0)
		goto cleanup;

	mark = &fanotify_mark_types[tc->mark_type];
	ignore_mark = &fanotify_mark_types[tc->ignore_mark_type];

	/*
	 * generate sequence of events
	 */
	fd = SAFE_OPEN(tc->event_path, O_RDONLY);
	SAFE_CLOSE(fd);

	/* First verify all groups without matching ignore mask got the event */
	for (p = 0; p < FANOTIFY_PRIORITIES; p++) {
		if (p > 0 && !tc->expect_event)
			break;

		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			ret = read(fd_notify[p][i], event_buf, EVENT_BUF_LEN);
			if (ret < 0) {
				if (errno == EAGAIN) {
					tst_res(TFAIL, "group %d (prio %d) "
						"with %s did not get event",
						i, p, mark->name);
				}
				tst_brk(TBROK | TERRNO,
					"reading fanotify events failed");
			}
			if (ret < (int)FAN_EVENT_METADATA_LEN) {
				tst_brk(TBROK,
					"short read when reading fanotify "
					"events (%d < %d)", ret,
					(int)EVENT_BUF_LEN);
			}
			event = (struct fanotify_event_metadata *)event_buf;
			if (ret > (int)event->event_len) {
				tst_res(TFAIL, "group %d (prio %d) with %s "
					"got more than one event (%d > %d)",
					i, p, mark->name, ret,
					event->event_len);
			} else {
				verify_event(i, event);
			}
			if (event->fd != FAN_NOFD)
				SAFE_CLOSE(event->fd);
		}
	}
	/* Then verify all groups with matching ignore mask did got the event */
	for (p = 1; p < FANOTIFY_PRIORITIES && !tc->expect_event; p++) {
		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			ret = read(fd_notify[p][i], event_buf, EVENT_BUF_LEN);
			if (ret == 0) {
				tst_brk(TBROK,
					"zero length read from fanotify fd");
			}
			if (ret > 0) {
				tst_res(TFAIL, "group %d (prio %d) with %s and "
					"%s ignore mask got event",
					i, p, mark->name, ignore_mark->name);
				if (event->fd != FAN_NOFD)
					SAFE_CLOSE(event->fd);
			} else if (errno == EAGAIN) {
				tst_res(TPASS, "group %d (prio %d) with %s and "
					"%s ignore mask got no event",
					i, p, mark->name, ignore_mark->name);
			} else {
				tst_brk(TBROK | TERRNO,
					"reading fanotify events failed");
			}
		}
	}
cleanup:
	cleanup_fanotify_groups();
}

static void setup(void)
{
	/* Create another bind mount at another path for generating events */
	SAFE_MKDIR(MNT2_PATH, 0755);
	SAFE_MOUNT(MOUNT_PATH, MNT2_PATH, "none", MS_BIND, NULL);
	bind_mount_created = 1;

	SAFE_FILE_PRINTF(FILE_PATH, "1");
	SAFE_FILE_PRINTF(FILE2_PATH, "1");
}

static void cleanup(void)
{
	cleanup_fanotify_groups();

	if (bind_mount_created && tst_umount(MNT2_PATH) < 0)
		tst_brk(TBROK | TERRNO, "bind umount failed");
}

static struct tst_test test = {
	.test = test_fanotify,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.mount_device = 1,
	.mntpoint = MOUNT_PATH,
	.needs_tmpdir = 1,
	.needs_root = 1
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
