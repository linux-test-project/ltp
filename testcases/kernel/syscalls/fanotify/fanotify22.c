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

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#ifndef EFSCORRUPTED
#define EFSCORRUPTED    EUCLEAN         /* Filesystem is corrupted */
#endif

#define BUF_SIZE 256

#define MOUNT_PATH "test_mnt"
#define BASE_DIR "internal_dir"
#define BAD_DIR BASE_DIR"/bad_dir"

#ifdef HAVE_NAME_TO_HANDLE_AT

static char event_buf[BUF_SIZE];
static int fd_notify;

/* These expected FIDs are common to multiple tests */
static struct fanotify_fid_t null_fid;
static struct fanotify_fid_t bad_file_fid;

static void trigger_fs_abort(void)
{
	SAFE_MOUNT(tst_device->dev, MOUNT_PATH, tst_device->fs_type,
		   MS_REMOUNT|MS_RDONLY, "abort");
}

static void do_debugfs_request(const char *dev, char *request)
{
	const char *const cmd[] = {"debugfs", "-w", dev, "-R", request, NULL};

	SAFE_CMD(cmd, NULL, NULL);
}

static void tcase2_trigger_lookup(void)
{
	int ret;

	/* SAFE_OPEN cannot be used here because we expect it to fail. */
	ret = open(MOUNT_PATH"/"BAD_DIR, O_RDONLY, 0);
	if (ret != -1 && errno != EUCLEAN)
		tst_res(TFAIL, "Unexpected lookup result(%d) of %s (%d!=%d)",
			ret, BAD_DIR, errno, EUCLEAN);
}

static void tcase3_trigger(void)
{
	trigger_fs_abort();
	tcase2_trigger_lookup();
}

static void tcase4_trigger(void)
{
	tcase2_trigger_lookup();
	trigger_fs_abort();
}

static struct test_case {
	char *name;
	int error;
	unsigned int error_count;
	struct fanotify_fid_t *fid;
	void (*trigger_error)(void);
} testcases[] = {
	{
		.name = "Trigger abort",
		.trigger_error = &trigger_fs_abort,
		.error_count = 1,
		.error = ESHUTDOWN,
		.fid = &null_fid,
	},
	{
		.name = "Lookup of inode with invalid mode",
		.trigger_error = &tcase2_trigger_lookup,
		.error_count = 1,
		.error = EFSCORRUPTED,
		.fid = &bad_file_fid,
	},
	{
		.name = "Multiple error submission",
		.trigger_error = &tcase3_trigger,
		.error_count = 2,
		.error = ESHUTDOWN,
		.fid = &null_fid,
	},
	{
		.name = "Multiple error submission 2",
		.trigger_error = &tcase4_trigger,
		.error_count = 2,
		.error = EFSCORRUPTED,
		.fid = &bad_file_fid,
	}
};

static int check_error_event_info_fid(struct fanotify_event_info_fid *fid,
				 const struct test_case *ex)
{
	struct file_handle *fh = (struct file_handle *) &fid->handle;

	if (memcmp(&fid->fsid, &ex->fid->fsid, sizeof(fid->fsid))) {
		tst_res(TFAIL, "%s: Received bad FSID type (%x...!=%x...)",
			ex->name, FSID_VAL_MEMBER(fid->fsid, 0),
			ex->fid->fsid.val[0]);

		return 1;
	}
	if (fh->handle_type != ex->fid->handle.handle_type) {
		tst_res(TFAIL, "%s: Received bad file_handle type (%d!=%d)",
			ex->name, fh->handle_type, ex->fid->handle.handle_type);
		return 1;
	}

	if (fh->handle_bytes != ex->fid->handle.handle_bytes) {
		tst_res(TFAIL, "%s: Received bad file_handle len (%d!=%d)",
			ex->name, fh->handle_bytes, ex->fid->handle.handle_bytes);
		return 1;
	}

	if (memcmp(fh->f_handle, ex->fid->handle.f_handle, fh->handle_bytes)) {
		tst_res(TFAIL, "%s: Received wrong handle. "
			"Expected (%x...) got (%x...) ", ex->name,
			*(int *)ex->fid->handle.f_handle, *(int *)fh->f_handle);
		return 1;
	}
	return 0;
}

static int check_error_event_info_error(struct fanotify_event_info_error *info_error,
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

static int check_error_event_metadata(struct fanotify_event_metadata *event)
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

static void check_event(char *buf, size_t len, const struct test_case *ex)
{
	struct fanotify_event_metadata *event =
		(struct fanotify_event_metadata *) buf;
	struct fanotify_event_info_error *info_error;
	struct fanotify_event_info_fid *info_fid;
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

	info_fid = get_event_info_fid(event);
	if (info_fid)
		fail += check_error_event_info_fid(info_fid, ex);
	else {
		tst_res(TFAIL, "FID record not found");
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

static void pre_corrupt_fs(void)
{
	SAFE_MKDIR(MOUNT_PATH"/"BASE_DIR, 0777);
	SAFE_MKDIR(MOUNT_PATH"/"BAD_DIR, 0777);

	fanotify_save_fid(MOUNT_PATH"/"BAD_DIR, &bad_file_fid);

	SAFE_UMOUNT(MOUNT_PATH);
	do_debugfs_request(tst_device->dev, "sif " BAD_DIR " mode 0xff");
	SAFE_MOUNT(tst_device->dev, MOUNT_PATH, tst_device->fs_type, 0, NULL);
}

static void init_null_fid(void)
{
	/* Use fanotify_save_fid to fill the fsid and overwrite the
	 * file_handler to create a null_fid
	 */
	fanotify_save_fid(MOUNT_PATH, &null_fid);

	null_fid.handle.handle_type = FILEID_INVALID;
	null_fid.handle.handle_bytes = 0;
}

static void setup(void)
{
	REQUIRE_FANOTIFY_EVENTS_SUPPORTED_ON_FS(FAN_CLASS_NOTIF|FAN_REPORT_FID,
						FAN_MARK_FILESYSTEM,
						FAN_FS_ERROR, ".");
	pre_corrupt_fs();

	fd_notify = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF|FAN_REPORT_FID,
				       O_RDONLY);

	init_null_fid();
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
	.dev_fs_type = "ext4",
	.tags = (const struct tst_tag[]) {
		{"linux-git", "124e7c61deb2"},
		{}
	},
	.timeout = 10,
	.needs_cmds = (const char *[]) {
		"debugfs",
		NULL
	}
};

#else
	TST_TEST_TCONF("system does not have required name_to_handle_at() support");
#endif
#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
