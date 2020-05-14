// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 CTERA Networks. All Rights Reserved.
 *
 * Started by Amir Goldstein <amir73il@gmail.com>
 *
 * DESCRIPTION
 *     Check FAN_DIR_MODIFY events with name info
 */
#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include "tst_test.h"
#include "fanotify.h"

#if defined(HAVE_SYS_FANOTIFY_H)
#include <sys/fanotify.h>
#include <sys/inotify.h>

#define EVENT_MAX 10

/* Size of the event structure, not including file handle */
#define EVENT_SIZE (sizeof(struct fanotify_event_metadata) + \
		    sizeof(struct fanotify_event_info_fid))
/* Tripple events buffer size to account for file handles and names */
#define EVENT_BUF_LEN (EVENT_MAX * EVENT_SIZE * 3)


#define BUF_SIZE 256

#ifdef HAVE_NAME_TO_HANDLE_AT
struct event_t {
	unsigned long long mask;
	struct fanotify_fid_t *fid;
	char name[BUF_SIZE];
};

static char fname1[BUF_SIZE + 11], fname2[BUF_SIZE + 11];
static char dname1[BUF_SIZE], dname2[BUF_SIZE];
static int fd_notify;

static struct event_t event_set[EVENT_MAX];

static char event_buf[EVENT_BUF_LEN];

#define DIR_NAME1 "test_dir1"
#define DIR_NAME2 "test_dir2"
#define FILE_NAME1 "test_file1"
#define FILE_NAME2 "test_file2"
#define MOUNT_PATH "fs_mnt"

static struct test_case_t {
	const char *tname;
	struct fanotify_mark_type mark;
	unsigned long mask;
	struct fanotify_mark_type sub_mark;
	unsigned long sub_mask;
} test_cases[] = {
	{
		/* Filesystem watch for dir modify and delete self events */
		"FAN_REPORT_FID on filesystem with FAN_DIR_MODIFY",
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_DIR_MODIFY | FAN_DELETE_SELF | FAN_ONDIR,
		{},
		0,
	},
	{
		/* Recursive watches for dir modify events */
		"FAN_REPORT_FID on directories with FAN_DIR_MODIFY",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_DIR_MODIFY,
		/* Watches for delete self event on subdir */
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_DIR_MODIFY | FAN_DELETE_SELF | FAN_ONDIR,
	},
};

static void do_test(unsigned int number)
{
	int fd, len = 0, i = 0, test_num = 0, tst_count = 0;
	struct test_case_t *tc = &test_cases[number];
	struct fanotify_mark_type *mark = &tc->mark;
	struct fanotify_mark_type *sub_mark = &tc->sub_mark;
	struct fanotify_fid_t root_fid, dir_fid, file_fid;

	tst_res(TINFO, "Test #%d: %s", number, tc->tname);

	fd_notify = fanotify_init(FAN_REPORT_FID, 0);
	if (fd_notify == -1) {
		if (errno == EINVAL)
			tst_brk(TCONF,
				"FAN_REPORT_FID not supported by kernel");

		tst_brk(TBROK | TERRNO,
			"fanotify_init(FAN_REPORT_FID, 0) failed");
	}

	/*
	 * Watch dir modify events with name in filesystem/dir
	 */
	if (fanotify_mark(fd_notify, FAN_MARK_ADD | mark->flag, tc->mask,
			  AT_FDCWD, MOUNT_PATH) < 0) {
		if (errno == EINVAL)
			tst_brk(TCONF,
				"FAN_DIR_MODIFY not supported by kernel");

		tst_brk(TBROK | TERRNO,
		    "fanotify_mark (%d, FAN_MARK_ADD | %s, 0x%lx, "
		    "AT_FDCWD, '"MOUNT_PATH"') failed",
		    fd_notify, mark->name, tc->mask);
	}

	/* Save the mount root fid */
	fanotify_save_fid(MOUNT_PATH, &root_fid);

	/*
	 * Create subdir and watch open events "on children" with name.
	 */
	SAFE_MKDIR(dname1, 0755);

	/* Save the subdir fid */
	fanotify_save_fid(dname1, &dir_fid);

	if (tc->sub_mask &&
	    fanotify_mark(fd_notify, FAN_MARK_ADD | sub_mark->flag, tc->sub_mask,
			  AT_FDCWD, dname1) < 0) {
		tst_brk(TBROK | TERRNO,
		    "fanotify_mark (%d, FAN_MARK_ADD | %s, 0x%lx, "
		    "AT_FDCWD, '%s') failed",
		    fd_notify, sub_mark->name, tc->sub_mask, dname1);
	}

	event_set[tst_count].mask = FAN_DIR_MODIFY;
	event_set[tst_count].fid = &root_fid;
	strcpy(event_set[tst_count].name, DIR_NAME1);
	tst_count++;

	/* Generate modify events "on child" */
	fd = SAFE_CREAT(fname1, 0755);

	/* Save the file fid */
	fanotify_save_fid(fname1, &file_fid);

	SAFE_WRITE(1, fd, "1", 1);
	SAFE_RENAME(fname1, fname2);
	SAFE_CLOSE(fd);

	/* Generate delete events with fname2 */
	SAFE_UNLINK(fname2);

	/* Read events on files in subdir */
	len += SAFE_READ(0, fd_notify, event_buf + len, EVENT_BUF_LEN - len);

	/*
	 * FAN_DIR_MODIFY events with the same name are merged.
	 */
	event_set[tst_count].mask = FAN_DIR_MODIFY;
	event_set[tst_count].fid = &dir_fid;
	strcpy(event_set[tst_count].name, FILE_NAME1);
	tst_count++;
	event_set[tst_count].mask = FAN_DIR_MODIFY;
	event_set[tst_count].fid = &dir_fid;
	strcpy(event_set[tst_count].name, FILE_NAME2);
	tst_count++;

	/*
	 * Directory watch does not get self events on children.
	 * Filesystem watch gets self event w/o name info.
	 */
	if (mark->flag == FAN_MARK_FILESYSTEM) {
		event_set[tst_count].mask = FAN_DELETE_SELF;
		event_set[tst_count].fid = &file_fid;
		strcpy(event_set[tst_count].name, "");
		tst_count++;
	}

	SAFE_RENAME(dname1, dname2);
	SAFE_RMDIR(dname2);

	/* Read more events on dirs */
	len += SAFE_READ(0, fd_notify, event_buf + len, EVENT_BUF_LEN - len);

	event_set[tst_count].mask = FAN_DIR_MODIFY;
	event_set[tst_count].fid = &root_fid;
	strcpy(event_set[tst_count].name, DIR_NAME1);
	tst_count++;
	event_set[tst_count].mask = FAN_DIR_MODIFY;
	event_set[tst_count].fid = &root_fid;
	strcpy(event_set[tst_count].name, DIR_NAME2);
	tst_count++;
	/*
	 * Directory watch gets self event on itself w/o name info.
	 */
	event_set[tst_count].mask = FAN_DELETE_SELF | FAN_ONDIR;
	strcpy(event_set[tst_count].name, "");
	event_set[tst_count].fid = &dir_fid;
	tst_count++;

	/*
	 * Cleanup the marks
	 */
	SAFE_CLOSE(fd_notify);
	fd_notify = -1;

	while (i < len) {
		struct event_t *expected = &event_set[test_num];
		struct fanotify_event_metadata *event;
		struct fanotify_event_info_fid *event_fid;
		struct file_handle *file_handle;
		unsigned int fhlen;
		const char *filename;
		int namelen, info_type;

		event = (struct fanotify_event_metadata *)&event_buf[i];
		event_fid = (struct fanotify_event_info_fid *)(event + 1);
		file_handle = (struct file_handle *)event_fid->handle;
		fhlen = file_handle->handle_bytes;
		filename = (char *)file_handle->f_handle + fhlen;
		namelen = ((char *)event + event->event_len) - filename;
		/* End of event could have name, zero padding, both or none */
		if (namelen > 0) {
			namelen = strlen(filename);
		} else {
			filename = "";
			namelen = 0;
		}

		if (expected->name[0]) {
			info_type = FAN_EVENT_INFO_TYPE_DFID_NAME;
		} else {
			info_type = FAN_EVENT_INFO_TYPE_FID;
		}

		if (test_num >= tst_count) {
			tst_res(TFAIL,
				"got unnecessary event: mask=%llx "
				"pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d",
				(unsigned long long)event->mask,
				(unsigned)event->pid, event->fd, filename,
				event->event_len, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		} else if (!fhlen || namelen < 0) {
			tst_res(TFAIL,
				"got event without fid: mask=%llx pid=%u fd=%d, "
				"len=%d info_type=%d info_len=%d fh_len=%d",
				(unsigned long long)event->mask,
				(unsigned)event->pid, event->fd,
				event->event_len, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		} else if (event->mask != expected->mask) {
			tst_res(TFAIL,
				"got event: mask=%llx (expected %llx) "
				"pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d",
				(unsigned long long)event->mask, expected->mask,
				(unsigned)event->pid, event->fd, filename,
				event->event_len, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		} else if (info_type != event_fid->hdr.info_type) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u fd=%d, "
				"len=%d info_type=%d expected(%d) info_len=%d fh_len=%d",
				(unsigned long long)event->mask,
				(unsigned)event->pid, event->fd,
				event->event_len, event_fid->hdr.info_type,
				info_type, event_fid->hdr.len, fhlen);
		} else if (fhlen != expected->fid->handle.handle_bytes) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d expected(%d)"
				"fh_type=%d",
				(unsigned long long)event->mask,
				(unsigned)event->pid, event->fd, filename,
				event->event_len, info_type,
				event_fid->hdr.len, fhlen,
				expected->fid->handle.handle_bytes,
				file_handle->handle_type);
		} else if (file_handle->handle_type !=
			   expected->fid->handle.handle_type) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d "
				"fh_type=%d expected(%x)",
				(unsigned long long)event->mask,
				(unsigned)event->pid, event->fd, filename,
				event->event_len, info_type,
				event_fid->hdr.len, fhlen,
				file_handle->handle_type,
				expected->fid->handle.handle_type);
		} else if (memcmp(file_handle->f_handle,
				  expected->fid->handle.f_handle, fhlen)) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d "
				"fh_type=%d unexpected file handle (%x...)",
				(unsigned long long)event->mask,
				(unsigned)event->pid, event->fd, filename,
				event->event_len, info_type,
				event_fid->hdr.len, fhlen,
				file_handle->handle_type,
				*(int *)(file_handle->f_handle));
		} else if (memcmp(&event_fid->fsid, &expected->fid->fsid,
				  sizeof(event_fid->fsid)) != 0) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d "
				"fsid=%x.%x (expected %x.%x)",
				(unsigned long long)event->mask,
				(unsigned)event->pid, event->fd, filename,
				event->event_len, info_type,
				event_fid->hdr.len, fhlen,
				FSID_VAL_MEMBER(event_fid->fsid, 0),
				FSID_VAL_MEMBER(event_fid->fsid, 1),
				expected->fid->fsid.val[0],
				expected->fid->fsid.val[1]);
		} else if (strcmp(expected->name, filename)) {
			tst_res(TFAIL,
				"got event: mask=%llx "
				"pid=%u fd=%d name='%s' expected('%s') "
				"len=%d info_type=%d info_len=%d fh_len=%d",
				(unsigned long long)event->mask,
				(unsigned)event->pid, event->fd,
				filename, expected->name,
				event->event_len, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		} else if (event->pid != getpid()) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u "
				"(expected %u) fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d",
				(unsigned long long)event->mask,
				(unsigned)event->pid,
				(unsigned)getpid(),
				event->fd, filename,
				event->event_len, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		} else {
			tst_res(TPASS,
				"got event #%d: mask=%llx pid=%u fd=%d name='%s' "
				"len=%d info_type=%d info_len=%d fh_len=%d",
				test_num, (unsigned long long)event->mask,
				(unsigned)event->pid, event->fd, filename,
				event->event_len, event_fid->hdr.info_type,
				event_fid->hdr.len, fhlen);
		}

		i += event->event_len;
		if (event->fd > 0)
			SAFE_CLOSE(event->fd);
		test_num++;
	}

	for (; test_num < tst_count; test_num++) {
		tst_res(TFAIL, "didn't get event: mask=%llx, name='%s'",
			 event_set[test_num].mask, event_set[test_num].name);

	}
}

static void setup(void)
{
	int fd;

	/* Check kernel for fanotify support */
	fd = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF, O_RDONLY);
	SAFE_CLOSE(fd);

	sprintf(dname1, "%s/%s", MOUNT_PATH, DIR_NAME1);
	sprintf(dname2, "%s/%s", MOUNT_PATH, DIR_NAME2);
	sprintf(fname1, "%s/%s", dname1, FILE_NAME1);
	sprintf(fname2, "%s/%s", dname1, FILE_NAME2);
}

static void cleanup(void)
{
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(test_cases),
	.dev_fs_flags = TST_FS_SKIP_FUSE,
	.setup = setup,
	.cleanup = cleanup,
	.mount_device = 1,
	.mntpoint = MOUNT_PATH,
	.all_filesystems = 1,
	.needs_tmpdir = 1,
	.needs_root = 1
};

#else
	TST_TEST_TCONF("system does not have required name_to_handle_at() support");
#endif
#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
