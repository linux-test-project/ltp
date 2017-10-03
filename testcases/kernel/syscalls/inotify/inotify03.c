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
#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"
#include "inotify.h"

char *TCID = "inotify03";
int TST_TOTAL = 3;

#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE (sizeof(struct inotify_event))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN		(EVENT_MAX * (EVENT_SIZE + 16))

static void setup(void);
static void cleanup(void);

#define BUF_SIZE 1024
static char fname[BUF_SIZE];
static int fd, fd_notify;
static int wd;

static int event_set[EVENT_MAX];

static char event_buf[EVENT_BUF_LEN];

#define DIR_MODE	(S_IRWXU | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP)

static char *mntpoint = "mntpoint";
static int mount_flag;
static const char *device;
static const char *fs_type;

int main(int argc, char *argv[])
{
	int ret;
	int len, i, test_num;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	tst_count = 0;

	event_set[tst_count] = IN_UNMOUNT;
	tst_count++;
	event_set[tst_count] = IN_IGNORED;
	tst_count++;

	/*check exit code from inotify_rm_watch */
	tst_count++;

	if (TST_TOTAL != tst_count) {
		tst_brkm(TBROK, cleanup,
			 "TST_TOTAL and tst_count are not equal");
	}
	tst_count = 0;

	tst_resm(TINFO, "umount %s", device);
	TEST(tst_umount(mntpoint));
	if (TEST_RETURN != 0) {
		tst_brkm(TBROK, cleanup, "umount(2) Failed "
			 "while unmounting errno = %d : %s",
			 TEST_ERRNO, strerror(TEST_ERRNO));
	}
	mount_flag = 0;

	len = read(fd_notify, event_buf, EVENT_BUF_LEN);
	if (len < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "read(%d, buf, %zu) failed", fd_notify, EVENT_BUF_LEN);
	}

	/* check events */
	test_num = 0;
	i = 0;
	while (i < len) {
		struct inotify_event *event;
		event = (struct inotify_event *)&event_buf[i];
		if (test_num >= (TST_TOTAL - 1)) {
			tst_resm(TFAIL,
				 "get unnecessary event: wd=%d mask=%x "
				 "cookie=%u len=%u",
				 event->wd, event->mask,
				 event->cookie, event->len);
		} else if (event_set[test_num] == event->mask) {
			tst_resm(TPASS, "get event: wd=%d mask=%x"
				 " cookie=%u len=%u",
				 event->wd, event->mask,
				 event->cookie, event->len);

		} else {
			tst_resm(TFAIL, "get event: wd=%d mask=%x "
				 "(expected %x) cookie=%u len=%u",
				 event->wd, event->mask,
				 event_set[test_num],
				 event->cookie, event->len);
		}
		test_num++;
		i += EVENT_SIZE + event->len;
	}
	for (; test_num < TST_TOTAL - 1; test_num++) {
		tst_resm(TFAIL, "don't get event: mask=%x ",
			 event_set[test_num]);

	}
	ret = myinotify_rm_watch(fd_notify, wd);
	if (ret != -1 || errno != EINVAL)
		tst_resm(TFAIL | TERRNO,
			 "inotify_rm_watch (%d, %d) didn't return EINVAL",
			 fd_notify, wd);
	else
		tst_resm(TPASS, "inotify_rm_watch (%d, %d) returned EINVAL",
			 fd_notify, wd);

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int ret;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	fs_type = tst_dev_fs_type();

	tst_tmpdir();

	device = tst_acquire_device(cleanup);
	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	SAFE_MKDIR(cleanup, mntpoint, DIR_MODE);

	/* Call mount(2) */
	tst_resm(TINFO, "mount %s to %s fs_type=%s", device, mntpoint, fs_type);
	TEST(mount(device, mntpoint, fs_type, 0, NULL));

	/* check return code */
	if (TEST_RETURN != 0) {
		tst_brkm(TBROK | TTERRNO, cleanup, "mount(2) failed");
	}
	mount_flag = 1;

	sprintf(fname, "%s/tfile_%d", mntpoint, getpid());
	fd = SAFE_OPEN(cleanup, fname, O_RDWR | O_CREAT, 0700);

	ret = write(fd, fname, 1);
	if (ret == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "write(%d, %s, 1) failed", fd, fname);
	}

	/* close the file we have open */
	SAFE_CLOSE(cleanup, fd);

	fd_notify = myinotify_init();

	if (fd_notify < 0) {
		if (errno == ENOSYS)
			tst_brkm(TCONF, cleanup,
				 "inotify is not configured in this kernel.");
		else
			tst_brkm(TBROK | TERRNO, cleanup,
				 "inotify_init failed");
	}

	wd = myinotify_add_watch(fd_notify, fname, IN_ALL_EVENTS);
	if (wd < 0)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "inotify_add_watch (%d, %s, IN_ALL_EVENTS) failed.",
			 fd_notify, fname);
}

static void cleanup(void)
{
	if (fd_notify > 0 && close(fd_notify) == -1)
		tst_resm(TWARN | TERRNO, "close(%d) failed", fd_notify);

	if (mount_flag) {
		TEST(tst_umount(mntpoint));
		if (TEST_RETURN != 0)
			tst_resm(TWARN | TTERRNO, "umount(%s) failed",
				 mntpoint);
	}

	tst_release_device(device);

	tst_rmdir();
}

#else

int main(void)
{
	tst_brkm(TCONF, NULL, "system doesn't have required inotify support");
}

#endif
