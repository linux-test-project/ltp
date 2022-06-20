// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 CTERA Networks.  All Rights Reserved.
 *
 * Started by Amir Goldstein <amir73il@gmail.com>
 */

/*\
 * [Description]
 * Check that fanotify handles events on children correctly when both parent and
 * subdir or mountpoint marks exist.
 */

/*
 * This is a regression test for commit:
 *
 *      54a307ba8d3c fanotify: fix logic of events on child
 *
 * Test case #1 is a regression test for commit:
 *
 *      b469e7e47c8a fanotify: fix handling of events on child sub-directory
 *
 * Test case #2 is a regression test for commit:
 *
 *      55bf882c7f13 fanotify: fix merging marks masks with FAN_ONDIR
 *
 * Test case #5 is a regression test for commit:
 *
 *      7372e79c9eb9 fanotify: fix logic of reporting name info with watched parent
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
#include <stdint.h>
#include "tst_test.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof(struct fanotify_event_metadata))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * EVENT_SIZE)

#define NUM_GROUPS 3

#define BUF_SIZE 256
static char fname[BUF_SIZE];
static char symlnk[BUF_SIZE];
static char fdpath[BUF_SIZE];
static int fd_notify[NUM_GROUPS];

static char event_buf[EVENT_BUF_LEN];

#define MOUNT_PATH "fs_mnt"
#define MOUNT_NAME "mntpoint"
#define DIR_NAME "testdir"
#define FILE2_NAME "testfile"
static int mount_created;

static int fan_report_dfid_unsupported;

static struct tcase {
	const char *tname;
	struct fanotify_mark_type mark;
	unsigned int ondir;
	unsigned int report_name;
	const char *close_nowrite;
	int nevents;
	unsigned int nonfirst_event;
} tcases[] = {
	{
		"Events on non-dir child with both parent and mount marks",
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		0,
		0,
		DIR_NAME,
		1, 0,
	},
	{
		"Events on non-dir child and subdir with both parent and mount marks",
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_ONDIR,
		0,
		DIR_NAME,
		2, 0,
	},
	{
		"Events on non-dir child and parent with both parent and mount marks",
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_ONDIR,
		0,
		".",
		2, 0
	},
	{
		"Events on non-dir child and subdir with both parent and subdir marks",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_ONDIR,
		0,
		DIR_NAME,
		2, 0,
	},
	{
		"Events on non-dir children with both parent and mount marks",
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		0,
		0,
		FILE2_NAME,
		2, FAN_CLOSE_NOWRITE,
	},
	{
		"Events on non-dir child with both parent and mount marks and filename info",
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		0,
		FAN_REPORT_DFID_NAME,
		FILE2_NAME,
		2, FAN_CLOSE_NOWRITE,
	},
};

static void create_fanotify_groups(struct tcase *tc)
{
	struct fanotify_mark_type *mark = &tc->mark;
	int i;

	for (i = 0; i < NUM_GROUPS; i++) {
		/*
		 * The first group may request events with filename info and
		 * events on subdirs and always request events on children.
		 */
		unsigned int report_name = tc->report_name;
		unsigned int mask_flags = tc->ondir | FAN_EVENT_ON_CHILD;
		unsigned int parent_mask;

		/*
		 * The non-first groups do not request events on children and
		 * subdirs.
		 */
		if (i > 0) {
			report_name = 0;
			mask_flags = 0;
		}

		fd_notify[i] = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF | report_name |
						  FAN_NONBLOCK, O_RDONLY);

		/*
		 * Add subdir or mount mark for each group with CLOSE event,
		 * but only the first group requests events on dir.
		 */
		SAFE_FANOTIFY_MARK(fd_notify[i],
				    FAN_MARK_ADD | mark->flag,
				    FAN_CLOSE_NOWRITE | mask_flags,
				    AT_FDCWD, tc->close_nowrite);

		/*
		 * Add inode mark on parent for each group with MODIFY event,
		 * but only the first group requests events on child.
		 * The one mark with FAN_EVENT_ON_CHILD is needed for
		 * setting the DCACHE_FSNOTIFY_PARENT_WATCHED dentry flag.
		 */
		parent_mask = FAN_MODIFY | tc->ondir | mask_flags;
		SAFE_FANOTIFY_MARK(fd_notify[i], FAN_MARK_ADD,
				    parent_mask,
				    AT_FDCWD, ".");
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
		      struct fanotify_event_metadata *event,
		      const char *filename)
{
	if (event->fd != FAN_NOFD) {
		int len = 0;

		sprintf(symlnk, "/proc/self/fd/%d", event->fd);
		len = readlink(symlnk, fdpath, sizeof(fdpath));
		if (len < 0)
			len = 0;
		fdpath[len] = 0;
		filename = fdpath;
	}

	tst_res(ttype, "group %d got event: mask %llx pid=%u fd=%d filename=%s",
		group, (unsigned long long)event->mask,
		(unsigned int)event->pid, event->fd, filename);
}

static const char *event_filename(struct fanotify_event_metadata *event)
{
	struct fanotify_event_info_fid *event_fid;
	struct file_handle *file_handle;
	const char *filename, *end;

	if (event->event_len <= FAN_EVENT_METADATA_LEN)
		return "";

	event_fid = (struct fanotify_event_info_fid *)(event + 1);
	file_handle = (struct file_handle *)event_fid->handle;
	filename = (char *)file_handle->f_handle + file_handle->handle_bytes;
	end = (char *)event_fid + event_fid->hdr.len;

	/* End of event_fid could have name, zero padding, both or none */
	return (filename == end) ? "" : filename;
}

static void verify_event(int group, struct fanotify_event_metadata *event,
			 uint32_t expect, const char *expect_filename)
{
	const char *filename = event_filename(event);

	if (event->mask != expect) {
		tst_res(TFAIL, "group %d got event: mask %llx (expected %llx) "
			"pid=%u fd=%d filename=%s", group, (unsigned long long)event->mask,
			(unsigned long long)expect,
			(unsigned int)event->pid, event->fd, filename);
	} else if (event->pid != getpid()) {
		tst_res(TFAIL, "group %d got event: mask %llx pid=%u "
			"(expected %u) fd=%d filename=%s", group,
			(unsigned long long)event->mask, (unsigned int)event->pid,
			(unsigned int)getpid(), event->fd, filename);
	} else if (strcmp(filename, expect_filename)) {
		tst_res(TFAIL, "group %d got event: mask %llx pid=%u "
			"fd=%d filename='%s' (expected '%s')", group,
			(unsigned long long)event->mask, (unsigned int)event->pid,
			event->fd, filename, expect_filename);
	} else {
		event_res(TPASS, group, event, filename);
	}
	if (event->fd != FAN_NOFD)
		SAFE_CLOSE(event->fd);
}

static void close_event_fds(struct fanotify_event_metadata *event, int buflen)
{
	/* Close all file descriptors of read events */
	for (; FAN_EVENT_OK(event, buflen); FAN_EVENT_NEXT(event, buflen)) {
		if (event->fd != FAN_NOFD)
			SAFE_CLOSE(event->fd);
	}
}

static void test_fanotify(unsigned int n)
{
	int ret, dirfd;
	unsigned int i;
	struct fanotify_event_metadata *event;
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	if (fan_report_dfid_unsupported && tc->report_name) {
		FANOTIFY_INIT_FLAGS_ERR_MSG(FAN_REPORT_DFID_NAME, fan_report_dfid_unsupported);
		return;
	}

	create_fanotify_groups(tc);

	/*
	 * generate MODIFY event and no FAN_CLOSE_NOWRITE event.
	 */
	SAFE_FILE_PRINTF(fname, "1");
	/*
	 * generate FAN_CLOSE_NOWRITE event on a child, subdir or "."
	 */
	dirfd = SAFE_OPEN(tc->close_nowrite, O_RDONLY);
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
	event = (struct fanotify_event_metadata *)event_buf;
	if (ret < tc->nevents * (int)FAN_EVENT_METADATA_LEN) {
		tst_res(TFAIL,
			"short read when reading fanotify events (%d < %d)",
			ret, tc->nevents * (int)FAN_EVENT_METADATA_LEN);
	}
	if (FAN_EVENT_OK(event, ret)) {
		verify_event(0, event, FAN_MODIFY, tc->report_name ? fname : "");
		event = FAN_EVENT_NEXT(event, ret);
	}
	if (tc->nevents > 1 && FAN_EVENT_OK(event, ret)) {
		verify_event(0, event, FAN_CLOSE_NOWRITE,
			     tc->report_name ? (tc->ondir ? "." : tc->close_nowrite) : "");
		event = FAN_EVENT_NEXT(event, ret);
	}
	if (ret > 0) {
		tst_res(TFAIL,
			"first group got more than %d events (%d bytes)",
			tc->nevents, ret);
	}
	close_event_fds(event, ret);

	/*
	 * Then verify the rest of the groups did not get the MODIFY event and
	 * got the FAN_CLOSE_NOWRITE event only on a non-directory.
	 */
	for (i = 1; i < NUM_GROUPS; i++) {
		ret = read(fd_notify[i], event_buf, EVENT_BUF_LEN);
		if (ret > 0) {
			event = (struct fanotify_event_metadata *)event_buf;
			verify_event(i, event, tc->nonfirst_event, "");
			event = FAN_EVENT_NEXT(event, ret);

			close_event_fds(event, ret);
			continue;
		}

		if (ret == 0) {
			tst_res(TFAIL, "group %d zero length read from fanotify fd", i);
			continue;
		}

		if (errno != EAGAIN) {
			tst_brk(TBROK | TERRNO,
				"reading fanotify events failed");
		}

		if (tc->nonfirst_event)
			tst_res(TFAIL, "group %d expected and got no event", i);
		else
			tst_res(TPASS, "group %d got no event as expected", i);
	}
	cleanup_fanotify_groups();
}

static void setup(void)
{
	fan_report_dfid_unsupported = fanotify_init_flags_supported_on_fs(FAN_REPORT_DFID_NAME,
									  MOUNT_PATH);

	SAFE_MKDIR(MOUNT_NAME, 0755);
	SAFE_MOUNT(MOUNT_PATH, MOUNT_NAME, "none", MS_BIND, NULL);
	mount_created = 1;
	SAFE_CHDIR(MOUNT_NAME);
	SAFE_MKDIR(DIR_NAME, 0755);

	sprintf(fname, "tfile_%d", getpid());
	SAFE_FILE_PRINTF(fname, "1");
	SAFE_FILE_PRINTF(FILE2_NAME, "1");
}

static void cleanup(void)
{
	cleanup_fanotify_groups();

	SAFE_CHDIR("../");

	if (mount_created)
		SAFE_UMOUNT(MOUNT_NAME);
}

static struct tst_test test = {
	.test = test_fanotify,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.mount_device = 1,
	.mntpoint = MOUNT_PATH,
	.needs_root = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "54a307ba8d3c"},
		{"linux-git", "b469e7e47c8a"},
		{"linux-git", "55bf882c7f13"},
		{"linux-git", "7372e79c9eb9"},
		{}
	}
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
