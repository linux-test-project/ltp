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

/**
 * struct tst_device - Block device used by a test.
 * @dev: Device path (e.g. /dev/loop0).
 * @fs_type: Filesystem type used to format the device.
 * @size: Device size in megabytes.
 * @is_fuse: Set by the library when the mounted filesystem is FUSE-based.
 */
struct tst_device {
	const char *dev;
	const char *fs_type;
	uint64_t size;
	int is_fuse;
};

/*
 * tst_device - Pointer to the test block device.
 *
 * Automatically initialized if tst_test.needs_device is set.
 */
extern struct tst_device *tst_device;

/**
 * tst_umount() - Unmount a filesystem, retrying on transient failures.
 * @path: Mount point to unmount.
 *
 * Return: 0 on success, -1 on failure.
 */
int tst_umount(const char *path);

/**
 * tst_mount_has_opt() - Check if a mount point has a specific mount option.
 * @path: Mount point to check.
 * @opt: Mount option to look for.
 *
 * Return: 1 if found, 0 otherwise.
 */
int tst_mount_has_opt(const char *path, const char *opt);

/**
 * tst_is_mounted() - Check if a path is a mount point.
 * @path: Path to check.
 *
 * Return: 1 if mounted, 0 otherwise.
 */
int tst_is_mounted(const char *path);

/**
 * tst_is_mounted_ro() - Check if a path is mounted read-only.
 * @path: Path to check.
 *
 * Return: 1 if mounted read-only, 0 otherwise.
 */
int tst_is_mounted_ro(const char *path);

/**
 * tst_is_mounted_rw() - Check if a path is mounted read-write.
 * @path: Path to check.
 *
 * Return: 1 if mounted read-write, 0 otherwise.
 */
int tst_is_mounted_rw(const char *path);

/**
 * tst_is_mounted_at_tmpdir() - Check if a path is mounted at the test tmpdir.
 * @path: Path to check.
 *
 * Return: 1 if mounted at tmpdir, 0 otherwise.
 */
int tst_is_mounted_at_tmpdir(const char *path);

/**
 * tst_clear_device() - Wipe filesystem signatures from a block device.
 * @dev: Device path.
 *
 * Clears the first few blocks of the device so that mkfs utilities do not
 * refuse to reformat it. Called automatically by tst_mkfs().
 *
 * Return: 0 on success, -1 on failure.
 */
int tst_clear_device(const char *dev);

/**
 * tst_find_free_loopdev() - Find an unused loop device.
 * @path: If non-NULL, filled with the loop device path.
 * @path_len: Size of @path buffer.
 *
 * Return: The loop device minor number, or -1 if none is free.
 */
int tst_find_free_loopdev(char *path, size_t path_len);

/**
 * tst_attach_device() - Attach a file to a loop device.
 * @dev_path: Path to the loop device (e.g. /dev/loop0).
 * @file_path: Path to a file (e.g. disk.img).
 *
 * Return: Zero on success, non-zero otherwise.
 */
int tst_attach_device(const char *dev_path, const char *file_path);

/**
 * tst_get_device_size() - Get device size in megabytes.
 * @dev_path: Device path.
 *
 * Return: Device size in MB.
 */
uint64_t tst_get_device_size(const char *dev_path);

/**
 * tst_detach_device_by_fd() - Detach a file from a loop device using an fd.
 * @dev_path: Path to the loop device (e.g. /dev/loop0).
 * @dev_fd: Open fd for the loop device; set to -1 on completion.
 *
 * The @dev_fd must be the last file descriptor opened for the device.
 *
 * Return: Zero on success, non-zero otherwise.
 */
int tst_detach_device_by_fd(const char *dev_path, int *dev_fd);

/**
 * tst_detach_device() - Detach a file from a loop device.
 * @dev_path: Path to the loop device (e.g. /dev/loop0).
 *
 * Opens the device internally and calls tst_detach_device_by_fd(). If the
 * device fd is already open, use tst_detach_device_by_fd() instead.
 *
 * Return: Zero on success, non-zero otherwise.
 */
int tst_detach_device(const char *dev_path);

/**
 * tst_dev_sync() - Sync filesystem to avoid deferred IO interference.
 * @fd: Open file descriptor on the filesystem to sync.
 *
 * Should be called before tst_dev_bytes_written() to flush deferred I/O
 * and ensure the first measurement is accurate.
 *
 * Return: 0 on success, -1 on failure.
 */
int tst_dev_sync(int fd);

/**
 * tst_dev_bytes_written() - Get bytes written to a block device since last call.
 * @dev: Test block device path.
 *
 * The call is usually called twice to measure a number of bytes written
 * during a certain operation. However in order to avoid interference from
 * deferred I/O the tst_dev_sync() must be called before we take the first
 * measurement.
 *
 * Return: Number of bytes written since the previous invocation.
 */
unsigned long tst_dev_bytes_written(const char *dev);

/**
 * tst_find_backing_dev() - Find the block device backing a path.
 * @path: Path to look up.
 * @dev: Buffer to store the block device path.
 * @dev_size: Size of @dev buffer.
 */
void tst_find_backing_dev(const char *path, char *dev, size_t dev_size);

/**
 * tst_stat_mount_dev() - Stat the device mounted at a given path.
 * @mnt_path: Mount point path.
 * @st: Stat buffer to fill.
 */
void tst_stat_mount_dev(const char *const mnt_path, struct stat *const st);

/**
 * tst_dev_block_size() - Get physical block size for a device.
 * @path: Path on the filesystem to query.
 *
 * Return: Physical block size in bytes.
 */
int tst_dev_block_size(const char *path);

#endif /* TST_DEVICE_H__ */
