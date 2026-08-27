// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verifies that the loop device sysfs attributes exported under
 * /sys/block/loopN/loop/ correctly reflect the state of an attached backing
 * file, and reset once the device is detached again.
 *
 * The test creates a plain file, attaches it to a free loop device and
 * verifies that:
 *
 * - loop/backing_file matches the absolute path of the attached file
 * - loop/offset and loop/sizelimit are 0 (defaults for a plain attach)
 * - loop/autoclear, loop/partscan and loop/dio (when present) are booleans
 *   and default to 0
 * - the device's own size (in 512 byte sectors) matches the backing file
 *   size
 * - ro is 0 (attached read-write by default)
 *
 * After detaching the device it verifies that:
 *
 * - the device's size is back to 0
 * - loop/backing_file no longer exists or reads back empty
 *
 * The device is then reconfigured via the LOOP_SET_STATUS64 ioctl (offset,
 * sizelimit and the partscan flag) and it is verified that:
 *
 * - loop/offset and loop/sizelimit match the values that were set
 * - loop/partscan reflects the requested flag
 * - the device's size matches sizelimit/512, i.e. it is capped by sizelimit
 *   rather than reflecting the whole backing file
 *
 * Note that LO_FLAGS_READ_ONLY is not exercised this way: on an already
 * attached (read-write) device, LOOP_SET_STATUS64 does not actually flip the
 * device to read-only, ro stays 0 regardless of the flag passed in. Read-only
 * mode has to be requested at LOOP_SET_FD time instead, by opening the loop
 * device (or the backing file) O_RDONLY before issuing the ioctl, which is
 * exercised separately: the device is attached this way and it is verified
 * that:
 *
 * - ro reads back as 1
 * - the device genuinely rejects being opened or written to O_RDWR, not just
 *   that the attribute says so
 *
 * ro turns out not to reset to 0 merely from detaching: it is a property of
 * the whole gendisk, recalculated only on the next LOOP_SET_FD, so after
 * detaching the device is reattached read-write and ro is checked to be 0
 * then instead.
 *
 * Finally, autoclear is exercised for real rather than just checking that
 * the attribute round-trips: the device is attached again, LO_FLAGS_AUTOCLEAR
 * is set and the last open file descriptor on the device is closed without
 * calling LOOP_CLR_FD explicitly. The kernel is expected to detach the
 * device on its own (asynchronously, hence the test polls for it).
 *
 * This needs root to attach a loop device.
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tst_test.h"
#include "tst_device.h"
#include "tst_sysfs_assert.h"
#include "lapi/loop.h"

#define BACKING_FILE "sys_block_loop01.img"
#define FILE_SIZE_MB 4
#define RECONF_OFFSET 4096
#define RECONF_SIZELIMIT (1024 * 1024)

static char dev_path[64];
static char loop_dir[64];
static char abs_backing_file[PATH_MAX + sizeof(BACKING_FILE)];
static int dev_num;
static int attached;

static void setup(void)
{
	char cwd[PATH_MAX];

	dev_num = tst_find_free_loopdev(dev_path, sizeof(dev_path));
	if (dev_num < 0)
		tst_brk(TBROK, "Failed to find free loop device");

	snprintf(loop_dir, sizeof(loop_dir), "/sys/block/loop%d", dev_num);

	tst_prealloc_file(BACKING_FILE, 1024 * 1024, FILE_SIZE_MB);

	SAFE_GETCWD(cwd, sizeof(cwd));
	snprintf(abs_backing_file, sizeof(abs_backing_file), "%s/%s", cwd,
		 BACKING_FILE);
}

static void check_backing_file(void)
{
	char actual[PATH_MAX] = "";

	TST_SYSFS_READ_STR(actual, sizeof(actual), "%s/loop/backing_file",
			   loop_dir);

	TST_EXP_EQ_STR(actual, abs_backing_file);
}

static void check_attached(void)
{
	check_backing_file();

	TST_SYSFS_EXP_EQ_LI(0, "%s/loop/offset", loop_dir);
	TST_SYSFS_EXP_EQ_LI(0, "%s/loop/sizelimit", loop_dir);

	TST_SYSFS_ASSERT_BOOL("%s/loop/autoclear", loop_dir);
	TST_SYSFS_EXP_EQ_LI(0, "%s/loop/autoclear", loop_dir);

	TST_SYSFS_ASSERT_BOOL("%s/loop/partscan", loop_dir);
	TST_SYSFS_EXP_EQ_LI(0, "%s/loop/partscan", loop_dir);

	if (tst_sysfs_exists("%s/loop/dio", loop_dir)) {
		TST_SYSFS_ASSERT_BOOL("%s/loop/dio", loop_dir);
		TST_SYSFS_EXP_EQ_LI(0, "%s/loop/dio", loop_dir);
	} else {
		tst_res(TCONF, "%s/loop/dio does not exist", loop_dir);
	}

	TST_SYSFS_ASSERT_BOOL("/sys/block/loop%d/ro", dev_num);
	TST_SYSFS_EXP_EQ_LI(0, "/sys/block/loop%d/ro", dev_num);

	TST_SYSFS_EXP_EQ_LI((long)FILE_SIZE_MB * 1024 * 1024 / 512,
			 "/sys/block/loop%d/size", dev_num);
}

static void check_detached(void)
{
	TST_SYSFS_EXP_EQ_LI(0, "/sys/block/loop%d/size", dev_num);

	if (!tst_sysfs_exists("%s/loop/backing_file", loop_dir)) {
		tst_res(TPASS, "backing_file no longer exists after detach");
		return;
	}

	char actual[PATH_MAX] = "";

	TST_SYSFS_READ_STR(actual, sizeof(actual), "%s/loop/backing_file",
			   loop_dir);

	if (actual[0] == '\0')
		tst_res(TPASS, "backing_file is empty after detach");
	else
		tst_res(TFAIL, "backing_file = '%s' after detach, expected empty",
			actual);
}

static void check_reconfigured(void)
{
	struct loop_info64 info = { 0 };
	int fd;

	fd = SAFE_OPEN(dev_path, O_RDWR);

	info.lo_offset = RECONF_OFFSET;
	info.lo_sizelimit = RECONF_SIZELIMIT;
	info.lo_flags = LO_FLAGS_PARTSCAN;

	SAFE_IOCTL(fd, LOOP_SET_STATUS64, &info);
	SAFE_CLOSE(fd);

	tst_res(TINFO,
		"Reconfigured via LOOP_SET_STATUS64: offset=%d sizelimit=%d partscan",
		RECONF_OFFSET, RECONF_SIZELIMIT);

	TST_SYSFS_EXP_EQ_LI(RECONF_OFFSET, "%s/loop/offset", loop_dir);
	TST_SYSFS_EXP_EQ_LI(RECONF_SIZELIMIT, "%s/loop/sizelimit", loop_dir);
	TST_SYSFS_EXP_EQ_LI(1, "%s/loop/partscan", loop_dir);
	TST_SYSFS_EXP_EQ_LI(RECONF_SIZELIMIT / 512, "/sys/block/loop%d/size",
			 dev_num);
}

static void check_readonly_attach(void)
{
	int dev_fd, file_fd;

	tst_res(TINFO,
		"Checking a read-only attach (LOOP_SET_FD with an O_RDONLY device fd)");

	dev_fd = SAFE_OPEN(dev_path, O_RDWR);
	file_fd = SAFE_OPEN(BACKING_FILE, O_RDONLY);

	SAFE_IOCTL(dev_fd, LOOP_SET_FD, file_fd);
	SAFE_CLOSE(file_fd);
	SAFE_CLOSE(dev_fd);
	attached = 1;

	TST_SYSFS_EXP_EQ_LI(1, "/sys/block/loop%d/ro", dev_num);

	dev_fd = open(dev_path, O_RDWR);

	if (dev_fd < 0) {
		tst_res(TPASS,
			"open(%s, O_RDWR) failed as expected on a read-only device: %s",
			dev_path, tst_strerrno(errno));
	} else {
		char buf[512] = { 0 };
		ssize_t written = write(dev_fd, buf, sizeof(buf));

		if (written < 0) {
			tst_res(TPASS,
				"write() to a read-only loop device failed as expected: %s",
				tst_strerrno(errno));
		} else {
			tst_res(TFAIL,
				"write() to a read-only loop device unexpectedly succeeded (%zd bytes)",
				written);
		}

		SAFE_CLOSE(dev_fd);
	}

	if (tst_detach_device(dev_path))
		tst_brk(TBROK, "Failed to detach %s", dev_path);
	attached = 0;

	/*
	 * ro does not reset to 0 merely from detaching, it is a property of
	 * the whole gendisk that is only recalculated on the next LOOP_SET_FD,
	 * so the device is reattached read-write here to verify that.
	 */
	if (tst_attach_device(dev_path, BACKING_FILE))
		tst_brk(TBROK, "Failed to attach %s to %s", BACKING_FILE,
			dev_path);
	attached = 1;

	TST_SYSFS_EXP_EQ_LI(0, "/sys/block/loop%d/ro", dev_num);

	if (tst_detach_device(dev_path))
		tst_brk(TBROK, "Failed to detach %s", dev_path);
	attached = 0;
}

/* Reads back /sys/block/loopN/size, used to poll for autoclear below. */
static long read_loop_size(void)
{
	return TST_SYSFS_READ_LI("/sys/block/loop%d/size", dev_num);
}

static void check_autoclear_behavior(void)
{
	struct loop_info64 info = { 0 };
	long size;
	int fd;

	tst_res(TINFO,
		"Checking that autoclear detaches the device on last close");

	if (tst_attach_device(dev_path, BACKING_FILE)) {
		tst_res(TFAIL, "Failed to attach %s to %s for autoclear check",
			BACKING_FILE, dev_path);
		return;
	}
	attached = 1;

	fd = SAFE_OPEN(dev_path, O_RDWR);
	info.lo_flags = LO_FLAGS_AUTOCLEAR;
	SAFE_IOCTL(fd, LOOP_SET_STATUS64, &info);
	SAFE_CLOSE(fd);

	/* autoclear runs asynchronously off a kernel workqueue, poll for it */
	size = TST_RETRY_FN_EXP_BACKOFF(read_loop_size(), TST_RETVAL_EQ0, 5);

	if (size == 0) {
		tst_res(TPASS, "Device auto-detached");
	} else {
		tst_res(TFAIL, "Device did not auto-detach in time");
		tst_detach_device(dev_path);
	}

	attached = 0;
}

static void run(void)
{
	if (tst_attach_device(dev_path, BACKING_FILE))
		tst_brk(TBROK, "Failed to attach %s to %s", BACKING_FILE,
			dev_path);
	attached = 1;

	check_attached();
	check_reconfigured();

	if (tst_detach_device(dev_path))
		tst_brk(TBROK, "Failed to detach %s", dev_path);
	attached = 0;

	check_detached();

	check_readonly_attach();
	check_autoclear_behavior();
}

static void cleanup(void)
{
	if (attached)
		tst_detach_device(dev_path);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.needs_kconfigs = (const char *const[]){
		"CONFIG_BLK_DEV_LOOP",
		NULL
	},
};
