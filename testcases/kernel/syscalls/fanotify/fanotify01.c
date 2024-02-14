// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 SUSE.  All Rights Reserved.
 *
 * Started by Jan Kara <jack@suse.cz>
 */

/*\
 * [Description]
 * Check that fanotify work for a file.
 */

#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include "tst_test.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof(struct fanotify_event_metadata))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * EVENT_SIZE)

#define BUF_SIZE 256
#define TST_TOTAL 12

#define MOUNT_PATH "fs_mnt"

static struct tcase {
	const char *tname;
	struct fanotify_mark_type mark;
	unsigned int init_flags;
} tcases[] = {
	{
		"inode mark events",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CLASS_NOTIF
	},
	{
		"mount mark events",
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_CLASS_NOTIF
	},
	{
		"filesystem mark events",
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_CLASS_NOTIF
	},
	{
		"inode mark events (FAN_REPORT_FID)",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_CLASS_NOTIF|FAN_REPORT_FID
	},
	{
		"mount mark events (FAN_REPORT_FID)",
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_CLASS_NOTIF|FAN_REPORT_FID
	},
	{
		"filesystem mark events (FAN_REPORT_FID)",
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_CLASS_NOTIF|FAN_REPORT_FID
	},
};

static char fname[BUF_SIZE];
static char buf[BUF_SIZE];
static int fd_notify;
static int fan_report_fid_unsupported;
static int mount_mark_fid_unsupported;
static int inode_mark_fid_xdev;
static int filesystem_mark_unsupported;

static unsigned long long event_set[EVENT_MAX];

static char event_buf[EVENT_BUF_LEN];

static void test_fanotify(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct fanotify_mark_type *mark = &tc->mark;
	int fd, ret, len, i = 0, test_num = 0;
	int tst_count = 0;
	int report_fid = (tc->init_flags & FAN_REPORT_FID);

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	if (fan_report_fid_unsupported && report_fid) {
		FANOTIFY_INIT_FLAGS_ERR_MSG(FAN_REPORT_FID, fan_report_fid_unsupported);
		return;
	}

	if (filesystem_mark_unsupported && mark->flag == FAN_MARK_FILESYSTEM) {
		FANOTIFY_MARK_FLAGS_ERR_MSG(mark, filesystem_mark_unsupported);
		return;
	}

	if (mount_mark_fid_unsupported && report_fid && mark->flag != FAN_MARK_INODE) {
		FANOTIFY_MARK_FLAGS_ERR_MSG(mark, mount_mark_fid_unsupported);
		return;
	}

	fd_notify = SAFE_FANOTIFY_INIT(tc->init_flags, O_RDONLY);

	SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD | mark->flag,
			  FAN_ACCESS | FAN_MODIFY | FAN_CLOSE | FAN_OPEN, AT_FDCWD, fname);

	/*
	 * generate sequence of events
	 */
	fd = SAFE_OPEN(fname, O_RDONLY);
	event_set[tst_count] = FAN_OPEN;
	tst_count++;

	SAFE_READ(0, fd, buf, BUF_SIZE);
	event_set[tst_count] = FAN_ACCESS;
	tst_count++;

	SAFE_CLOSE(fd);
	event_set[tst_count] = FAN_CLOSE_NOWRITE;
	tst_count++;

	/*
	 * Get list of events so far. We get events here to avoid
	 * merging of following events with the previous ones.
	 */
	ret = SAFE_READ(0, fd_notify, event_buf, EVENT_BUF_LEN);
	len = ret;

	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);
	event_set[tst_count] = FAN_OPEN;
	tst_count++;

	SAFE_WRITE(SAFE_WRITE_ALL, fd, fname, strlen(fname));
	event_set[tst_count] = FAN_MODIFY;
	tst_count++;

	SAFE_CLOSE(fd);
	event_set[tst_count] = FAN_CLOSE_WRITE;
	tst_count++;

	/*
	 * get another list of events
	 */
	ret = SAFE_READ(0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	len += ret;

	/*
	 * Ignore mask testing
	 */

	/* Ignore access events */
	SAFE_FANOTIFY_MARK(fd_notify,
			  FAN_MARK_ADD | mark->flag | FAN_MARK_IGNORED_MASK,
			  FAN_ACCESS, AT_FDCWD, fname);

	fd = SAFE_OPEN(fname, O_RDWR);
	event_set[tst_count] = FAN_OPEN;
	tst_count++;

	/* This event should be ignored */
	SAFE_READ(0, fd, buf, BUF_SIZE);

	/*
	 * get another list of events to verify the last one got ignored
	 */
	ret = SAFE_READ(0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	len += ret;

	SAFE_LSEEK(fd, 0, SEEK_SET);
	/* Generate modify event to clear ignore mask */
	SAFE_WRITE(SAFE_WRITE_ALL, fd, fname, 1);
	event_set[tst_count] = FAN_MODIFY;
	tst_count++;

	/*
	 * This event shouldn't be ignored because previous modification
	 * should have removed the ignore mask
	 */
	SAFE_READ(0, fd, buf, BUF_SIZE);
	event_set[tst_count] = FAN_ACCESS;
	tst_count++;

	SAFE_CLOSE(fd);
	event_set[tst_count] = FAN_CLOSE_WRITE;
	tst_count++;

	/* Read events to verify previous access was properly generated */
	ret = SAFE_READ(0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	len += ret;

	/*
	 * Now ignore open & close events regardless of file
	 * modifications
	 */
	SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD | mark->flag |
			  FAN_MARK_IGNORED_MASK | FAN_MARK_IGNORED_SURV_MODIFY,
			  FAN_OPEN | FAN_CLOSE, AT_FDCWD, fname);

	/* This event should be ignored */
	fd = SAFE_OPEN(fname, O_RDWR);

	SAFE_WRITE(SAFE_WRITE_ALL, fd, fname, 1);
	event_set[tst_count] = FAN_MODIFY;
	tst_count++;

	/* This event should be still ignored */
	SAFE_CLOSE(fd);

	/* This event should still be ignored */
	fd = SAFE_OPEN(fname, O_RDWR);

	/* Read events to verify open & close were ignored */
	ret = SAFE_READ(0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	len += ret;

	/* Now remove open and close from ignored mask */
	SAFE_FANOTIFY_MARK(fd_notify,
			  FAN_MARK_REMOVE | mark->flag | FAN_MARK_IGNORED_MASK,
			  FAN_OPEN | FAN_CLOSE, AT_FDCWD, fname);

	SAFE_CLOSE(fd);
	event_set[tst_count] = FAN_CLOSE_WRITE;
	tst_count++;

	/* Read events to verify close was generated */
	ret = SAFE_READ(0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	len += ret;

	if (TST_TOTAL != tst_count) {
		tst_brk(TBROK,
			"TST_TOTAL (%d) and tst_count (%d) are not "
			"equal", TST_TOTAL, tst_count);
	}
	tst_count = 0;

	/*
	 * check events
	 */
	while (i < len) {
		struct fanotify_event_metadata *event;

		event = (struct fanotify_event_metadata *)&event_buf[i];
		if (test_num >= TST_TOTAL) {
			tst_res(TFAIL,
				"got unnecessary event: mask=%llx "
				"pid=%u fd=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd);
		} else if (!(event->mask & event_set[test_num])) {
			tst_res(TFAIL,
				"got event: mask=%llx (expected %llx) "
				"pid=%u fd=%d",
				(unsigned long long)event->mask,
				event_set[test_num],
				(unsigned int)event->pid, event->fd);
		} else if (event->pid != getpid()) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u "
				"(expected %u) fd=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid,
				(unsigned int)getpid(),
				event->fd);
		} else {
			if (event->fd == -2 || (event->fd == FAN_NOFD &&
			    (tc->init_flags & FAN_REPORT_FID)))
				goto pass;
			ret = read(event->fd, buf, BUF_SIZE);
			if (ret != (int)strlen(fname)) {
				tst_res(TFAIL,
					"cannot read from returned fd "
					"of event: mask=%llx pid=%u "
					"fd=%d ret=%d (errno=%d)",
					(unsigned long long)event->mask,
					(unsigned int)event->pid,
					event->fd, ret, errno);
			} else if (memcmp(buf, fname, strlen(fname))) {
				tst_res(TFAIL,
					"wrong data read from returned fd "
					"of event: mask=%llx pid=%u "
					"fd=%d",
					(unsigned long long)event->mask,
					(unsigned int)event->pid,
					event->fd);
			} else {
pass:
				tst_res(TPASS,
					"got event: mask=%llx pid=%u fd=%d",
					(unsigned long long)event->mask,
					(unsigned int)event->pid, event->fd);
			}
		}

		/*
		 * We have verified the data now so close fd and
		 * invalidate it so that we don't check it again
		 * unnecessarily
		 */
		if (event->fd >= 0)
			SAFE_CLOSE(event->fd);
		event->fd = -2;
		event->mask &= ~event_set[test_num];

		/* No events left in current mask? Go for next event */
		if (event->mask == 0)
			i += event->event_len;

		test_num++;
	}

	for (; test_num < TST_TOTAL; test_num++) {
		tst_res(TFAIL, "didn't get event: mask=%llx",
			event_set[test_num]);

	}


	/*
	 * Try to setup a bogus mark on test tmp dir, to check if marks on
	 * different filesystems are supported.
	 * When tested fs has zero fsid (e.g. fuse) and events are reported
	 * with fsid+fid, watching different filesystems is not supported.
	 */
	ret = report_fid ? inode_mark_fid_xdev : 0;
	TST_EXP_FD_OR_FAIL(fanotify_mark(fd_notify, FAN_MARK_ADD, FAN_CLOSE_WRITE,
					 AT_FDCWD, "."), ret);

	/* Remove mark to clear FAN_MARK_IGNORED_SURV_MODIFY */
	SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_REMOVE | mark->flag,
			  FAN_ACCESS | FAN_MODIFY | FAN_CLOSE | FAN_OPEN,
			  AT_FDCWD, fname);

	SAFE_CLOSE(fd_notify);
}

static void setup(void)
{
	int fd;

	/* Check for kernel fanotify support */
	fd = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF, O_RDONLY);
	SAFE_CLOSE(fd);

	sprintf(fname, MOUNT_PATH"/tfile_%d", getpid());
	SAFE_FILE_PRINTF(fname, "1");

	fan_report_fid_unsupported = fanotify_init_flags_supported_on_fs(FAN_REPORT_FID, fname);
	filesystem_mark_unsupported = fanotify_mark_supported_on_fs(FAN_MARK_FILESYSTEM, fname);
	mount_mark_fid_unsupported = fanotify_flags_supported_on_fs(FAN_REPORT_FID,
								    FAN_MARK_MOUNT,
								    FAN_OPEN, fname);
	/*
	 * When mount mark is not supported due to zero fsid (e.g. fuse) or if TMPDIR has
	 * non-uniform fsid (e.g. btrfs subvol), multi fs inode marks are not supported.
	 */
	if (mount_mark_fid_unsupported && errno == ENODEV) {
		tst_res(TINFO, "filesystem %s does not support reporting events with fid from multi fs",
				tst_device->fs_type);
		inode_mark_fid_xdev = EXDEV;
	}

	if (fanotify_flags_supported_on_fs(FAN_REPORT_FID, FAN_MARK_MOUNT, FAN_OPEN, ".")) {
		inode_mark_fid_xdev = errno;
		tst_res(TINFO, "TMPDIR does not support reporting events with fid from multi fs");
	}
}

static void cleanup(void)
{
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.test = test_fanotify,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MOUNT_PATH,
	.all_filesystems = 1,
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
