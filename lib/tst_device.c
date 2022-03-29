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
#include <sys/mount.h>
#include <mntent.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/loop.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/sysmacros.h>
#include "lapi/syscalls.h"
#include "test.h"
#include "safe_macros.h"

#ifndef LOOP_CTL_GET_FREE
# define LOOP_CTL_GET_FREE 0x4C82
#endif

#define LOOP_CONTROL_FILE "/dev/loop-control"

#define DEV_FILE "test_dev.img"
#define DEV_SIZE_MB 256u

static char dev_path[1024];
static int device_acquired;
static unsigned long prev_dev_sec_write;

static const char *dev_variants[] = {
	"/dev/loop%i",
	"/dev/loop/%i",
	"/dev/block/loop%i"
};

static int set_dev_path(int dev, char *path, size_t path_len)
{
	unsigned int i;
	struct stat st;

	for (i = 0; i < ARRAY_SIZE(dev_variants); i++) {
		snprintf(path, path_len, dev_variants[i], dev);

		if (stat(path, &st) == 0 && S_ISBLK(st.st_mode))
			return 1;
	}

	return 0;
}

int tst_find_free_loopdev(char *path, size_t path_len)
{
	int ctl_fd, dev_fd, rc, i;
	struct loop_info loopinfo;
	char buf[1024];

	/* since Linux 3.1 */
	ctl_fd = open(LOOP_CONTROL_FILE, O_RDWR);

	if (ctl_fd > 0) {
		rc = ioctl(ctl_fd, LOOP_CTL_GET_FREE);
		close(ctl_fd);
		if (rc >= 0) {
			if (path)
				set_dev_path(rc, path, path_len);
			tst_resm(TINFO, "Found free device %d '%s'",
				rc, path ?: "");
			return rc;
		}
		tst_resm(TINFO, "Couldn't find free loop device");
		return -1;
	}

	switch (errno) {
	case ENOENT:
	break;
	case EACCES:
		tst_resm(TINFO | TERRNO,
		         "Not allowed to open " LOOP_CONTROL_FILE ". "
			 "Are you root?");
	break;
	default:
		tst_resm(TBROK | TERRNO, "Failed to open " LOOP_CONTROL_FILE);
	}

	/*
	 * Older way is to iterate over /dev/loop%i and /dev/loop/%i and try
	 * LOOP_GET_STATUS ioctl() which fails for free loop devices.
	 */
	for (i = 0; i < 256; i++) {

		if (!set_dev_path(i, buf, sizeof(buf)))
			continue;

		dev_fd = open(buf, O_RDONLY);

		if (dev_fd < 0)
			continue;

		if (ioctl(dev_fd, LOOP_GET_STATUS, &loopinfo) == 0) {
			tst_resm(TINFO, "Device '%s' in use", buf);
		} else {
			if (errno != ENXIO)
				continue;
			tst_resm(TINFO, "Found free device '%s'", buf);
			close(dev_fd);
			if (path != NULL) {
				strncpy(path, buf, path_len);
				path[path_len-1] = '\0';
			}
			return i;
		}

		close(dev_fd);
	}

	tst_resm(TINFO, "No free devices found");

	return -1;
}

int tst_attach_device(const char *dev, const char *file)
{
	int dev_fd, file_fd;
	struct loop_info loopinfo;

	dev_fd = open(dev, O_RDWR);
	if (dev_fd < 0) {
		tst_resm(TWARN | TERRNO, "open('%s', O_RDWR) failed", dev);
		return 1;
	}

	file_fd = open(file, O_RDWR);
	if (file_fd < 0) {
		tst_resm(TWARN | TERRNO, "open('%s', O_RDWR) failed", file);
		close(dev_fd);
		return 1;
	}

	if (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
		close(dev_fd);
		close(file_fd);
		tst_resm(TWARN | TERRNO, "ioctl(%s, LOOP_SET_FD, %s) failed",
			 dev, file);
		return 1;
	}

	/* Old mkfs.btrfs use LOOP_GET_STATUS instead of backing_file to get
	 * associated filename, so we need to set up the device by calling
	 * LOOP_SET_FD and LOOP_SET_STATUS.
	 */
	memset(&loopinfo, 0, sizeof(loopinfo));
	strcpy(loopinfo.lo_name, file);

	if (ioctl(dev_fd, LOOP_SET_STATUS, &loopinfo)) {
		close(dev_fd);
		close(file_fd);
		tst_resm(TWARN | TERRNO,
			 "ioctl(%s, LOOP_SET_STATUS, %s) failed", dev, file);
		return 1;
	}

	close(dev_fd);
	close(file_fd);
	return 0;
}

uint64_t tst_get_device_size(const char *dev_path)
{
	int fd;
	uint64_t size;
	struct stat st;

	if (!dev_path)
		tst_brkm(TBROK, NULL, "No block device path");

	if (stat(dev_path, &st)) {
		tst_resm(TWARN | TERRNO, "stat() failed");
		return -1;
	}

	if (!S_ISBLK(st.st_mode)) {
		tst_resm(TWARN, "%s is not a block device", dev_path);
		return -1;
	}

	fd = open(dev_path, O_RDONLY);
	if (fd < 0) {
		tst_resm(TWARN | TERRNO,
				"open(%s, O_RDONLY) failed", dev_path);
		return -1;
	}

	if (ioctl(fd, BLKGETSIZE64, &size)) {
		tst_resm(TWARN | TERRNO,
				"ioctl(fd, BLKGETSIZE64, ...) failed");
		close(fd);
		return -1;
	}

	if (close(fd)) {
		tst_resm(TWARN | TERRNO,
				"close(fd) failed");
		return -1;
	}

	return size/1024/1024;
}

int tst_detach_device_by_fd(const char *dev, int dev_fd)
{
	int ret, i;

	/* keep trying to clear LOOPDEV until we get ENXIO, a quick succession
	 * of attach/detach might not give udev enough time to complete */
	for (i = 0; i < 40; i++) {
		ret = ioctl(dev_fd, LOOP_CLR_FD, 0);

		if (ret && (errno == ENXIO))
			return 0;

		if (ret && (errno != EBUSY)) {
			tst_resm(TWARN,
				 "ioctl(%s, LOOP_CLR_FD, 0) unexpectedly failed with: %s",
				 dev, tst_strerrno(errno));
			return 1;
		}

		usleep(50000);
	}

	tst_resm(TWARN,
		"ioctl(%s, LOOP_CLR_FD, 0) no ENXIO for too long", dev);
	return 1;
}

int tst_detach_device(const char *dev)
{
	int dev_fd, ret;

	dev_fd = open(dev, O_RDONLY);
	if (dev_fd < 0) {
		tst_resm(TWARN | TERRNO, "open(%s) failed", dev);
		return 1;
	}

	ret = tst_detach_device_by_fd(dev, dev_fd);
	close(dev_fd);
	return ret;
}

int tst_dev_sync(int fd)
{
	return syscall(__NR_syncfs, fd);
}

const char *tst_acquire_loop_device(unsigned int size, const char *filename)
{
	unsigned int acq_dev_size = size ? size : DEV_SIZE_MB;

	if (tst_prealloc_file(filename, 1024 * 1024, acq_dev_size)) {
		tst_resm(TWARN | TERRNO, "Failed to create %s", filename);
		return NULL;
	}

	if (tst_find_free_loopdev(dev_path, sizeof(dev_path)) == -1)
		return NULL;

	if (tst_attach_device(dev_path, filename))
		return NULL;

	return dev_path;
}

const char *tst_acquire_device__(unsigned int size)
{
	const char *dev;
	unsigned int acq_dev_size;
	uint64_t ltp_dev_size;

	acq_dev_size = size ? size : DEV_SIZE_MB;

	dev = getenv("LTP_DEV");

	if (dev) {
		tst_resm(TINFO, "Using test device LTP_DEV='%s'", dev);

		ltp_dev_size = tst_get_device_size(dev);

		if (acq_dev_size <= ltp_dev_size)
			return dev;

		tst_resm(TINFO, "Skipping $LTP_DEV size %"PRIu64"MB, requested size %uMB",
				ltp_dev_size, acq_dev_size);
	}

	dev = tst_acquire_loop_device(acq_dev_size, DEV_FILE);

	if (dev)
		device_acquired = 1;

	return dev;
}

const char *tst_acquire_device_(void (cleanup_fn)(void), unsigned int size)
{
	const char *device;

	if (device_acquired) {
		tst_brkm(TBROK, cleanup_fn, "Device already acquired");
		return NULL;
	}

	if (!tst_tmpdir_created()) {
		tst_brkm(TBROK, cleanup_fn,
		         "Cannot acquire device without tmpdir() created");
		return NULL;
	}

	device = tst_acquire_device__(size);

	if (!device) {
		tst_brkm(TBROK, cleanup_fn, "Failed to acquire device");
		return NULL;
	}

	return device;
}

int tst_release_device(const char *dev)
{
	int ret;

	if (!device_acquired)
		return 0;

	/*
	 * Loop device was created -> we need to detach it.
	 *
	 * The file image is deleted in tst_rmdir();
	 */
	ret = tst_detach_device(dev);

	device_acquired = 0;

	return ret;
}

int tst_clear_device(const char *dev)
{
	if (tst_fill_file(dev, 0, 1024, 512)) {
		tst_resm(TWARN, "Failed to clear 512k block on %s", dev);
		return 1;
	}

	return 0;
}

int tst_umount(const char *path)
{
	int err, ret, i;

	for (i = 0; i < 50; i++) {
		ret = umount(path);
		err = errno;

		if (!ret)
			return 0;

		if (err != EBUSY) {
			tst_resm(TWARN, "umount('%s') failed with %s",
		         path, tst_strerrno(err));
			errno = err;
			return ret;
		}

		tst_resm(TINFO, "umount('%s') failed with %s, try %2i...",
		         path, tst_strerrno(err), i+1);

		if (i == 0) {
			tst_resm(TINFO, "Likely gvfsd-trash is probing newly "
			         "mounted fs, kill it to speed up tests.");
		}

		usleep(100000);
	}

	tst_resm(TWARN, "Failed to umount('%s') after 50 retries", path);
	errno = err;
	return -1;
}

int tst_is_mounted(const char *path)
{
	char line[PATH_MAX];
	FILE *file;
	int ret = 0;

	file = SAFE_FOPEN(NULL, "/proc/mounts", "r");

	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, path) != NULL) {
			ret = 1;
			break;
		}
	}

	SAFE_FCLOSE(NULL, file);

	if (!ret)
		tst_resm(TINFO, "No device is mounted at %s", path);

	return ret;
}

int tst_is_mounted_at_tmpdir(const char *path)
{
	char cdir[PATH_MAX], mpath[PATH_MAX];
	int ret;

	if (!getcwd(cdir, PATH_MAX)) {
		tst_resm(TWARN | TERRNO, "Failed to find current directory");
		return 0;
	}

	ret = snprintf(mpath, PATH_MAX, "%s/%s", cdir, path);
	if (ret < 0 || ret >= PATH_MAX) {
		tst_resm(TWARN | TERRNO,
			 "snprintf() should have returned %d instead of %d",
			 PATH_MAX, ret);
		return 0;
	}

	return tst_is_mounted(mpath);
}

int find_stat_file(const char *dev, char *path, size_t path_len)
{
	const char *devname = strrchr(dev, '/') + 1;

	snprintf(path, path_len, "/sys/block/%s/stat", devname);

	if (!access(path, F_OK))
		return 1;

	DIR *dir = SAFE_OPENDIR(NULL, "/sys/block/");
	struct dirent *ent;

	while ((ent = readdir(dir))) {
		snprintf(path, path_len, "/sys/block/%s/%s/stat", ent->d_name, devname);

		if (!access(path, F_OK)) {
			SAFE_CLOSEDIR(NULL, dir);
			return 1;
		}
	}

	SAFE_CLOSEDIR(NULL, dir);
	return 0;
}

unsigned long tst_dev_bytes_written(const char *dev)
{
	unsigned long dev_sec_write = 0, dev_bytes_written, io_ticks = 0;
	char dev_stat_path[1024];

	if (!find_stat_file(dev, dev_stat_path, sizeof(dev_stat_path)))
		tst_brkm(TCONF, NULL, "Test device stat file: %s not found",
			 dev_stat_path);

	SAFE_FILE_SCANF(NULL, dev_stat_path,
			"%*s %*s %*s %*s %*s %*s %lu %*s %*s %lu",
			&dev_sec_write, &io_ticks);

	if (!io_ticks)
		tst_brkm(TCONF, NULL, "Test device stat file: %s broken",
			 dev_stat_path);

	dev_bytes_written = (dev_sec_write - prev_dev_sec_write) * 512;

	prev_dev_sec_write = dev_sec_write;

	return dev_bytes_written;
}

void tst_find_backing_dev(const char *path, char *dev)
{
	struct stat buf;
	FILE *file;
	char line[PATH_MAX];
	char *pre = NULL;
	char *next = NULL;
	unsigned int dev_major, dev_minor, line_mjr, line_mnr;

	if (stat(path, &buf) < 0)
		tst_brkm(TWARN | TERRNO, NULL, "stat() failed");

	dev_major = major(buf.st_dev);
	dev_minor = minor(buf.st_dev);
	file = SAFE_FOPEN(NULL, "/proc/self/mountinfo", "r");
	*dev = '\0';

	while (fgets(line, sizeof(line), file)) {
		if (sscanf(line, "%*d %*d %d:%d", &line_mjr, &line_mnr) != 2)
			continue;

		if (line_mjr == dev_major && line_mnr == dev_minor) {
			pre = strstr(line, " - ");
			pre = strtok_r(pre, " ", &next);
			pre = strtok_r(NULL, " ", &next);
			pre = strtok_r(NULL, " ", &next);
			strcpy(dev, pre);
			break;
		}
	}

	SAFE_FCLOSE(NULL, file);

	if (!*dev)
		tst_brkm(TBROK, NULL, "Cannot find block device for %s", path);

	if (stat(dev, &buf) < 0)
		tst_brkm(TWARN | TERRNO, NULL, "stat(%s) failed", dev);

	if (S_ISBLK(buf.st_mode) != 1)
		tst_brkm(TCONF, NULL, "dev(%s) isn't a block dev", dev);
}

void tst_stat_mount_dev(const char *const mnt_path, struct stat *const st)
{
	struct mntent *mnt;
	FILE *mntf = setmntent("/proc/self/mounts", "r");

	if (!mntf) {
		tst_brkm(TBROK | TERRNO, NULL, "Can't open /proc/self/mounts");
		return;
	}

	mnt = getmntent(mntf);
	if (!mnt) {
		tst_brkm(TBROK | TERRNO, NULL, "Can't read mounts or no mounts?");
		return;
	}

	do {
		if (strcmp(mnt->mnt_dir, mnt_path)) {
			mnt = getmntent(mntf);
			continue;
		}

		if (stat(mnt->mnt_fsname, st)) {
			tst_brkm(TBROK | TERRNO, NULL,
				 "Can't stat '%s', mounted at '%s'",
				 mnt->mnt_fsname, mnt_path);
		}

		return;
	} while (mnt);

	tst_brkm(TBROK, NULL, "Could not find mount device");
}

int tst_dev_block_size(const char *path)
{
	int fd;
	int size;
	char dev_name[1024];

	tst_find_backing_dev(path, dev_name);

	fd = SAFE_OPEN(NULL, dev_name, O_RDONLY);
	SAFE_IOCTL(NULL, fd, BLKSSZGET, &size);
	SAFE_CLOSE(NULL, fd);

	return size;
}
