// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 CTERA Networks.  All Rights Reserved.
 * Author: Amir Goldstein <amir73il@gmail.com>
 */

/*\
 * [Description]
 *
 * Check that inotify work for an overlayfs directory after copy up and
 * drop caches.
 *
 * An inotify watch pins the directory inode in cache, but not the dentry.
 * The watch will not report events on the directory if overlayfs does not
 * obtain the pinned inode to the new allocated dentry after drop caches.
 *
 * The problem has been fixed by commit:
 * 31747eda41ef ("ovl: hash directory inodes for fsnotify").
 *
 * [Algorithm]
 *
 * Add watch on an overlayfs lower directory then chmod directory and drop
 * dentry and inode caches. Execute operations on directory and child and
 * expect events to be reported on directory watch.
 */

#include "config.h"

#if defined(HAVE_SYS_INOTIFY_H)
# include <sys/inotify.h>
#endif
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
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
	char name[BUF_SIZE];
	unsigned int mask;
};

#define DIR_NAME "test_dir"
#define DIR_PATH OVL_MNT"/"DIR_NAME
#define FILE_NAME "test_file"
#define FILE_PATH OVL_MNT"/"DIR_NAME"/"FILE_NAME

static const char mntpoint[] = OVL_BASE_MNTPOINT;
static struct event_t event_set[EVENT_MAX];
static char event_buf[EVENT_BUF_LEN];

void verify_inotify(void)
{
	int test_cnt = 0;

	/*
	 * generate sequence of events
	 */
	SAFE_CHMOD(DIR_PATH, 0755);
	event_set[test_cnt].mask = IN_ISDIR | IN_ATTRIB;
	strcpy(event_set[test_cnt].name, "");
	test_cnt++;

	SAFE_TOUCH(FILE_PATH, 0644, NULL);
	event_set[test_cnt].mask = IN_OPEN;
	strcpy(event_set[test_cnt].name, FILE_NAME);
	test_cnt++;
	event_set[test_cnt].mask = IN_CLOSE_WRITE;
	strcpy(event_set[test_cnt].name, FILE_NAME);
	test_cnt++;
	event_set[test_cnt].mask = IN_ATTRIB;
	strcpy(event_set[test_cnt].name, FILE_NAME);
	test_cnt++;

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
		} else if ((event_set[test_num].mask == event->mask)
				&&
				(!strncmp
				 (event_set[test_num].name, event->name,
				  event->len))) {
			tst_res(TPASS,
				"get event: wd=%d mask=%08x "
				"cookie=%-5u len=%-2u name=\"%.*s\"",
				event->wd, event->mask,
				event->cookie, event->len,
				event->len, event->name);
		} else {
			tst_res(TFAIL, "get event: wd=%d mask=%08x "
				"(expected %x) cookie=%-5u len=%-2u "
				"name=\"%.*s\" (expected \"%s\") %d",
				event->wd, event->mask,
				event_set[test_num].mask,
				event->cookie, event->len, event->len,
				event->name, event_set[test_num].name,
				strcmp(event_set[test_num].name,
					event->name));
		}
		test_num++;
		i += EVENT_SIZE + event->len;
	}

	for (; test_num < test_cnt; test_num++) {
		tst_res(TFAIL, "didn't get event: mask=%08x ",
			event_set[test_num].mask);
	}
}

static void setup(void)
{
	struct stat buf;

	/* Setup an overlay mount with lower dir and file */
	SAFE_UMOUNT(OVL_MNT);
	SAFE_MKDIR(OVL_LOWER"/"DIR_NAME, 0755);
	SAFE_TOUCH(OVL_LOWER"/"DIR_NAME"/"FILE_NAME, 0644, NULL);
	SAFE_MOUNT_OVERLAY();

	fd_notify = SAFE_MYINOTIFY_INIT1(O_NONBLOCK);

	/* Setup a watch on an overlayfs lower directory */
	wd = SAFE_MYINOTIFY_ADD_WATCH(fd_notify, DIR_PATH, IN_ALL_EVENTS);
	reap_wd = 1;

	SAFE_STAT(DIR_PATH, &buf);
	tst_res(TINFO, DIR_PATH " ino=%lu", buf.st_ino);

	/* Drop dentry caches, so overlayfs will allocate a new dentry */
	SAFE_FILE_PRINTF("/proc/sys/vm/drop_caches", "2");

	/* Copy up directory to make it a merge directory */
	SAFE_CHMOD(DIR_PATH, 0700);

	/* Lookup directory and see if we got the watched directory inode */
	SAFE_STAT(DIR_PATH, &buf);
	tst_res(TINFO, DIR_PATH " ino=%lu", buf.st_ino);
}

static void cleanup(void)
{
	if (reap_wd && myinotify_rm_watch(fd_notify, wd) < 0) {
		tst_res(TWARN, "inotify_rm_watch (%d, %d) failed",
			fd_notify, wd);
	}

	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.needs_root = 1,
	.mount_device = 1,
	.needs_overlay = 1,
	.mntpoint = mntpoint,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_inotify,
};

#else
	TST_TEST_TCONF("system doesn't have required inotify support");
#endif
