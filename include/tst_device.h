// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2019 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_DEVICE_H__
#define TST_DEVICE_H__

#include <unistd.h>

struct tst_device {
	const char *dev;
	const char *fs_type;
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
int tst_find_free_loopdev(const char *path, size_t path_len);

/*
 * Attaches a file to a loop device.
 *
 * @dev_path Path to the loop device e.g. /dev/loop0
 * @file_path Path to a file e.g. disk.img
 * @return Zero on success, non-zero otherwise.
 */
int tst_attach_device(const char *dev_path, const char *file_path);

/*
 * Detaches a file from a loop device.
 *
 * @dev_path Path to the loop device e.g. /dev/loop0
 * @return Zero on succes, non-zero otherwise.
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
 * Wipe the contents of given directory but keep the directory itself
 */
void tst_purge_dir(const char *path);

#endif	/* TST_DEVICE_H__ */
