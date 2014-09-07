/*
 * Copyright (C) 2014 Cyril Hrubis chrubis@suse.cz
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
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef __linux__
#include <linux/loop.h>
#endif
#include "test.h"
#include "safe_macros.h"

static int device_acquired;

#ifdef __linux__

#ifndef LOOP_CTL_GET_FREE
# define LOOP_CTL_GET_FREE 0x4C82
#endif

#define LOOP_CONTROL_FILE "/dev/loop-control"

#define DEV_FILE "test_dev.img"

static char dev_path[1024];

static const char *dev_variants[] = {
	"/dev/loop%i",
	"/dev/loop/%i"
};

static int set_dev_path(int dev)
{
	unsigned int i;
	struct stat st;

	for (i = 0; i < ARRAY_SIZE(dev_variants); i++) {
		snprintf(dev_path, sizeof(dev_path), dev_variants[i], dev);

		if (stat(dev_path, &st) == 0 && S_ISBLK(st.st_mode))
			return 1;
	}

	return 0;
}

static int find_free_loopdev(void)
{
	int ctl_fd, dev_fd, rc, i;
	struct loop_info loopinfo;

	/* since Linux 3.1 */
	ctl_fd = open(LOOP_CONTROL_FILE, O_RDWR);

	if (ctl_fd > 0) {
		rc = ioctl(ctl_fd, LOOP_CTL_GET_FREE);
		close(ctl_fd);
		if (rc >= 0) {
			set_dev_path(rc);
			tst_resm(TINFO, "Found free device '%s'", dev_path);
			return 0;
		}
		tst_resm(TINFO, "Couldn't find free loop device");
		return 1;
	}

	switch (errno) {
	case ENOENT:
	break;
	default:
		tst_resm(TBROK | TERRNO, "Failed to open " LOOP_CONTROL_FILE);
	}

	/*
	 * Older way is to iterate over /dev/loop%i and /dev/loop/%i and try
	 * LOOP_GET_STATUS ioctl() which fails for free loop devices.
	 */
	for (i = 0; i < 256; i++) {

		if (!set_dev_path(i))
			continue;

		dev_fd = open(dev_path, O_RDONLY);

		if (dev_fd < 0)
			continue;

		if (ioctl(dev_fd, LOOP_GET_STATUS, &loopinfo) == 0) {
			tst_resm(TINFO, "Device '%s' in use", dev_path);
		} else {
			if (errno != ENXIO)
				continue;
			tst_resm(TINFO, "Found free device '%s'", dev_path);
			close(dev_fd);
			return 0;
		}

		close(dev_fd);
	}

	tst_resm(TINFO, "No free devices found");

	return 1;
}

static void attach_device(void (*cleanup_fn)(void),
                          const char *dev, const char *file)
{
	int dev_fd, file_fd, err;

	dev_fd = SAFE_OPEN(cleanup_fn, dev, O_RDWR);
	file_fd = SAFE_OPEN(cleanup_fn, file, O_RDWR);

	if (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
		err = errno;
		close(dev_fd);
		close(file_fd);
		tst_brkm(TBROK, cleanup_fn,
		         "ioctl(%s, LOOP_SET_FD, %s) failed: %s",
			 dev, file, tst_strerrno(err));
	}

	close(dev_fd);
	close(file_fd);
}

static void detach_device(void (*cleanup_fn)(void), const char *dev)
{
	int dev_fd, err;

	dev_fd = SAFE_OPEN(cleanup_fn, dev, O_RDONLY);

	if (ioctl(dev_fd, LOOP_CLR_FD, 0) < 0) {
		err = errno;
		close(dev_fd);
		tst_brkm(TBROK, cleanup_fn,
		         "ioctl(%s, LOOP_CLR_FD, 0) failed: %s",
			 dev, tst_strerrno(err));
	}

	close(dev_fd);
}

const char *tst_acquire_device(void (cleanup_fn)(void))
{
	char *dev;
	struct stat st;

	if (device_acquired)
		tst_brkm(TBROK, cleanup_fn, "Device allready acquired");

	if (!tst_tmpdir_created()) {
		tst_brkm(TBROK, cleanup_fn,
		         "Cannot acquire device without tmpdir() created");
	}

	dev = getenv("LTP_DEV");

	if (dev) {
		tst_resm(TINFO, "Using test device LTP_DEV='%s'", dev);

		SAFE_STAT(cleanup_fn, dev, &st);

		if (!S_ISBLK(st.st_mode)) {
			tst_brkm(TBROK, cleanup_fn,
			         "%s is not a block device", dev);
		}

		return dev;
	}

	if (tst_fill_file(DEV_FILE, 0, 1024, 20480)) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
		         "Failed to create " DEV_FILE);

	}

	if (find_free_loopdev())
		return NULL;

	attach_device(cleanup_fn, dev_path, DEV_FILE);

	device_acquired = 1;

	return dev_path;
}

void tst_release_device(void (cleanup_fn)(void), const char *dev)
{
	if (getenv("LTP_DEV"))
		return;

	/*
	 * Loop device was created -> we need to deatch it.
	 *
	 * The file image is deleted in tst_rmdir();
	 */
	detach_device(cleanup_fn, dev);

	device_acquired = 0;
}
#else
const char *tst_acquire_device(void (cleanup_fn)(void))
{

	return NULL;
}

void tst_release_device(void (cleanup_fn)(void), const char *dev)
{

	device_acquired = 0;
}
#endif
