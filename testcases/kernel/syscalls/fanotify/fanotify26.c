// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Edvard Conrad <edvard@tuta.io>
 */

/*\
 * Check that a permission event on a stacked filesystem does not deadlock the
 * :manpage:`fanotify(7)` listener.
 *
 * When the kernel reports a permission event, it creates the event file
 * descriptor with an open that must not generate further events. If the
 * reported path is on a stacked filesystem, that open opens the real file on
 * the backing filesystem. Should the backing file not inherit "generate no
 * events", and the listener watch the backing filesystem as well, the kernel
 * queues a second permission event to the same group while the listener is
 * still inside :manpage:`read(2)`. Only the listener can answer it, so a
 * listener that reads with a single thread never returns, and every task that
 * touches a watched filesystem queues up behind it.
 *
 * Regular files and directories reach their real file through different
 * paths in overlayfs, so both are tested. For each, the first test case marks
 * the overlay only and must pass on any kernel, the second one marks the
 * backing filesystem as well, which is the shape an anti-malware scanner uses
 * on an overlayfs root.
 *
 * Listing a directory through the event file descriptor can make overlayfs
 * open its layers again: every layer of a merged directory, and the upper
 * directory of an impure one, which holds entries moved in from below. The
 * last two test cases have the listener do that before it answers.
 *
 * The test needs root to create permission events and to mount overlayfs.
 */

#define _GNU_SOURCE
#include "config.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/xattr.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define BACKING_MNT	OVL_BASE_MNTPOINT
#define FNAME		"test_file"
#define DNAME		"test_dir"
#define MNAME		"merged_dir"
#define INAME		"impure_dir"
#define MOVED		"moved_file"
#define OVL_FILE	OVL_MNT "/" FNAME
#define OVL_DIR		OVL_MNT "/" DNAME
#define OVL_MDIR	OVL_MNT "/" MNAME
#define OVL_IDIR	OVL_MNT "/" INAME
#define OVL_MOVED	OVL_MNT "/" MOVED
#define OVL_IDIR_MOVED	OVL_IDIR "/" MOVED
#define LOWER_FILE	OVL_LOWER "/" FNAME
#define LOWER_DIR	OVL_LOWER "/" DNAME
#define LOWER_MDIR	OVL_LOWER "/" MNAME
#define LOWER_MOVED	OVL_LOWER "/" MOVED
#define UPPER_MDIR	OVL_UPPER "/" MNAME
#define UPPER_IDIR	OVL_UPPER "/" INAME

/*
 * The listener either answers the event at once or is blocked for good, so
 * this only has to outlast scheduling noise. Scaled through
 * tst_multiply_timeout() at the call sites, or a slow kernel build would
 * report a healthy but unscheduled listener as blocked.
 */
#define WAIT_MSEC	5000

static int fd_notify = -1;
static int idir_is_impure;

static struct tcase {
	const char *tname;
	const char *path;
	int mark_backing;
	int ondir;
	int list;
	int impure;
} tcases[] = {
	{"Overlay mark, file", OVL_FILE, 0, 0, 0, 0},
	{"Overlay and backing mark, file", OVL_FILE, 1, 0, 0, 0},
	{"Overlay mark, directory", OVL_DIR, 0, 1, 0, 0},
	{"Overlay and backing mark, directory", OVL_DIR, 1, 1, 0, 0},
	{"Overlay and backing mark, merged directory, listed",
	 OVL_MDIR, 1, 1, 1, 0},
	{"Overlay and backing mark, impure directory, listed",
	 OVL_IDIR, 1, 1, 1, 1},
};

static void list_dir(int fd)
{
	char buf[1024];
	long ret;

	/* Blocks for good if reading a layer generates a nested event. */
	do {
		ret = tst_syscall(__NR_getdents64, fd, buf, sizeof(buf));
	} while (ret > 0);

	if (ret == -1)
		tst_brk(TBROK | TERRNO, "getdents64() on the event fd failed");
}

static void answer_one_event(pid_t opener, int list)
{
	struct fanotify_event_metadata event;
	struct fanotify_response resp;

	/* Blocks for good if the event fd generates a nested event. */
	SAFE_READ(SAFE_READ_ALL, fd_notify, &event, sizeof(event));

	if (list && event.fd != FAN_NOFD)
		list_dir(event.fd);

	/*
	 * A wrong mask or a stranger's event is a result, not harness
	 * breakage, so report and still answer: the opener stays parked in
	 * the kernel until something does. FAN_ONDIR is not reported to a
	 * group without FAN_REPORT_FID, so a directory open arrives as a
	 * plain FAN_OPEN_PERM as well.
	 */
	if (event.mask != FAN_OPEN_PERM) {
		tst_res(TFAIL, "Got event mask %llx, expected %llx",
			(unsigned long long)event.mask,
			(unsigned long long)FAN_OPEN_PERM);
	} else if (event.pid != opener) {
		tst_res(TFAIL, "Got event from pid %u, expected %u",
			(unsigned int)event.pid, (unsigned int)opener);
	} else {
		tst_res(TPASS, "Listener answered the event");
	}

	resp.fd = event.fd;
	resp.response = FAN_ALLOW;
	SAFE_WRITE(SAFE_WRITE_ALL, fd_notify, &resp, sizeof(resp));

	if (event.fd != FAN_NOFD)
		SAFE_CLOSE(event.fd);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	uint64_t mask = FAN_OPEN_PERM | (tc->ondir ? FAN_ONDIR : 0);
	pid_t opener, reader;

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	if (tc->impure) {
		if (!idir_is_impure) {
			tst_res(TCONF, "Overlay did not mark the directory impure");
			return;
		}
		/*
		 * The impure listing is cached with the inode and reused, so
		 * a later iteration would not open the layer at all. Moving
		 * an entry with an origin out and back invalidates it.
		 */
		SAFE_RENAME(OVL_IDIR_MOVED, OVL_MOVED);
		SAFE_RENAME(OVL_MOVED, OVL_IDIR_MOVED);
	}

	fd_notify = SAFE_FANOTIFY_INIT(FAN_CLASS_CONTENT, O_RDONLY);

	SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
			   mask, AT_FDCWD, OVL_MNT);

	if (tc->mark_backing) {
		SAFE_FANOTIFY_MARK(fd_notify,
				   FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
				   mask, AT_FDCWD, BACKING_MNT);
	}

	opener = SAFE_FORK();
	if (!opener) {
		int fd;

		/* Must not hold the group, it has to be released on close. */
		SAFE_CLOSE(fd_notify);
		/*
		 * The kernel denies the event when a listener that never
		 * answered it goes away, so a failure here is the fallout of
		 * the test tearing a blocked listener down, not a result.
		 */
		fd = open(tc->path, O_RDONLY | (tc->ondir ? O_DIRECTORY : 0));
		if (fd != -1)
			SAFE_CLOSE(fd);
		exit(0);
	}

	/* The opener sleeps in the kernel once its event is queued. */
	if (TST_PROCESS_STATE_WAIT(opener, 'D',
				   tst_multiply_timeout(WAIT_MSEC)))
		tst_brk(TBROK, "Open generated no permission event");

	reader = SAFE_FORK();
	if (!reader) {
		answer_one_event(opener, tc->list);
		exit(0);
	}

	/*
	 * The listener answers and exits, or is blocked for good. Only the
	 * blocked case is handled here: the child reports its own success,
	 * so a child that died some other way cannot be taken for one that
	 * answered. The library reaps it on the normal path.
	 */
	if (TST_PROCESS_STATE_WAIT(reader, 'Z',
				   tst_multiply_timeout(WAIT_MSEC))) {
		tst_res(TFAIL, "Listener blocked by a nested event");
		SAFE_KILL(reader, SIGKILL);
		SAFE_WAITPID(reader, NULL, 0);
	}

	/*
	 * Releases whatever is still queued, so the opener can finish. The
	 * library reaps it and checks its exit status after this returns.
	 */
	SAFE_CLOSE(fd_notify);
}

static void setup(void)
{
	/* These have to exist in the layers to be opened through them. */
	SAFE_UMOUNT(OVL_MNT);
	SAFE_TOUCH(LOWER_FILE, 0644, NULL);
	SAFE_MKDIR(LOWER_DIR, 0755);
	SAFE_MKDIR(LOWER_MDIR, 0755);
	SAFE_MKDIR(UPPER_MDIR, 0755);
	SAFE_TOUCH(LOWER_MOVED, 0644, NULL);
	SAFE_MOUNT_OVERLAY();

	/*
	 * A directory made through the overlay is upper only. Moving a lower
	 * file into it records the file's origin and marks the directory
	 * impure, unless the layers cannot carry an origin.
	 */
	SAFE_MKDIR(OVL_IDIR, 0755);
	SAFE_RENAME(OVL_MOVED, OVL_IDIR_MOVED);
	idir_is_impure =
		getxattr(UPPER_IDIR, "trusted.overlay.impure", NULL, 0) != -1;

	/*
	 * Probe the widest combination run() uses, not a weaker proxy for
	 * it. Every case marks the overlay; the second of each pair also
	 * marks the base.
	 */
	REQUIRE_FANOTIFY_EVENTS_SUPPORTED_ON_FS(FAN_CLASS_CONTENT,
						FAN_MARK_FILESYSTEM,
						FAN_OPEN_PERM | FAN_ONDIR,
						OVL_MNT);
	REQUIRE_MARK_TYPE_SUPPORTED_ON_FS(FAN_MARK_FILESYSTEM, BACKING_MNT);
}

static void cleanup(void)
{
	if (fd_notify != -1)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_root = 1,
	.mount_device = 1,
	.needs_overlay = 1,
	.mntpoint = OVL_BASE_MNTPOINT,
	/* Test requires a base filesystem that supports RENAME_WHITEOUT */
	.filesystems = (struct tst_fs[]) {
		{.type = "ext4"},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "c3d355f427cea70fbe31b49e21f1915dbf37f2e6"},
		{}
	}
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
