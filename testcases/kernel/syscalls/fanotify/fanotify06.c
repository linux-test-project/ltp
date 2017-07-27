/*
 * Copyright (c) 2014 SUSE.  All Rights Reserved.
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
 * Started by Jan Kara <jack@suse.cz>
 *
 * DESCRIPTION
 *     Check that fanotify properly merges ignore mask of an inode and
 *     mountpoint.
 *
 * This is a regression test for:
 *
 *  commit 8edc6e1688fc8f02c8c1f53a2ec4928cb1055f4d
 *  Author: Jan Kara <jack@suse.cz>
 *  Date:   Thu Nov 13 15:19:33 2014 -0800
 *
 *      fanotify: fix notification of groups with inode & mount marks
 */
#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include "test.h"
#include "lapi/syscalls.h"
#include "fanotify.h"
#include "safe_macros.h"

char *TCID = "fanotify06";

#if defined(HAVE_SYS_FANOTIFY_H)
#include <sys/fanotify.h>

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct fanotify_event_metadata))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * EVENT_SIZE)

static void setup(void);
static void cleanup(void);

unsigned int fanotify_prio[] = {
	FAN_CLASS_PRE_CONTENT,
	FAN_CLASS_CONTENT,
	FAN_CLASS_NOTIF
};
#define FANOTIFY_PRIORITIES ARRAY_SIZE(fanotify_prio)

#define GROUPS_PER_PRIO 3

int TST_TOTAL = GROUPS_PER_PRIO * FANOTIFY_PRIORITIES;

#define BUF_SIZE 256
static char fname[BUF_SIZE];
static int fd;
static int fd_notify[FANOTIFY_PRIORITIES][GROUPS_PER_PRIO];

static char event_buf[EVENT_BUF_LEN];

#define MOUNT_NAME "mntpoint"
static int mount_created;

static void create_fanotify_groups(void)
{
	unsigned int p, i;
	int ret;

	for (p = 0; p < FANOTIFY_PRIORITIES; p++) {
		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			fd_notify[p][i] = fanotify_init(fanotify_prio[p] |
							FAN_NONBLOCK,
							O_RDONLY);
			if (fd_notify[p][i] < 0) {
				if (errno == ENOSYS) {
					tst_brkm(TCONF, cleanup,
						 "fanotify is not configured in"
						 " this kernel.");
				} else {
					tst_brkm(TBROK | TERRNO, cleanup,
						 "fanotify_init failed");
				}
			}
			/* Add mount mark for each group */
			ret = fanotify_mark(fd_notify[p][i],
					    FAN_MARK_ADD | FAN_MARK_MOUNT,
					    FAN_MODIFY,
					    AT_FDCWD, ".");
			if (ret < 0) {
				tst_brkm(TBROK | TERRNO, cleanup,
					 "fanotify_mark(%d, FAN_MARK_ADD | "
					 "FAN_MARK_MOUNT, FAN_MODIFY, AT_FDCWD,"
					 " '.') failed", fd_notify[p][i]);
			}
			/* Add ignore mark for groups with higher priority */
			if (p == 0)
				continue;
			ret = fanotify_mark(fd_notify[p][i],
					    FAN_MARK_ADD |
					    FAN_MARK_IGNORED_MASK |
					    FAN_MARK_IGNORED_SURV_MODIFY,
					    FAN_MODIFY, AT_FDCWD, fname);
			if (ret < 0) {
				tst_brkm(TBROK | TERRNO, cleanup,
					 "fanotify_mark(%d, FAN_MARK_ADD | "
					 "FAN_MARK_IGNORED_MASK | "
					 "FAN_MARK_IGNORED_SURV_MODIFY, "
					 "FAN_MODIFY, AT_FDCWD, %s) failed",
					 fd_notify[p][i], fname);
			}
		}
	}
}

static void cleanup_fanotify_groups(void)
{
	unsigned int i, p;

	for (p = 0; p < FANOTIFY_PRIORITIES; p++) {
		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			if (fd_notify[p][i] && fd_notify[p][i] != -1) {
				if (close(fd_notify[p][i]) == -1)
					tst_resm(TWARN, "close(%d) failed",
						 fd_notify[p][i]);
				fd_notify[p][i] = 0;
			}
		}
	}
}

static void verify_event(int group, struct fanotify_event_metadata *event)
{
	if (event->mask != FAN_MODIFY) {
		tst_resm(TFAIL, "group %d get event: mask %llx (expected %llx) "
			 "pid=%u fd=%u", group, (unsigned long long)event->mask,
			 (unsigned long long)FAN_MODIFY,
			 (unsigned)event->pid, event->fd);
	} else if (event->pid != getpid()) {
		tst_resm(TFAIL, "group %d get event: mask %llx pid=%u "
			 "(expected %u) fd=%u", group,
			 (unsigned long long)event->mask, (unsigned)event->pid,
			 (unsigned)getpid(), event->fd);
	} else {
		tst_resm(TPASS, "group %d get event: mask %llx pid=%u fd=%u",
			 group, (unsigned long long)event->mask,
			 (unsigned)event->pid, event->fd);
	}
}

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int ret;
		unsigned int p, i;
		struct fanotify_event_metadata *event;

		create_fanotify_groups();

		/*
		 * generate sequence of events
		 */
		fd = SAFE_OPEN(cleanup, fname, O_RDWR);
		SAFE_WRITE(cleanup, 1, fd, fname, strlen(fname));
		SAFE_CLOSE(cleanup, fd);

		/* First verify all groups without ignore mask got the event */
		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			ret = read(fd_notify[0][i], event_buf, EVENT_BUF_LEN);
			if (ret < 0) {
				if (errno == EAGAIN) {
					tst_resm(TFAIL, "group %d did not get "
						 "event", i);
				}
				tst_brkm(TBROK | TERRNO, cleanup,
					 "reading fanotify events failed");
			}
			if (ret < (int)FAN_EVENT_METADATA_LEN) {
				tst_brkm(TBROK, cleanup,
					 "short read when reading fanotify "
					 "events (%d < %d)", ret,
					 (int)EVENT_BUF_LEN);
			}
			event = (struct fanotify_event_metadata *)event_buf;
			if (ret > (int)event->event_len) {
				tst_resm(TFAIL, "group %d got more than one "
					 "event (%d > %d)", i, ret,
					 event->event_len);
			} else
				verify_event(i, event);
			close(event->fd);
		}
		for (p = 1; p < FANOTIFY_PRIORITIES; p++) {
			for (i = 0; i < GROUPS_PER_PRIO; i++) {
				ret = read(fd_notify[p][i], event_buf, EVENT_BUF_LEN);
				if (ret > 0) {
					tst_resm(TFAIL, "group %d got event",
						 p*GROUPS_PER_PRIO + i);
				} else if (ret == 0) {
					tst_brkm(TBROK, cleanup, "zero length "
						 "read from fanotify fd");
				} else if (errno != EAGAIN) {
					tst_brkm(TBROK | TERRNO, cleanup,
						 "reading fanotify events failed");
				} else {
					tst_resm(TPASS, "group %d got no event",
						 p*GROUPS_PER_PRIO + i);
				}
			}
		}
		cleanup_fanotify_groups();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_require_root();
	tst_tmpdir();

	SAFE_MKDIR(cleanup, MOUNT_NAME, 0755);
	SAFE_MOUNT(cleanup, MOUNT_NAME, MOUNT_NAME, NULL, MS_BIND, NULL);
	mount_created = 1;
	SAFE_CHDIR(cleanup, MOUNT_NAME);

	sprintf(fname, "tfile_%d", getpid());
	fd = SAFE_OPEN(cleanup, fname, O_RDWR | O_CREAT, 0700);
	SAFE_WRITE(cleanup, 1, fd, fname, 1);

	/* close the file we have open */
	SAFE_CLOSE(cleanup, fd);
}

static void cleanup(void)
{
	cleanup_fanotify_groups();

	if (chdir(tst_get_tmpdir()) < 0)
		tst_brkm(TBROK, NULL, "chdir(tmpdir) failed");

	if (mount_created && tst_umount(MOUNT_NAME) < 0)
		tst_brkm(TBROK | TERRNO, NULL, "umount failed");

	tst_rmdir();
}

#else

int main(void)
{
	tst_brkm(TCONF, NULL, "system doesn't have required fanotify support");
}

#endif
