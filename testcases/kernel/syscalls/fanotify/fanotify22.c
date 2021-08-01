// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Collabora Ltd.
 *
 * Author: Gabriel Krisman Bertazi <gabriel@krisman.be>
 * Based on previous work by Amir Goldstein <amir73il@gmail.com>
 */

/*\
 * [Description]
 * Check fanotify FAN_ERROR_FS events triggered by intentionally
 * corrupted filesystems:
 *
 * - Generate a broken filesystem
 * - Start FAN_FS_ERROR monitoring group
 * - Make the file system notice the error through ordinary operations
 * - Observe the event generated
 */

#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include "tst_test.h"
#include <sys/fanotify.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define BUF_SIZE 256
static char event_buf[BUF_SIZE];
int fd_notify;

#define MOUNT_PATH "test_mnt"

static void trigger_fs_abort(void)
{
	SAFE_MOUNT(tst_device->dev, MOUNT_PATH, tst_device->fs_type,
		   MS_REMOUNT|MS_RDONLY, "abort");
}

static struct test_case {
	char *name;
	int error;
	unsigned int error_count;
	void (*trigger_error)(void);
} testcases[] = {
	{
		.name = "Trigger abort",
		.trigger_error = &trigger_fs_abort,
		.error_count = 1,
		.error = ESHUTDOWN,
	},
};

int check_error_event_info_error(struct fanotify_event_info_error *info_error,
				 const struct test_case *ex)
{
	int fail = 0;

	if (info_error->error_count != ex->error_count) {
		tst_res(TFAIL, "%s: Unexpected error_count (%d!=%d)",
			ex->name, info_error->error_count, ex->error_count);
		fail++;
	}

	if (info_error->error != ex->error) {
		tst_res(TFAIL, "%s: Unexpected error code value (%d!=%d)",
			ex->name, info_error->error, ex->error);
		fail++;
	}

	return fail;
}

int check_error_event_metadata(struct fanotify_event_metadata *event)
{
	int fail = 0;

	if (event->mask != FAN_FS_ERROR) {
		fail++;
		tst_res(TFAIL, "got unexpected event %llx",
			(unsigned long long)event->mask);
	}

	if (event->fd != FAN_NOFD) {
		fail++;
		tst_res(TFAIL, "Weird FAN_FD %llx",
			(unsigned long long)event->mask);
	}
	return fail;
}

void check_event(char *buf, size_t len, const struct test_case *ex)
{
	struct fanotify_event_metadata *event =
		(struct fanotify_event_metadata *) buf;
	struct fanotify_event_info_error *info_error;
	int fail = 0;

	if (len < FAN_EVENT_METADATA_LEN) {
		tst_res(TFAIL, "No event metadata found");
		return;
	}

	if (check_error_event_metadata(event))
		return;

	info_error = get_event_info_error(event);
	if (info_error)
		fail += check_error_event_info_error(info_error, ex);
	else {
		tst_res(TFAIL, "Generic error record not found");
		fail++;
	}

	if (!fail)
		tst_res(TPASS, "Successfully received: %s", ex->name);
}

static void do_test(unsigned int i)
{
	const struct test_case *tcase = &testcases[i];
	size_t read_len;

	SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD|FAN_MARK_FILESYSTEM,
			   FAN_FS_ERROR, AT_FDCWD, MOUNT_PATH);

	tcase->trigger_error();

	read_len = SAFE_READ(0, fd_notify, event_buf, BUF_SIZE);

	SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_REMOVE|FAN_MARK_FILESYSTEM,
			   FAN_FS_ERROR, AT_FDCWD, MOUNT_PATH);

	check_event(event_buf, read_len, tcase);
}

static void setup(void)
{
	REQUIRE_FANOTIFY_EVENTS_SUPPORTED_ON_FS(FAN_CLASS_NOTIF|FAN_REPORT_FID,
						FAN_MARK_FILESYSTEM,
						FAN_FS_ERROR, ".");

	fd_notify = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF|FAN_REPORT_FID,
				       O_RDONLY);
}

static void cleanup(void)
{
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(testcases),
	.setup = setup,
	.cleanup = cleanup,
	.mount_device = 1,
	.mntpoint = MOUNT_PATH,
	.needs_root = 1,
	.dev_fs_type = "ext4"
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
