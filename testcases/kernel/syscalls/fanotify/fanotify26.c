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
 * The first test case marks the overlay only and must pass on any kernel. The
 * second one marks the backing filesystem as well, which is the shape an
 * anti-malware scanner uses on an overlayfs root.
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
#include "tst_test.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define BACKING_MNT	OVL_BASE_MNTPOINT
#define FNAME		"test_file"
#define OVL_FILE	OVL_MNT "/" FNAME
#define LOWER_FILE	OVL_LOWER "/" FNAME

/*
 * The listener either answers the event at once or is blocked for good, so
 * this only has to outlast scheduling noise. Scaled through
 * tst_multiply_timeout() at the call sites, or a slow kernel build would
 * report a healthy but unscheduled listener as blocked.
 */
#define WAIT_MSEC	5000

static int fd_notify = -1;

static struct tcase {
	const char *tname;
	int mark_backing;
} tcases[] = {
	{"Mark on the overlay", 0},
	{"Mark on the overlay and on the backing filesystem", 1},
};

static void answer_one_event(pid_t opener)
{
	struct fanotify_event_metadata event;
	struct fanotify_response resp;

	/* Blocks for good if the event fd generates a nested event. */
	SAFE_READ(SAFE_READ_ALL, fd_notify, &event, sizeof(event));

	/*
	 * A wrong mask or a stranger's event is a result, not harness
	 * breakage, so report and still answer: the opener stays parked in
	 * the kernel until something does.
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
	pid_t opener, reader;

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	fd_notify = SAFE_FANOTIFY_INIT(FAN_CLASS_CONTENT, O_RDONLY);

	SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
			   FAN_OPEN_PERM, AT_FDCWD, OVL_MNT);

	if (tc->mark_backing) {
		SAFE_FANOTIFY_MARK(fd_notify,
				   FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
				   FAN_OPEN_PERM, AT_FDCWD, BACKING_MNT);
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
		fd = open(OVL_FILE, O_RDONLY);
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
		answer_one_event(opener);
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
		tst_res(TFAIL,
			"Listener blocked in read() by a nested event");
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
	/* The file has to exist in the lower layer to be opened through it. */
	SAFE_UMOUNT(OVL_MNT);
	SAFE_TOUCH(LOWER_FILE, 0644, NULL);
	SAFE_MOUNT_OVERLAY();

	/*
	 * Probe the combination run() uses, not a weaker proxy for it. Every
	 * case marks the overlay; the second also marks the base.
	 */
	REQUIRE_FANOTIFY_EVENTS_SUPPORTED_ON_FS(FAN_CLASS_CONTENT,
						FAN_MARK_FILESYSTEM,
						FAN_OPEN_PERM, OVL_MNT);
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
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
