// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 CTERA Networks.  All Rights Reserved.
 *
 * Started by Amir Goldstein <amir73il@gmail.com>
 *
 * DESCRIPTION
 *     Check that fanotify handles events on children correctly when
 *     both inode and mountpoint marks exist.
 *
 * This is a regression test for commit 54a307ba8d3c:
 *
 *      fanotify: fix logic of events on child
 *
 * Test case #2 is a regression test for commit b469e7e47c8a:
 *
 *      fanotify: fix handling of events on child sub-directory
 *
 * Test case #3 is a regression test for commit 55bf882c7f13:
 *
 *      fanotify: fix merging marks masks with FAN_ONDIR
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
#include <stdint.h>
#include "tst_test.h"
#include "fanotify.h"

#if defined(HAVE_SYS_FANOTIFY_H)
#include <sys/fanotify.h>

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct fanotify_event_metadata))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * EVENT_SIZE)

#define NUM_GROUPS 3

#define BUF_SIZE 256
static char fname[BUF_SIZE];
static char symlnk[BUF_SIZE];
static char fdpath[BUF_SIZE];
static int fd_notify[NUM_GROUPS];

static char event_buf[EVENT_BUF_LEN];

#define MOUNT_NAME "mntpoint"
#define DIR_NAME "testdir"
static int mount_created;

static struct tcase {
	const char *tname;
	unsigned int ondir;
	const char *testdir;
	int nevents;
} tcases[] = {
	{
		"Events on children with both inode and mount marks",
		0,
		DIR_NAME,
		1,
	},
	{
		"Events on children and subdirs with both inode and mount marks",
		FAN_ONDIR,
		DIR_NAME,
		2,
	},
	{
		"Events on files and dirs with both inode and mount marks",
		FAN_ONDIR,
		".",
		2,
	},
};

static void create_fanotify_groups(unsigned int ondir)
{
	unsigned int i, onchild;
	int ret;

	for (i = 0; i < NUM_GROUPS; i++) {
		fd_notify[i] = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF |
						  FAN_NONBLOCK,
						  O_RDONLY);

		/* Add mount mark for each group without MODIFY event */
		onchild = (i == 0) ? FAN_EVENT_ON_CHILD | ondir : 0;
		ret = fanotify_mark(fd_notify[i],
				    FAN_MARK_ADD | FAN_MARK_MOUNT,
				    FAN_CLOSE_NOWRITE | onchild,
				    AT_FDCWD, ".");
		if (ret < 0) {
			tst_brk(TBROK | TERRNO,
				"fanotify_mark(%d, FAN_MARK_ADD | "
				"FAN_MARK_MOUNT, FAN_MODIFY%s, AT_FDCWD,"
				" '.') failed", fd_notify[i],
				ondir ? " | FAN_ONDIR" : "");
		}
		/*
		 * Add inode mark on parent for each group with MODIFY
		 * event, but only one group requests events on child.
		 * The one mark with FAN_EVENT_ON_CHILD is needed for
		 * setting the DCACHE_FSNOTIFY_PARENT_WATCHED dentry
		 * flag.
		 */
		ret = fanotify_mark(fd_notify[i], FAN_MARK_ADD,
				    FAN_MODIFY | ondir | onchild,
				    AT_FDCWD, ".");
		if (ret < 0) {
			tst_brk(TBROK | TERRNO,
				"fanotify_mark(%d, FAN_MARK_ADD, "
				"FAN_MODIFY%s%s, AT_FDCWD, '.') failed",
				fd_notify[i],
				ondir ? " | FAN_ONDIR" : "",
				onchild ? " | FAN_EVENT_ON_CHILD" : "");
		}
	}
}

static void cleanup_fanotify_groups(void)
{
	unsigned int i;

	for (i = 0; i < NUM_GROUPS; i++) {
		if (fd_notify[i] > 0)
			SAFE_CLOSE(fd_notify[i]);
	}
}

static void event_res(int ttype, int group,
		      struct fanotify_event_metadata *event)
{
	int len;
	sprintf(symlnk, "/proc/self/fd/%d", event->fd);
	len = readlink(symlnk, fdpath, sizeof(fdpath));
	if (len < 0)
		len = 0;
	fdpath[len] = 0;
	tst_res(ttype, "group %d got event: mask %llx pid=%u fd=%d path=%s",
		group, (unsigned long long)event->mask,
		(unsigned)event->pid, event->fd, fdpath);
}

static void verify_event(int group, struct fanotify_event_metadata *event,
			 uint32_t expect)
{
	if (event->mask != expect) {
		tst_res(TFAIL, "group %d got event: mask %llx (expected %llx) "
			"pid=%u fd=%d", group, (unsigned long long)event->mask,
			(unsigned long long)expect,
			(unsigned)event->pid, event->fd);
	} else if (event->pid != getpid()) {
		tst_res(TFAIL, "group %d got event: mask %llx pid=%u "
			"(expected %u) fd=%d", group,
			(unsigned long long)event->mask, (unsigned)event->pid,
			(unsigned)getpid(), event->fd);
	} else {
		event_res(TPASS, group, event);
	}
}

static void test_fanotify(unsigned int n)
{
	int ret, dirfd;
	unsigned int i;
	struct fanotify_event_metadata *event, *ev;
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	create_fanotify_groups(tc->ondir);

	/*
	 * generate MODIFY event and no FAN_CLOSE_NOWRITE event.
	 */
	SAFE_FILE_PRINTF(fname, "1");
	/*
	 * generate FAN_CLOSE_NOWRITE event on a testdir (subdir or ".")
	 */
	dirfd = SAFE_OPEN(tc->testdir, O_RDONLY);
	if (dirfd >= 0)
		SAFE_CLOSE(dirfd);

	/*
	 * First verify the first group got the file MODIFY event and got just
	 * one FAN_CLOSE_NOWRITE event.
	 */
	ret = read(fd_notify[0], event_buf, EVENT_BUF_LEN);
	if (ret < 0) {
		if (errno == EAGAIN) {
			tst_res(TFAIL, "first group did not get event");
		} else {
			tst_brk(TBROK | TERRNO,
				"reading fanotify events failed");
		}
	}
	if (ret < tc->nevents * (int)FAN_EVENT_METADATA_LEN) {
		tst_brk(TBROK,
			"short read when reading fanotify events (%d < %d)",
			ret, tc->nevents * (int)FAN_EVENT_METADATA_LEN);
	}
	event = (struct fanotify_event_metadata *)event_buf;
	verify_event(0, event, FAN_MODIFY);
	if (tc->ondir)
		verify_event(0, event + 1, FAN_CLOSE_NOWRITE);
	if (ret > tc->nevents * (int)FAN_EVENT_METADATA_LEN) {
		tst_res(TFAIL,
			"first group got more than %d events (%d > %d)",
			tc->nevents, ret,
			tc->nevents * (int)FAN_EVENT_METADATA_LEN);
	}
	/* Close all file descriptors of read events */
	for (ev = event; ret >= (int)FAN_EVENT_METADATA_LEN; ev++) {
		if (ev->fd != FAN_NOFD)
			SAFE_CLOSE(ev->fd);
		ret -= (int)FAN_EVENT_METADATA_LEN;
	}

	/*
	 * Then verify the rest of the groups did not get the MODIFY event and
	 * did not get the FAN_CLOSE_NOWRITE event on testdir.
	 */
	for (i = 1; i < NUM_GROUPS; i++) {
		ret = read(fd_notify[i], event_buf, FAN_EVENT_METADATA_LEN);
		if (ret > 0) {
			event_res(TFAIL, i, event);
			if (event->fd != FAN_NOFD)
				SAFE_CLOSE(event->fd);
			continue;
		}

		if (ret == 0) {
			tst_brk(TBROK, "zero length read from fanotify fd");
		}

		if (errno != EAGAIN) {
			tst_brk(TBROK | TERRNO,
				"reading fanotify events failed");
		}

		tst_res(TPASS, "group %d got no event", i);
	}
	cleanup_fanotify_groups();
}

static void setup(void)
{
	SAFE_MKDIR(MOUNT_NAME, 0755);
	SAFE_MOUNT(MOUNT_NAME, MOUNT_NAME, "none", MS_BIND, NULL);
	mount_created = 1;
	SAFE_CHDIR(MOUNT_NAME);
	SAFE_MKDIR(DIR_NAME, 0755);

	sprintf(fname, "tfile_%d", getpid());
	SAFE_FILE_PRINTF(fname, "1");
}

static void cleanup(void)
{
	cleanup_fanotify_groups();

	SAFE_CHDIR("../");

	if (mount_created && tst_umount(MOUNT_NAME) < 0)
		tst_brk(TBROK | TERRNO, "umount failed");
}

static struct tst_test test = {
	.test = test_fanotify,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "54a307ba8d3c"},
		{"linux-git", "b469e7e47c8a"},
		{"linux-git", "55bf882c7f13"},
		{}
	}
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
