/*
 * Copyright (c) 2018 CTERA Networks.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 or any later of the GNU General Public License
 * as published by the Free Software Foundation.
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
 * Started by Amir Goldstein <amir73il@gmail.com>
 *
 * DESCRIPTION
 *     Check that inotify work for an overlayfs file after copy up and
 *     drop caches.
 *
 *     An inotify watch pins the file inode in cache, but not the dentry.
 *     The watch will not report events on the file if overlayfs does not
 *     obtain the pinned inode to the new allocated dentry after drop caches.
 *
 *     The problem has been fixed by commit:
 *       764baba80168 "ovl: hash non-dir by lower inode for fsnotify"
 *
 *
 * ALGORITHM
 *     Add watch on an overlayfs lower file then chmod file and drop dentry
 *     and inode caches. Execute operations on file and expect events to be
 *     reported on file watch.
 */

#include "config.h"

#if defined(HAVE_SYS_INOTIFY_H)
# include <sys/inotify.h>
#endif
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/mount.h>
#include <limits.h>
#include "tst_test.h"
#include "inotify.h"

#if defined(HAVE_SYS_INOTIFY_H)

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct inotify_event))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * (EVENT_SIZE + 16))

#define BUF_SIZE 256
static int fd_notify, reap_wd;
static int wd;

struct event_t {
	unsigned int mask;
};

#define OVL_MNT "ovl"
#define FILE_NAME "test_file"
#define FILE_PATH OVL_MNT"/"FILE_NAME

static int ovl_mounted;

static struct event_t event_set[EVENT_MAX];

static char event_buf[EVENT_BUF_LEN];

void verify_inotify(void)
{
	int test_cnt = 0;

	/*
	 * generate sequence of events
	 */
	SAFE_CHMOD(FILE_PATH, 0644);
	event_set[test_cnt].mask = IN_ATTRIB;
	test_cnt++;

	SAFE_FILE_PRINTF(FILE_PATH, "1");

	event_set[test_cnt].mask = IN_OPEN;
	test_cnt++;

	event_set[test_cnt].mask = IN_CLOSE_WRITE;
	test_cnt++;

	/* Make sure events on upper/lower do not show in overlay watch */
	SAFE_TOUCH("lower/"FILE_NAME, 0644, NULL);
	SAFE_TOUCH("upper/"FILE_NAME, 0644, NULL);

	int len = read(fd_notify, event_buf, EVENT_BUF_LEN);
	if (len == -1 && errno != EAGAIN) {
		tst_brk(TBROK | TERRNO,
			"read(%d, buf, %zu) failed",
			fd_notify, EVENT_BUF_LEN);
	}

	int i = 0, test_num = 0;
	while (i < len) {
		struct inotify_event *event;
		event = (struct inotify_event *)&event_buf[i];
		if (test_num >= test_cnt) {
			tst_res(TFAIL,
				"get unnecessary event: "
				"wd=%d mask=%08x cookie=%-5u len=%-2u "
				"name=\"%.*s\"", event->wd, event->mask,
				event->cookie, event->len, event->len,
				event->name);
		} else if (event_set[test_num].mask == event->mask &&
			   !event->len) {
			tst_res(TPASS,
				"get event: wd=%d mask=%08x "
				"cookie=%-5u len=%-2u",
				event->wd, event->mask,
				event->cookie, event->len);
		} else {
			tst_res(TFAIL, "get event: wd=%d mask=%08x "
				"(expected %x) cookie=%-5u len=%-2u "
				"name=\"%.*s\" (expected \"\")",
				event->wd, event->mask,
				event_set[test_num].mask,
				event->cookie, event->len, event->len,
				event->name);
		}
		test_num++;
		i += EVENT_SIZE + event->len;
	}

	for (; test_num < test_cnt; test_num++) {
		tst_res(TFAIL, "didn't get event: mask=%x ",
			event_set[test_num].mask);
	}
}

static void setup(void)
{
	struct stat buf;
	int ret;

	/* Setup an overlay mount with lower file */
	SAFE_MKDIR("lower", 0755);
	SAFE_TOUCH("lower/"FILE_NAME, 0644, NULL);
	SAFE_MKDIR("upper", 0755);
	SAFE_MKDIR("work", 0755);
	SAFE_MKDIR(OVL_MNT, 0755);
	ret = mount("overlay", OVL_MNT, "overlay", 0,
		    "lowerdir=lower,upperdir=upper,workdir=work");
	if (ret < 0) {
		if (errno == ENODEV) {
			tst_brk(TCONF,
				"overlayfs is not configured in this kernel.");
		} else {
			tst_brk(TBROK | TERRNO,
				"overlayfs mount failed");
		}
	}
	ovl_mounted = 1;

	fd_notify = myinotify_init1(O_NONBLOCK);
	if (fd_notify < 0) {
		if (errno == ENOSYS) {
			tst_brk(TCONF,
				"inotify is not configured in this kernel.");
		} else {
			tst_brk(TBROK | TERRNO,
				"inotify_init () failed");
		}
	}

	/* Setup a watch on an overlayfs lower file */
	if ((wd = myinotify_add_watch(fd_notify, FILE_PATH,
				IN_ATTRIB | IN_OPEN | IN_CLOSE_WRITE)) < 0) {
		tst_brk(TBROK | TERRNO,
			"inotify_add_watch (%d, " FILE_PATH ", "
			"IN_ATTRIB | IN_OPEN | IN_CLOSE_WRITE) failed",
			fd_notify);
		reap_wd = 1;
	};

	SAFE_STAT(FILE_PATH, &buf);
	tst_res(TINFO, FILE_PATH " ino=%lu, dev=%u:%u", buf.st_ino,
			major(buf.st_dev), minor(buf.st_dev));

	/* Drop dentry caches, so overlayfs will allocate a new dentry */
	SAFE_FILE_PRINTF("/proc/sys/vm/drop_caches", "2");

	/* Copy up file */
	SAFE_CHMOD(FILE_PATH, 0600);

	/* Lookup file and see if we got the watched file inode number */
	SAFE_STAT(FILE_PATH, &buf);
	tst_res(TINFO, FILE_PATH " ino=%lu, dev=%u:%u", buf.st_ino,
			major(buf.st_dev), minor(buf.st_dev));
}

static void cleanup(void)
{
	if (reap_wd && myinotify_rm_watch(fd_notify, wd) < 0) {
		tst_res(TWARN,
			"inotify_rm_watch (%d, %d) failed,", fd_notify, wd);

	}

	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);

	if (ovl_mounted)
		SAFE_UMOUNT(OVL_MNT);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_inotify,
};

#else
	TST_TEST_TCONF("system doesn't have required inotify support");
#endif
