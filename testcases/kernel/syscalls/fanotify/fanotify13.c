// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Matthew Bobrowski. All Rights Reserved.
 *
 * Started by Matthew Bobrowski <mbobrowski@mbobrowski.org>
 *
 * DESCRIPTION
 *	Validate that the values returned within an event when
 *	FAN_REPORT_FID is specified matches those that are obtained via
 *	explicit invocation to system calls statfs(2) and
 *	name_to_handle_at(2).
 *
 * This is also regression test for:
 *     c285a2f01d69 ("fanotify: update connector fsid cache on add mark")
 */
#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "tst_test.h"
#include "fanotify.h"

#if defined(HAVE_SYS_FANOTIFY_H)
#include <sys/fanotify.h>

#define PATH_LEN 128
#define BUF_SIZE 256
#define DIR_ONE "dir_one"
#define FILE_ONE "file_one"
#define FILE_TWO "file_two"
#define MOUNT_PATH "mntpoint"
#define EVENT_MAX ARRAY_SIZE(objects)
#define DIR_PATH_ONE MOUNT_PATH"/"DIR_ONE
#define FILE_PATH_ONE MOUNT_PATH"/"FILE_ONE
#define FILE_PATH_TWO MOUNT_PATH"/"FILE_TWO

#if defined(HAVE_NAME_TO_HANDLE_AT)
struct event_t {
	unsigned long long expected_mask;
};

static struct object_t {
	const char *path;
	int is_dir;
	struct fanotify_fid_t fid;
} objects[] = {
	{FILE_PATH_ONE, 0, {}},
	{FILE_PATH_TWO, 0, {}},
	{DIR_PATH_ONE, 1, {}}
};

static struct test_case_t {
	struct fanotify_mark_type mark;
	unsigned long long mask;
} test_cases[] = {
	{
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_OPEN | FAN_CLOSE_NOWRITE
	},
	{
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_OPEN | FAN_CLOSE_NOWRITE | FAN_ONDIR
	},
	{
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_OPEN | FAN_CLOSE_NOWRITE
	},
	{
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_OPEN | FAN_CLOSE_NOWRITE | FAN_ONDIR
	},
	{
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_OPEN | FAN_CLOSE_NOWRITE
	},
	{
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_OPEN | FAN_CLOSE_NOWRITE | FAN_ONDIR
	}
};

static int nofid_fd;
static int fanotify_fd;
static char events_buf[BUF_SIZE];
static struct event_t event_set[EVENT_MAX];

static void create_objects(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(objects); i++) {
		if (objects[i].is_dir)
			SAFE_MKDIR(objects[i].path, 0755);
		else
			SAFE_FILE_PRINTF(objects[i].path, "0");
	}
}

static void get_object_stats(void)
{
	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(objects); i++)
		fanotify_save_fid(objects[i].path, &objects[i].fid);
}

static int setup_marks(unsigned int fd, struct test_case_t *tc)
{
	unsigned int i;
	struct fanotify_mark_type *mark = &tc->mark;

	for (i = 0; i < ARRAY_SIZE(objects); i++) {
		if (fanotify_mark(fd, FAN_MARK_ADD | mark->flag, tc->mask,
					AT_FDCWD, objects[i].path) == -1) {
			if (errno == EINVAL &&
				mark->flag & FAN_MARK_FILESYSTEM) {
				tst_res(TCONF,
					"FAN_MARK_FILESYSTEM not supported by "
					"kernel");
				return 1;
			} else if (errno == ENODEV &&
				   !objects[i].fid.fsid.val[0] &&
				   !objects[i].fid.fsid.val[1]) {
				tst_res(TCONF,
					"FAN_REPORT_FID not supported on "
					"filesystem type %s",
					tst_device->fs_type);
				return 1;
			}
			tst_brk(TBROK | TERRNO,
				"fanotify_mark(%d, FAN_MARK_ADD, FAN_OPEN, "
				"AT_FDCWD, %s) failed",
				fanotify_fd, objects[i].path);
		}

		/* Setup the expected mask for each generated event */
		event_set[i].expected_mask = tc->mask;
		if (!objects[i].is_dir)
			event_set[i].expected_mask &= ~FAN_ONDIR;
	}
	return 0;
}

static void do_test(unsigned int number)
{
	unsigned int i;
	int len, fds[ARRAY_SIZE(objects)];

	struct file_handle *event_file_handle;
	struct fanotify_event_metadata *metadata;
	struct fanotify_event_info_fid *event_fid;
	struct test_case_t *tc = &test_cases[number];
	struct fanotify_mark_type *mark = &tc->mark;

	tst_res(TINFO,
		"Test #%d: FAN_REPORT_FID with mark flag: %s",
		number, mark->name);

	fanotify_fd = fanotify_init(FAN_CLASS_NOTIF | FAN_REPORT_FID, O_RDONLY);
	if (fanotify_fd == -1) {
		if (errno == EINVAL) {
			tst_res(TCONF,
				"FAN_REPORT_FID not supported by kernel");
			return;
		}
		tst_brk(TBROK | TERRNO,
			"fanotify_init(FAN_CLASS_NOTIF | FAN_REPORT_FID, "
			"O_RDONLY) failed");
	}

	/*
	 * Place marks on a set of objects and setup the expected masks
	 * for each event that is expected to be generated.
	 */
	if (setup_marks(fanotify_fd, tc) != 0)
		goto out;

	/* Generate sequence of FAN_OPEN events on objects */
	for (i = 0; i < ARRAY_SIZE(objects); i++)
		fds[i] = SAFE_OPEN(objects[i].path, O_RDONLY);

	/*
	 * Generate sequence of FAN_CLOSE_NOWRITE events on objects. Each
	 * FAN_CLOSE_NOWRITE event is expected to be merged with its
	 * respective FAN_OPEN event that was performed on the same object.
	 */
	for (i = 0; i < ARRAY_SIZE(objects); i++) {
		if (fds[i] > 0)
			SAFE_CLOSE(fds[i]);
	}

	/* Read events from event queue */
	len = SAFE_READ(0, fanotify_fd, events_buf, BUF_SIZE);

	/* Iterate over event queue */
	for (i = 0, metadata = (struct fanotify_event_metadata *) events_buf;
		FAN_EVENT_OK(metadata, len);
		metadata = FAN_EVENT_NEXT(metadata, len), i++) {
		struct fanotify_fid_t *expected_fid = &objects[i].fid;
		event_fid = (struct fanotify_event_info_fid *) (metadata + 1);
		event_file_handle = (struct file_handle *) event_fid->handle;

		/* File descriptor is redundant with FAN_REPORT_FID */
		if (metadata->fd != FAN_NOFD)
			tst_res(TFAIL,
				"Unexpectedly received file descriptor %d in "
				"event. Expected to get FAN_NOFD(%d)",
				metadata->fd, FAN_NOFD);

		/* Ensure that the correct mask has been reported in event */
		if (metadata->mask != event_set[i].expected_mask)
			tst_res(TFAIL,
				"Unexpected mask received: %llx (expected: "
				"%llx) in event",
				metadata->mask,
				event_set[i].expected_mask);

		/* Verify handle_bytes returned in event */
		if (event_file_handle->handle_bytes !=
		    expected_fid->handle.handle_bytes) {
			tst_res(TFAIL,
				"handle_bytes (%x) returned in event does not "
				"equal to handle_bytes (%x) returned in "
				"name_to_handle_at(2)",
				event_file_handle->handle_bytes,
				expected_fid->handle.handle_bytes);
			continue;
		}

		/* Verify handle_type returned in event */
		if (event_file_handle->handle_type !=
		    expected_fid->handle.handle_type) {
			tst_res(TFAIL,
				"handle_type (%x) returned in event does not "
				"equal to handle_type (%x) returned in "
				"name_to_handle_at(2)",
				event_file_handle->handle_type,
				expected_fid->handle.handle_type);
			continue;
		}

		/* Verify file identifier f_handle returned in event */
		if (memcmp(event_file_handle->f_handle,
			   expected_fid->handle.f_handle,
			   expected_fid->handle.handle_bytes) != 0) {
			tst_res(TFAIL,
				"file_handle returned in event does not match "
				"the file_handle returned in "
				"name_to_handle_at(2)");
			continue;
		}

		/* Verify filesystem ID fsid  returned in event */
		if (memcmp(&event_fid->fsid, &expected_fid->fsid,
			   sizeof(expected_fid->fsid)) != 0) {
			tst_res(TFAIL,
				"event_fid.fsid != stat.f_fsid that was "
				"obtained via statfs(2)");
			continue;
		}

		tst_res(TPASS,
			"got event: mask=%llx, pid=%d, fid=%x.%x.%lx values "
			"returned in event match those returned in statfs(2) "
			"and name_to_handle_at(2)",
			metadata->mask,
			getpid(),
			FSID_VAL_MEMBER(event_fid->fsid, 0),
			FSID_VAL_MEMBER(event_fid->fsid, 1),
			*(unsigned long *) event_file_handle->f_handle);
	}
out:
	SAFE_CLOSE(fanotify_fd);
}

static void do_setup(void)
{
	/* Check for kernel fanotify support */
	nofid_fd = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF, O_RDONLY);

	/* Create file and directory objects for testing */
	create_objects();

	/*
	 * Create a mark on first inode without FAN_REPORT_FID, to test
	 * uninitialized connector->fsid cache. This mark remains for all test
	 * cases and is not expected to get any events (no writes in this test).
	 */
	if (fanotify_mark(nofid_fd, FAN_MARK_ADD, FAN_CLOSE_WRITE, AT_FDCWD,
			  FILE_PATH_ONE) == -1) {
		tst_brk(TBROK | TERRNO,
			"fanotify_mark(%d, FAN_MARK_ADD, FAN_CLOSE_WRITE, "
			"AT_FDCWD, "FILE_PATH_ONE") failed",
			nofid_fd);
	}

	/* Get the filesystem fsid and file handle for each created object */
	get_object_stats();
}

static void do_cleanup(void)
{
	SAFE_CLOSE(nofid_fd);
	if (fanotify_fd > 0)
		SAFE_CLOSE(fanotify_fd);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(test_cases),
	.setup = do_setup,
	.cleanup = do_cleanup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.mount_device = 1,
	.mntpoint = MOUNT_PATH,
	.all_filesystems = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "c285a2f01d69"},
		{}
	}
};

#else
	TST_TEST_TCONF("System does not have required name_to_handle_at() support");
#endif
#else
	TST_TEST_TCONF("System does not have required fanotify support");
#endif
