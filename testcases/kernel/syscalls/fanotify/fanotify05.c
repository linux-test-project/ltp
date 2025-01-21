// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 SUSE Linux.  All Rights Reserved.
 *
 * Started by Jan Kara <jack@suse.cz>
 */

/*\
 * [Description]
 * Check that fanotify overflow event is properly generated.
 *
 * [Algorithm]
 * Generate enough events without reading them and check that overflow
 * event is generated.
 */

#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include "tst_test.h"
#include "tst_timer.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define MOUNT_PATH "fs_mnt"
#define FNAME_PREFIX "fname_"
#define FNAME_PREFIX_LEN 6
#define PATH_PREFIX MOUNT_PATH "/" FNAME_PREFIX

#define SYSFS_MAX_EVENTS "/proc/sys/fs/fanotify/max_queued_events"

/* In older kernels this limit is fixed in kernel */
#define DEFAULT_MAX_EVENTS 16384

static int max_events;

static struct tcase {
	const char *tname;
	unsigned int init_flags;
} tcases[] = {
	{
		"Limited queue",
		FAN_CLASS_NOTIF,
	},
	{
		"Unlimited queue",
		FAN_CLASS_NOTIF | FAN_UNLIMITED_QUEUE,
	},
};

#define BUF_SIZE 256
static char fname[BUF_SIZE];
static char symlnk[BUF_SIZE];
static char fdpath[BUF_SIZE];
static int fd, fd_notify;

static struct fanotify_event_metadata event;

static void event_res(struct fanotify_event_metadata *event, int i)
{
	int len = 0;
	const char *filename;

	sprintf(symlnk, "/proc/self/fd/%d", event->fd);
	len = readlink(symlnk, fdpath, sizeof(fdpath));
	if (len < 0)
		len = 0;
	fdpath[len] = 0;
	filename = basename(fdpath);

	if (len > FNAME_PREFIX_LEN && atoi(filename + FNAME_PREFIX_LEN) != i)
		tst_res(TFAIL, "Got event #%d out of order filename=%s", i, filename);
	else if (i == 0)
		tst_res(TINFO, "Got event #%d filename=%s", i, filename);
}

static void generate_events(int open_flags, int num_files)
{
	long long elapsed_ms;
	int i;

	tst_timer_start(CLOCK_MONOTONIC);

	for (i = 0; i < num_files; i++) {
		sprintf(fname, PATH_PREFIX "%d", i);
		fd = SAFE_OPEN(fname, open_flags, 0644);
		SAFE_CLOSE(fd);
	}

	tst_timer_stop();

	elapsed_ms = tst_timer_elapsed_ms();

	tst_res(TINFO, "%s %d files in %llims",
		(open_flags & O_CREAT) ? "Created" : "Opened", i, elapsed_ms);
}

static void test_fanotify(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int len, nevents = 0, got_overflow = 0;
	int num_files = max_events + 1;
	int expect_overflow = !(tc->init_flags & FAN_UNLIMITED_QUEUE);

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	fd_notify = SAFE_FANOTIFY_INIT(tc->init_flags | FAN_NONBLOCK, O_RDONLY);

	SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_MOUNT | FAN_MARK_ADD, FAN_OPEN,
			   AT_FDCWD, MOUNT_PATH);

	/*
	 * Generate events on unique files so they won't be merged
	 */
	generate_events(O_RDWR | O_CREAT, num_files);

	/*
	 * Generate more events on the same files that me be merged
	 */
	generate_events(O_RDONLY, num_files);

	while (1) {
		/*
		 * get list on events
		 */
		len = read(fd_notify, &event, sizeof(event));
		if (len < 0) {
			if (errno != EAGAIN) {
				tst_brk(TBROK | TERRNO,
					"read of notification event failed");
			}
			if (!got_overflow)
				tst_res(expect_overflow ? TFAIL : TPASS, "Overflow event not generated!\n");
			break;
		}
		if (event.fd != FAN_NOFD) {
			/*
			 * Verify that events generated on unique files
			 * are received by the same order they were generated.
			 */
			if (nevents < num_files)
				event_res(&event, nevents);
			close(event.fd);
		}
		nevents++;

		/*
		 * check events
		 */
		if (event.mask != FAN_OPEN &&
		    event.mask != FAN_Q_OVERFLOW) {
			tst_res(TFAIL,
				"got event: mask=%llx (expected %llx) pid=%u fd=%d",
				(unsigned long long)event.mask,
				(unsigned long long)FAN_OPEN,
				(unsigned int)event.pid, event.fd);
			break;
		}
		if (event.mask == FAN_Q_OVERFLOW) {
			if (got_overflow || event.fd != FAN_NOFD) {
				tst_res(TFAIL,
					"%s overflow event: mask=%llx pid=%u fd=%d",
					got_overflow ? "unexpected" : "invalid",
					(unsigned long long)event.mask,
					(unsigned int)event.pid,
					event.fd);
				break;
			}
			tst_res(expect_overflow ? TPASS : TFAIL,
				"Got an overflow event: pid=%u fd=%d",
				(unsigned int)event.pid, event.fd);
			got_overflow = 1;
		}
	}
	tst_res(TINFO, "Got %d events", nevents);
	SAFE_CLOSE(fd_notify);
}

static void setup(void)
{
	int fd;

	/* Check for kernel fanotify support */
	fd = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF, O_RDONLY);
	SAFE_CLOSE(fd);

	/* In older kernels this limit is fixed in kernel */
	if (access(SYSFS_MAX_EVENTS, F_OK) && errno == ENOENT)
		max_events = DEFAULT_MAX_EVENTS;
	else
		SAFE_FILE_SCANF(SYSFS_MAX_EVENTS, "%d", &max_events);
	tst_res(TINFO, "max_queued_events=%d", max_events);
}

static void cleanup(void)
{
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.timeout = 13,
	.test = test_fanotify,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MOUNT_PATH,
};
#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
