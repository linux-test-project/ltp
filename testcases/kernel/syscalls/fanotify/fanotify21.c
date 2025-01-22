// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Google. All Rights Reserved.
 *
 * Started by Matthew Bobrowski <repnop@google.com>
 */

/*\
 * [Description]
 *
 * A test which verifies whether the returned struct
 * fanotify_event_info_pidfd in FAN_REPORT_PIDFD mode contains the
 * expected set of information.
 *
 * NOTE: FAN_REPORT_PIDFD support was added in v5.15-rc1 in af579beb666a
 * ("fanotify: add pidfd support to the fanotify API").
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_safe_macros.h"
#include "lapi/pidfd.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define BUF_SZ		4096
#define MOUNT_PATH	"fs_mnt"
#define TEST_FILE	MOUNT_PATH "/testfile"

struct pidfd_fdinfo_t {
	int pos;
	int flags;
	int mnt_id;
	int pid;
	int ns_pid;
};

static struct test_case_t {
	char *name;
	int fork;
	int want_pidfd_err;
	int remount_ro;
} test_cases[] = {
	{
		"return a valid pidfd for event created by self",
		0,
		0,
		0,
	},
	{
		"return invalid pidfd for event created by terminated child",
		1,
		1,
		0,
	},
	{
		"fail to open rw fd for event created on read-only mount",
		0,
		0,
		1,
	},
};

static int fanotify_fd;
static char event_buf[BUF_SZ];
static struct pidfd_fdinfo_t *self_pidfd_fdinfo;

static int fd_error_unsupported;

static struct pidfd_fdinfo_t *read_pidfd_fdinfo(int pidfd)
{
	char *fdinfo_path;
	struct pidfd_fdinfo_t *pidfd_fdinfo;

	pidfd_fdinfo = SAFE_MALLOC(sizeof(struct pidfd_fdinfo_t));

	SAFE_ASPRINTF(&fdinfo_path, "/proc/self/fdinfo/%d", pidfd);
	SAFE_FILE_LINES_SCANF(fdinfo_path, "pos: %d", &pidfd_fdinfo->pos);
	SAFE_FILE_LINES_SCANF(fdinfo_path, "flags: %d", &pidfd_fdinfo->flags);
	SAFE_FILE_LINES_SCANF(fdinfo_path, "mnt_id: %d", &pidfd_fdinfo->mnt_id);
	SAFE_FILE_LINES_SCANF(fdinfo_path, "Pid: %d", &pidfd_fdinfo->pid);
	SAFE_FILE_LINES_SCANF(fdinfo_path, "NSpid: %d", &pidfd_fdinfo->ns_pid);

	free(fdinfo_path);

	return pidfd_fdinfo;
}

static void generate_event(void)
{
	int fd;

	/* Generate a single FAN_OPEN event on the watched object. */
	fd = SAFE_OPEN(TEST_FILE, O_RDONLY);
	SAFE_CLOSE(fd);
}

static void do_fork(void)
{
	int status;
	pid_t child;

	child = SAFE_FORK();
	if (child == 0) {
		SAFE_CLOSE(fanotify_fd);
		generate_event();
		exit(EXIT_SUCCESS);
	}

	SAFE_WAITPID(child, &status, 0);
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		tst_brk(TBROK,
			"child process terminated incorrectly");
}

static void do_setup(void)
{
	int pidfd;
	int init_flags = FAN_REPORT_PIDFD;

	if (tst_variant) {
		fanotify_fd = -1;
		fd_error_unsupported = fanotify_init_flags_supported_on_fs(FAN_REPORT_FD_ERROR, ".");
		if (fd_error_unsupported)
			return;
		init_flags |= FAN_REPORT_FD_ERROR;
	}

	SAFE_TOUCH(TEST_FILE, 0666, NULL);

	/*
	 * An explicit check for FAN_REPORT_PIDFD is performed early
	 * on in the test initialization as it's a prerequisite for
	 * all test cases.
	 */
	REQUIRE_FANOTIFY_INIT_FLAGS_SUPPORTED_ON_FS(FAN_REPORT_PIDFD,
						    TEST_FILE);

	fanotify_fd = SAFE_FANOTIFY_INIT(init_flags, O_RDWR);
	SAFE_FANOTIFY_MARK(fanotify_fd, FAN_MARK_ADD, FAN_OPEN, AT_FDCWD,
			   TEST_FILE);

	pidfd = SAFE_PIDFD_OPEN(getpid(), 0);

	self_pidfd_fdinfo = read_pidfd_fdinfo(pidfd);
	if (self_pidfd_fdinfo == NULL) {
		tst_brk(TBROK,
			"pidfd=%d, failed to read pidfd fdinfo",
			pidfd);
	}
}

static void do_test(unsigned int num)
{
	int i = 0, len;
	struct test_case_t *tc = &test_cases[num];
	int nopidfd_err = tc->want_pidfd_err ?
			  (tst_variant ? -ESRCH : FAN_NOPIDFD) : 0;
	int fd_err = (tc->remount_ro && tst_variant) ? -EROFS : 0;

	tst_res(TINFO, "Test #%d.%d: %s %s", num, tst_variant, tc->name,
			tst_variant ? "(FAN_REPORT_FD_ERROR)" : "");

	if (fd_error_unsupported && tst_variant) {
		FANOTIFY_INIT_FLAGS_ERR_MSG(FAN_REPORT_FD_ERROR, fd_error_unsupported);
		return;
	}

	if (tc->remount_ro) {
		/* SAFE_MOUNT fails to remount FUSE */
		if (mount(tst_device->dev, MOUNT_PATH, tst_device->fs_type,
			  MS_REMOUNT|MS_RDONLY, NULL) != 0) {
			tst_brk(TFAIL,
				"filesystem %s failed to remount readonly",
				tst_device->fs_type);
		}
	}

	/*
	 * Generate the event in either self or a child process. Event
	 * generation in a child process is done so that the FAN_NOPIDFD case
	 * can be verified.
	 */
	if (tc->fork)
		do_fork();
	else
		generate_event();

	/*
	 * Read all of the queued events into the provided event
	 * buffer.
	 */
	len = read(fanotify_fd, event_buf, sizeof(event_buf));
	if (len < 0) {
		if (tc->remount_ro && !fd_err && errno == EROFS) {
			tst_res(TPASS, "cannot read event with rw fd from a ro fs");
			return;
		}
		tst_brk(TBROK | TERRNO, "reading fanotify events failed");
	} else if (tc->remount_ro && !fd_err) {
		tst_res(TFAIL, "got unexpected event with rw fd from a ro fs");
	}
	while (i < len) {
		struct fanotify_event_metadata *event;
		struct fanotify_event_info_pidfd *info;
		struct pidfd_fdinfo_t *event_pidfd_fdinfo = NULL;

		event = (struct fanotify_event_metadata *)&event_buf[i];
		info = (struct fanotify_event_info_pidfd *)(event + 1);

		/*
		 * Checks ensuring that pidfd information record object header
		 * fields are set correctly.
		 */
		if (info->hdr.info_type != FAN_EVENT_INFO_TYPE_PIDFD) {
			tst_res(TFAIL,
				"unexpected info_type received in info "
				"header (expected: %d, got: %d",
				FAN_EVENT_INFO_TYPE_PIDFD,
				info->hdr.info_type);
			info = NULL;
			goto next_event;
		} else if (info->hdr.len !=
			   sizeof(struct fanotify_event_info_pidfd)) {
			tst_res(TFAIL,
				"unexpected info object length "
				"(expected: %lu, got: %d",
				sizeof(struct fanotify_event_info_pidfd),
				info->hdr.len);
			info = NULL;
			goto next_event;
		}

		/*
		 * Check if event->fd reported any errors during
		 * creation and whether they're expected.
		 */
		if (!fd_err && event->fd >= 0) {
			tst_res(TPASS,
				"event->fd %d is valid as expected",
				event->fd);
		} else if (fd_err && event->fd == fd_err) {
			tst_res(TPASS,
				"event->fd is error %d as expected",
				event->fd);
		} else if (fd_err) {
			tst_res(TFAIL,
				"event->fd is %d, but expected error %d",
				event->fd, fd_err);
		} else {
			tst_res(TFAIL,
				"event->fd creation failed with error %d",
				event->fd);
		}

		/*
		 * Check if pidfd information object reported any errors during
		 * creation and whether they're expected.
		 */
		if (info->pidfd < 0 && !tc->want_pidfd_err) {
			tst_res(TFAIL,
				"pidfd creation failed for pid: %u with pidfd error value "
				"set to: %d",
				(unsigned int)event->pid,
				info->pidfd);
			goto next_event;
		} else if (tc->want_pidfd_err && info->pidfd != nopidfd_err) {
			tst_res(TFAIL,
				"pidfd set to an unexpected error: %d for pid: %u",
				info->pidfd,
				(unsigned int)event->pid);
			goto next_event;
		} else if (tc->want_pidfd_err && info->pidfd == nopidfd_err) {
			tst_res(TPASS,
				"pid: %u terminated before pidfd was created, "
				"pidfd set to the value of: %d, as expected",
				(unsigned int)event->pid,
				nopidfd_err);
			goto next_event;
		}

		/*
		 * No pidfd errors occurred, continue with verifying pidfd
		 * fdinfo validity.
		 */
		event_pidfd_fdinfo = read_pidfd_fdinfo(info->pidfd);
		if (event_pidfd_fdinfo == NULL) {
			tst_brk(TBROK,
				"reading fdinfo for pidfd: %d "
				"describing pid: %u failed",
				info->pidfd,
				(unsigned int)event->pid);
			goto next_event;
		} else if (event_pidfd_fdinfo->pid != event->pid) {
			tst_res(TFAIL,
				"pidfd provided for incorrect pid "
				"(expected pidfd for pid: %u, got pidfd for "
				"pid: %u)",
				(unsigned int)event->pid,
				(unsigned int)event_pidfd_fdinfo->pid);
			goto next_event;
		} else if (memcmp(event_pidfd_fdinfo, self_pidfd_fdinfo,
				  sizeof(struct pidfd_fdinfo_t))) {
			tst_res(TFAIL,
				"pidfd fdinfo values for self and event differ "
				"(expected pos: %d, flags: %x, mnt_id: %d, "
				"pid: %d, ns_pid: %d, got pos: %d, "
				"flags: %x, mnt_id: %d, pid: %d, ns_pid: %d",
				self_pidfd_fdinfo->pos,
				self_pidfd_fdinfo->flags,
				self_pidfd_fdinfo->mnt_id,
				self_pidfd_fdinfo->pid,
				self_pidfd_fdinfo->ns_pid,
				event_pidfd_fdinfo->pos,
				event_pidfd_fdinfo->flags,
				event_pidfd_fdinfo->mnt_id,
				event_pidfd_fdinfo->pid,
				event_pidfd_fdinfo->ns_pid);
			goto next_event;
		} else {
			tst_res(TPASS,
				"got an event with a valid pidfd info record, "
				"mask: %lld, pid: %u, fd: %d, "
				"pidfd: %d, info_type: %d, info_len: %d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid,
				event->fd,
				info->pidfd,
				info->hdr.info_type,
				info->hdr.len);
		}

next_event:
		i += event->event_len;
		if (event->fd >= 0)
			SAFE_CLOSE(event->fd);

		if (info && info->pidfd >= 0)
			SAFE_CLOSE(info->pidfd);

		if (event_pidfd_fdinfo)
			free(event_pidfd_fdinfo);
	}
}

static void do_cleanup(void)
{
	if (fanotify_fd >= 0)
		SAFE_CLOSE(fanotify_fd);

	if (self_pidfd_fdinfo)
		free(self_pidfd_fdinfo);
}

static struct tst_test test = {
	.setup = do_setup,
	.test = do_test,
	.tcnt = ARRAY_SIZE(test_cases),
	.test_variants = 2,
	.cleanup = do_cleanup,
	.all_filesystems = 1,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MOUNT_PATH,
	.forks_child = 1,
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif /* HAVE_SYS_FANOTIFY_H */
