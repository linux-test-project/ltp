// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Matthew Bobrowski. All Rights Reserved.
 *
 * Started by Matthew Bobrowski <mbobrowski@mbobrowski.org>
 */

/*\
 * [Description]
 * Validate that the values returned within an event when FAN_REPORT_FID is
 * specified matches those that are obtained via explicit invocation to system
 * calls statfs(2) and name_to_handle_at(2).
 */

/*
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
#include <sys/mount.h>
#include <errno.h>
#include <unistd.h>
#include "tst_test.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define PATH_LEN 128
#define BUF_SIZE 1024
#define DIR_ONE "dir_one"
#define FILE_ONE "file_one"
#define FILE_TWO "file_two"
#define MOUNT_PATH "tstmnt"
#define EVENT_MAX (ARRAY_SIZE(objects)+1)
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

static int ovl_mounted;
static int bind_mounted;
static int ovl_bind_mounted;
static int nofid_fd;
static int fanotify_fd;
static int at_handle_fid;
static int filesystem_mark_unsupported;
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

	for (i = 0; i < ARRAY_SIZE(objects); i++) {
		at_handle_fid |=
			fanotify_save_fid(objects[i].path, &objects[i].fid);
	}
}

static int setup_marks(unsigned int fd, struct test_case_t *tc)
{
	unsigned int i;
	struct fanotify_mark_type *mark = &tc->mark;

	for (i = 0; i < ARRAY_SIZE(objects); i++) {
		SAFE_FANOTIFY_MARK(fd, FAN_MARK_ADD | mark->flag, tc->mask,
				   AT_FDCWD, objects[i].path);

		/*
		 * Setup the expected mask for each generated event.
		 * No events are expected on directory without FAN_ONDIR.
		 */
		event_set[i].expected_mask = tc->mask;
		if (!objects[i].is_dir)
			event_set[i].expected_mask &= ~FAN_ONDIR;
		else if (!(event_set[i].expected_mask & FAN_ONDIR))
			event_set[i].expected_mask = 0;
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
		"Test #%d.%d: FAN_REPORT_FID with mark flag: %s",
		number, tst_variant, mark->name);

	if (tst_variant && !ovl_mounted) {
		tst_res(TCONF, "overlayfs not supported on %s", tst_device->fs_type);
		return;
	}

	if (filesystem_mark_unsupported && mark->flag != FAN_MARK_INODE) {
		FANOTIFY_MARK_FLAGS_ERR_MSG(mark, filesystem_mark_unsupported);
		return;
	}

	fanotify_fd = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF | FAN_REPORT_FID |
					 FAN_NONBLOCK, O_RDONLY);

	/*
	 * Place marks on a set of objects and setup the expected masks
	 * for each event that is expected to be generated.
	 */
	if (setup_marks(fanotify_fd, tc) != 0)
		goto out;

	/* Watching base fs - open files on overlayfs */
	if (tst_variant && !ovl_bind_mounted) {
		if (mark->flag & FAN_MARK_MOUNT) {
			tst_res(TCONF, "overlayfs base fs cannot be watched with mount mark");
			goto out;
		}
		SAFE_MOUNT(OVL_MNT, MOUNT_PATH, "none", MS_BIND, NULL);
	}

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

	if (tst_variant && !ovl_bind_mounted)
		SAFE_UMOUNT(MOUNT_PATH);

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

	/*
	 * Verify that we did not get an extra event, for example, that we did
	 * not get an event on directory without FAN_ONDIR.
	 */
	if (event_set[i].expected_mask) {
		tst_res(TFAIL,
			"Did not get an expected event (expected: %llx)",
			event_set[i].expected_mask);
	}
out:
	SAFE_CLOSE(fanotify_fd);
}

static void do_setup(void)
{
	const char *mnt;

	/*
	 * Bind mount to either base fs or to overlayfs over base fs:
	 * Variant #0: watch base fs - open files on base fs
	 * Variant #1: watch lower fs - open lower files on overlayfs
	 * Variant #2: watch upper fs - open upper files on overlayfs
	 * Variant #3: watch overlayfs - open lower files on overlayfs
	 * Variant #4: watch overlayfs - open upper files on overlayfs
	 *
	 * Variants 1,2 test a bug whose fix bc2473c90fca ("ovl: enable fsnotify
	 * events on underlying real files") in kernel 6.5 is not likely to be
	 * backported to older kernels.
	 * To avoid waiting for events that won't arrive when testing old kernels,
	 * require that kernel supports encoding fid with new flag AT_HANDLE_FID,
	 * also merged to 6.5 and not likely to be backported to older kernels.
	 * Variants 3,4 test overlayfs watch with FAN_REPORT_FID, which also
	 * requires kernel with support for AT_HANDLE_FID.
	 */
	if (tst_variant) {
		REQUIRE_HANDLE_TYPE_SUPPORTED_BY_KERNEL(AT_HANDLE_FID);
		ovl_mounted = TST_MOUNT_OVERLAY();
		if (!ovl_mounted)
			return;

		mnt = tst_variant & 1 ? OVL_LOWER : OVL_UPPER;
	} else {
		mnt = OVL_BASE_MNTPOINT;

	}
	REQUIRE_FANOTIFY_INIT_FLAGS_SUPPORTED_ON_FS(FAN_REPORT_FID, mnt);
	SAFE_MKDIR(MOUNT_PATH, 0755);
	SAFE_MOUNT(mnt, MOUNT_PATH, "none", MS_BIND, NULL);
	bind_mounted = 1;

	nofid_fd = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF, O_RDONLY);

	/* Create file and directory objects for testing on base fs */
	create_objects();

	if (tst_variant > 2) {
		/* Setup watches on overlayfs */
		SAFE_MOUNT(OVL_MNT, MOUNT_PATH, "none", MS_BIND, NULL);
		ovl_bind_mounted = 1;
	}

	filesystem_mark_unsupported =
		fanotify_flags_supported_on_fs(FAN_REPORT_FID, FAN_MARK_FILESYSTEM, FAN_OPEN,
					       ovl_bind_mounted ? OVL_MNT : MOUNT_PATH);

	/*
	 * Create a mark on first inode without FAN_REPORT_FID, to test
	 * uninitialized connector->fsid cache. This mark remains for all test
	 * cases and is not expected to get any events (no writes in this test).
	 */
	SAFE_FANOTIFY_MARK(nofid_fd, FAN_MARK_ADD, FAN_CLOSE_WRITE, AT_FDCWD,
			  FILE_PATH_ONE);

	/* Get the filesystem fsid and file handle for each created object */
	get_object_stats();
}

static void do_cleanup(void)
{
	if (nofid_fd > 0)
		SAFE_CLOSE(nofid_fd);
	if (fanotify_fd > 0)
		SAFE_CLOSE(fanotify_fd);
	if (ovl_bind_mounted)
		SAFE_UMOUNT(MOUNT_PATH);
	if (bind_mounted) {
		SAFE_UMOUNT(MOUNT_PATH);
		SAFE_RMDIR(MOUNT_PATH);
	}
	if (ovl_mounted)
		SAFE_UMOUNT(OVL_MNT);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(test_cases),
	.test_variants = 5,
	.setup = do_setup,
	.cleanup = do_cleanup,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = OVL_BASE_MNTPOINT,
	.all_filesystems = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "c285a2f01d69"},
		{"linux-git", "bc2473c90fca"},
		{}
	}
};

#else
	TST_TEST_TCONF("System does not have required name_to_handle_at() support");
#endif
#else
	TST_TEST_TCONF("System does not have required fanotify support");
#endif
