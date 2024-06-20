// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2019 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2019-2024
 */

#ifndef TST_DEVICE_H__
#define TST_DEVICE_H__

#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>

struct tst_device {
	const char *dev;
	const char *fs_type;
	uint64_t size;
};

/*
 * Automatically initialized if test.needs_device is set.
 */
extern struct tst_device *tst_device;

/*
 * Just like umount() but retries several times on failure.
 * @path: Path to umount
 */
int tst_umount(const char *path);

/*
 * Verifies if an earlier mount is successful or not.
 * @path: Mount path to verify
 */
int tst_is_mounted(const char *path);
int tst_is_mounted_at_tmpdir(const char *path);

/*
 * Clears a first few blocks of the device. This is needed when device has
 * already been formatted with a filesystems, subset of mkfs.foo utils aborts
 * the operation if it finds a filesystem signature there.
 *
 * Note that this is called from tst_mkfs() automatically, so you probably will
 * not need to use this from the test yourself.
 */
int tst_clear_device(const char *dev);

/*
 * Finds a free loop device for use and returns the free loopdev minor(-1 for no
 * free loopdev). If path is non-NULL, it will be filled with free loopdev path.
 *
 */
int tst_find_free_loopdev(char *path, size_t path_len);

/*
 * Attaches a file to a loop device.
 *
 * @dev_path Path to the loop device e.g. /dev/loop0
 * @file_path Path to a file e.g. disk.img
 * @return Zero on success, non-zero otherwise.
 */
int tst_attach_device(const char *dev_path, const char *file_path);

/*
 * Get size (in MB) of the given device
 */
uint64_t tst_get_device_size(const char *dev_path);

/*
 * Detaches a file from a loop device fd.
 *
 * @dev_path Path to the loop device e.g. /dev/loop0
 * @dev_fd a open fd for the loop device
 * @return Zero on succes, non-zero otherwise.
 */
int tst_detach_device_by_fd(const char *dev_path, int dev_fd);

/*
 * Detaches a file from a loop device.
 *
 * @dev_path Path to the loop device e.g. /dev/loop0
 * @return Zero on succes, non-zero otherwise.
 *
 * Internally this function opens the device and calls
 * tst_detach_device_by_fd(). If you keep device file descriptor open you
 * have to call the by_fd() variant since having the device open twice will
 * prevent it from being detached.
 */
int tst_detach_device(const char *dev_path);

/*
 * To avoid FS deferred IO metadata/cache interference, so we do syncfs
 * simply before the tst_dev_bytes_written invocation. For easy to use,
 * we create this inline function tst_dev_sync.
 */
int tst_dev_sync(int fd);

/*
 * Reads test block device stat file and returns the bytes written since the
 * last call of this function.
 * @dev: test block device
 */
unsigned long tst_dev_bytes_written(const char *dev);

/*
 * Find the file or path belongs to which block dev
 * @path       Path to find the backing dev
 * @dev        The buffer to store the block dev in
 * @dev_size   The length of the block dev buffer
 */
void tst_find_backing_dev(const char *path, char *dev, size_t dev_size);

/*
 * Stat the device mounted on a given path.
 */
void tst_stat_mount_dev(const char *const mnt_path, struct stat *const st);

/*
 * Returns the size of a physical device block size for the specific path
 * @path   Path to find the block size
 * @return Size of the block size
 */
int tst_dev_block_size(const char *path);

#endif /* TST_DEVICE_H__ */
