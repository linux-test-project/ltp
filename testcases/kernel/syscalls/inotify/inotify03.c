/*
 * Copyright (c) 2008 Parallels.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Started by Andrew Vagin <avagin@gmail.com>
 *
 * DESCRIPTION
 *	Check that inotify get IN_UNMOUNT event and
 *	don't block the umount command.
 *
 * ALGORITHM
 *	Execute sequence file's operation and check return events
 *
 */
#include "config.h"

#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <signal.h>
#include "tst_test.h"
#include "inotify.h"

#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE (sizeof(struct inotify_event))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN		(EVENT_MAX * (EVENT_SIZE + 16))

#define BUF_SIZE 1024
static char fname[BUF_SIZE];
static int fd, fd_notify;
static int wd;

static unsigned int event_set[EVENT_MAX];

static char event_buf[EVENT_BUF_LEN];

#define DIR_MODE	(S_IRWXU | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP)

static char *mntpoint = "mntpoint";
static int mount_flag;

void verify_inotify(void)
{
	int ret;
	int len, i, test_num;

	int test_cnt = 0;

	SAFE_MOUNT(tst_device->dev, mntpoint, tst_device->fs_type, 0, NULL);
	mount_flag = 1;

	wd = myinotify_add_watch(fd_notify, fname, IN_ALL_EVENTS);
	if (wd < 0) {
		tst_brk(TBROK | TERRNO,
			"inotify_add_watch (%d, %s, IN_ALL_EVENTS) failed.",
			fd_notify, fname);
	}

	event_set[test_cnt] = IN_UNMOUNT;
	test_cnt++;
	event_set[test_cnt] = IN_IGNORED;
	test_cnt++;

	/*check exit code from inotify_rm_watch */
	test_cnt++;

	tst_res(TINFO, "umount %s", tst_device->dev);
	TEST(tst_umount(mntpoint));
	if (TST_RET != 0) {
		tst_brk(TBROK, "umount(2) Failed "
			"while unmounting errno = %d : %s",
			TST_ERR, strerror(TST_ERR));
	}
	mount_flag = 0;

	len = read(fd_notify, event_buf, EVENT_BUF_LEN);
	if (len < 0) {
		tst_brk(TBROK | TERRNO,
			"read(%d, buf, %zu) failed", fd_notify, EVENT_BUF_LEN);
	}

	/* check events */
	test_num = 0;
	i = 0;
	while (i < len) {
		struct inotify_event *event;
		event = (struct inotify_event *)&event_buf[i];
		if (test_num >= (test_cnt - 1)) {
			tst_res(TFAIL,
				"get unnecessary event: wd=%d mask=%x "
				"cookie=%u len=%u",
				event->wd, event->mask,
				event->cookie, event->len);
		} else if (event_set[test_num] == event->mask) {
			tst_res(TPASS, "get event: wd=%d mask=%x"
				" cookie=%u len=%u",
				event->wd, event->mask,
				event->cookie, event->len);

		} else {
			tst_res(TFAIL, "get event: wd=%d mask=%x "
				"(expected %x) cookie=%u len=%u",
				event->wd, event->mask,
				event_set[test_num],
				event->cookie, event->len);
		}
		test_num++;
		i += EVENT_SIZE + event->len;
	}
	for (; test_num < test_cnt - 1; test_num++) {
		tst_res(TFAIL, "don't get event: mask=%x ",
			event_set[test_num]);

	}
	ret = myinotify_rm_watch(fd_notify, wd);
	if (ret != -1 || errno != EINVAL)
		tst_res(TFAIL | TERRNO,
			"inotify_rm_watch (%d, %d) didn't return EINVAL",
			fd_notify, wd);
	else
		tst_res(TPASS, "inotify_rm_watch (%d, %d) returned EINVAL",
			fd_notify, wd);
}

static void setup(void)
{
	int ret;

	SAFE_MKDIR(mntpoint, DIR_MODE);

	SAFE_MOUNT(tst_device->dev, mntpoint, tst_device->fs_type, 0, NULL);
	mount_flag = 1;

	sprintf(fname, "%s/tfile_%d", mntpoint, getpid());
	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);

	ret = write(fd, fname, 1);
	if (ret == -1) {
		tst_brk(TBROK | TERRNO,
			 "write(%d, %s, 1) failed", fd, fname);
	}

	/* close the file we have open */
	SAFE_CLOSE(fd);

	fd_notify = myinotify_init();
	if (fd_notify < 0) {
		if (errno == ENOSYS)
			tst_brk(TCONF,
				"inotify is not configured in this kernel.");
		else
			tst_brk(TBROK | TERRNO,
				"inotify_init failed");
	}

	tst_umount(mntpoint);
	mount_flag = 0;
}

static void cleanup(void)
{
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);

	if (mount_flag) {
		TEST(tst_umount(mntpoint));
		if (TST_RET != 0)
			tst_res(TWARN | TTERRNO, "umount(%s) failed",
				mntpoint);
	}
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.format_device = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_inotify,
};

#else
	TST_TEST_TCONF("system doesn't have required inotify support");
#endif
