/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2015-2017 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2017-2022
 */

#ifndef TST_FS_H__
#define TST_FS_H__

/* man 2 statfs or kernel-source/include/uapi/linux/magic.h */
#define TST_BTRFS_MAGIC    0x9123683E
#define TST_NFS_MAGIC      0x6969
#define TST_RAMFS_MAGIC    0x858458f6
#define TST_TMPFS_MAGIC    0x01021994
#define TST_TRACEFS_MAGIC  0x74726163
#define TST_V9FS_MAGIC     0x01021997
#define TST_XFS_MAGIC      0x58465342
#define TST_EXT2_OLD_MAGIC 0xEF51
/* ext2, ext3, ext4 have the same magic number */
#define TST_EXT234_MAGIC   0xEF53
#define TST_MINIX_MAGIC    0x137F
#define TST_MINIX_MAGIC2   0x138F
#define TST_MINIX2_MAGIC   0x2468
#define TST_MINIX2_MAGIC2  0x2478
#define TST_MINIX3_MAGIC   0x4D5A
#define TST_UDF_MAGIC      0x15013346
#define TST_SYSV2_MAGIC    0x012FF7B6
#define TST_SYSV4_MAGIC    0x012FF7B5
#define TST_UFS_MAGIC      0x00011954
#define TST_UFS2_MAGIC     0x19540119
#define TST_F2FS_MAGIC     0xF2F52010
#define TST_NILFS_MAGIC    0x3434
#define TST_EXOFS_MAGIC    0x5DF5
#define TST_OVERLAYFS_MAGIC 0x794c7630
#define TST_FUSE_MAGIC     0x65735546
#define TST_VFAT_MAGIC     0x4d44 /* AKA MSDOS */
#define TST_EXFAT_MAGIC    0x2011BAB0UL

/* fs/bcachefs/bcachefs_format.h */
#define TST_BCACHE_MAGIC		0xca451a4e

#include <stdint.h>

enum tst_fill_access_pattern {
	TST_FILL_BLOCKS,
	TST_FILL_RANDOM
};

enum {
	TST_BYTES = 1,
	TST_KB = 1024,
	TST_MB = 1048576,
	TST_GB = 1073741824,
};

#define OVL_BASE_MNTPOINT        "mntpoint"
#define OVL_LOWER	OVL_BASE_MNTPOINT"/lower"
#define OVL_UPPER	OVL_BASE_MNTPOINT"/upper"
#define OVL_WORK	OVL_BASE_MNTPOINT"/work"
#define OVL_MNT		OVL_BASE_MNTPOINT"/ovl"

/*
 * @path: path is the pathname of any file within the mounted file system
 * @mult: mult should be TST_KB, TST_MB or TST_GB
 * the required free space is calculated by @size * @mult
 */
int tst_fs_has_free_(void (*cleanup)(void), const char *path, uint64_t size,
		     unsigned int mult);

/*
 * Returns filesystem magic for a given path.
 *
 * The expected usage is:
 *
 *      if (tst_fs_type(".") == TST_NFS_MAGIC)
 *		tst_brk(TCONF, "Test not supported on NFS filesystem");
 */
long tst_fs_type_(void (*cleanup)(void), const char *path);

/**
 * tst_fs_type_name() - Returns filesystem name given magic.
 *
 * @f_type: Filesystem magic number.
 *
 * Return: Name of the filesystem as a string.
 */
const char *tst_fs_type_name(long f_type);

/*
 * Try to get maximum number of hard links to a regular file inside the @dir.
 *
 * Note: This number depends on the filesystem @dir is on.
 *
 * The code uses link(2) to create hard links to a single file until it gets
 * EMLINK or creates 65535 links.
 *
 * If limit is hit maximal number of hardlinks is returned and the @dir is
 * filled with hardlinks in format "testfile%i" where i belongs to [0, limit)
 * interval.
 *
 * If no limit is hit (succed to create 65535 without error) or if link()
 * failed with ENOSPC or EDQUOT zero is returned previously created files are
 * removed.
 */
int tst_fs_fill_hardlinks_(void (*cleanup) (void), const char *dir);

/*
 * Try to get maximum number of subdirectories in directory.
 *
 * Note: This number depends on the filesystem @dir is on.
 *
 * The code uses mkdir(2) to create directories in @dir until it gets EMLINK
 * or creates 65535 directories.
 *
 * If limit is hit the maximal number of subdirectories is returned and the
 * @dir is filled with subdirectories in format "testdir%i" where i belongs to
 * [0, limit - 2) interval (because each newly created dir has two links
 * already the '.' and link from parent dir).
 *
 * If no limit is hit or mkdir() failed with ENOSPC or EDQUOT zero is returned
 * previously created directories are removed.
 *
 */
int tst_fs_fill_subdirs_(void (*cleanup) (void), const char *dir);

/*
 * Checks if a given directory contains any entities,
 * returns 1 if directory is empty, 0 otherwise
 */
int tst_dir_is_empty_(void (*cleanup)(void), const char *name, int verbose);

/**
 * tst_get_path() - Searches PATH for program and returns its absolute path.
 *
 * @prog_name: Name of executable to look up.
 * @buf: Buffer to store the absolute path.
 * @buf_len: Size of the buffer in bytes.
 *
 * Return: 0 on success, -1 on failure (command not found or buffer too small).
 */
int tst_get_path(const char *prog_name, char *buf, size_t buf_len);

/**
 * tst_path_exists() - checks if path exists
 *
 * @fmt: A printf-like format used to construct the path.
 * @...: A printf-like parameter list.
 * return: Non-zero if path exists, zero otherwise.
 */
int tst_path_exists(const char *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));

/**
 * tst_fill_fd() - Fills an open file descriptor with pattern.
 *
 * @fd: File descriptor to write to.
 * @pattern: Byte pattern to fill with.
 * @bs: Block size in bytes.
 * @bcount: Number of blocks.
 *
 * Return: 0 on success, non-zero on error.
 */
int tst_fill_fd(int fd, char pattern, size_t bs, size_t bcount);

/**
 * tst_prealloc_size_fd() - Preallocates space in open file descriptor.
 *
 * @fd: File descriptor to preallocate space in.
 * @bs: Block size in bytes.
 * @bcount: Number of blocks.
 *
 * If fallocate() fails, falls back to using tst_fill_fd().
 *
 * Return: 0 on success, non-zero on failure.
 */
int tst_prealloc_size_fd(int fd, size_t bs, size_t bcount);

/**
 * tst_fill_file() - Creates or overwrites a file with pattern.
 *
 * @path: Path to the file.
 * @pattern: Byte pattern to fill with.
 * @bs: Block size in bytes.
 * @bcount: Number of blocks.
 *
 * Return: 0 on success, non-zero on failure.
 */
int tst_fill_file(const char *path, char pattern, size_t bs, size_t bcount);

/**
 * tst_prealloc_file() - Creates file of specified size.
 *
 * @path: Path to the file.
 * @bs: Block size in bytes.
 * @bcount: Number of blocks.
 *
 * Space will be preallocated if supported, otherwise filled with zeroes.
 *
 * Return: 0 on success, non-zero on failure.
 */
int tst_prealloc_file(const char *path, size_t bs, size_t bcount);

enum tst_fs_impl {
	TST_FS_UNSUPPORTED = 0,
	TST_FS_KERNEL = 1,
	TST_FS_FUSE = 2,
};

/**
 * tst_fs_is_supported() - Checks if filesystem is supported.
 *
 * @fs_type: Filesystem name to check support for.
 *
 * Return: TST_FS_KERNEL if driver is in kernel, TST_FS_FUSE if driver is
 * in FUSE, or TST_FS_UNSUPPORTED otherwise.
 */
enum tst_fs_impl tst_fs_is_supported(const char *fs_type);

/**
 * tst_fs_in_skiplist() - Checks if filesystem is in skiplist.
 *
 * @fs_type: Filesystem type to look up.
 * @skiplist: NULL-terminated array of filesystems to skip.
 *
 * Return: 1 if filesystem is in skiplist, 0 otherwise.
 */
int tst_fs_in_skiplist(const char *fs_type, const char *const *skiplist);

/**
 * tst_fill_fs() - Writes to files on given path until ENOSPC.
 *
 * @path: Path to directory on filesystem.
 * @verbose: If non-zero, prints information messages.
 * @pattern: Pattern access type (TST_FILL_BLOCKS or TST_FILL_RANDOM).
 */
void tst_fill_fs(const char *path, int verbose, enum tst_fill_access_pattern pattern);

/**
 * tst_fibmap() - Checks if FIBMAP ioctl is supported.
 *
 * @filename: Path to file to check.
 *
 * Tests need to set .needs_root = 1 in order to avoid EPERM.
 *
 * Return: 0 if FIBMAP is supported, 1 if FIBMAP is not supported.
 */
int tst_fibmap(const char *filename);

#ifdef TST_TEST_H__
/**
 * tst_fs_type() - Returns filesystem magic for a given path.
 *
 * @path: Path to inspect.
 *
 * Return: Filesystem magic number.
 */
static inline long tst_fs_type(const char *path)
{
	return tst_fs_type_(NULL, path);
}

/**
 * tst_fs_has_free() - Checks if filesystem has sufficient free space.
 *
 * @path: Pathname of any file within the mounted filesystem.
 * @size: Space amount.
 * @mult: Multiplier for size (TST_BYTES, TST_KB, TST_MB, or TST_GB).
 *
 * Return: 1 if required free space is available, 0 otherwise.
 */
static inline int tst_fs_has_free(const char *path, uint64_t size,
				  unsigned int mult)
{
	return tst_fs_has_free_(NULL, path, size, mult);
}

/**
 * tst_fs_fill_hardlinks() - Creates maximum number of hard links in directory.
 *
 * @dir: Directory path where hard links are created.
 *
 * Creates hard links to a single file inside dir until EMLINK or 65535 links
 * is reached. If the limit is reached, created files are left in dir and the
 * count is returned. If no limit is reached or link() fails with ENOSPC or
 * EDQUOT, previously created files are removed and 0 is returned.
 *
 * Return: Number of hard links on success, 0 on failure or no limit.
 */
static inline int tst_fs_fill_hardlinks(const char *dir)
{
	return tst_fs_fill_hardlinks_(NULL, dir);
}

/**
 * tst_fs_fill_subdirs() - Creates maximum number of subdirectories in directory.
 *
 * @dir: Directory path where subdirectories are created.
 *
 * Creates subdirectories in dir until EMLINK or 65535 directories is reached.
 * If the limit is reached, created directories are left in dir and the count
 * is returned. If no limit is reached or mkdir() fails with ENOSPC or EDQUOT,
 * previously created directories are removed and 0 is returned.
 *
 * Return: Number of subdirectories on success, 0 on failure or no limit.
 */
static inline int tst_fs_fill_subdirs(const char *dir)
{
	return tst_fs_fill_subdirs_(NULL, dir);
}

/**
 * tst_dir_is_empty() - Checks if directory contains any entries.
 *
 * @name: Path to the directory.
 * @verbose: If non-zero, prints messages about directory contents.
 *
 * Return: 1 if directory is empty (only '.' and '..'), 0 otherwise.
 */
static inline int tst_dir_is_empty(const char *name, int verbose)
{
	return tst_dir_is_empty_(NULL, name, verbose);
}
#else
static inline long tst_fs_type(void (*cleanup)(void), const char *path)
{
	return tst_fs_type_(cleanup, path);
}

static inline int tst_fs_has_free(void (*cleanup)(void), const char *path,
				  uint64_t size, unsigned int mult)
{
	return tst_fs_has_free_(cleanup, path, size, mult);
}

static inline int tst_fs_fill_hardlinks(void (*cleanup)(void), const char *dir)
{
	return tst_fs_fill_hardlinks_(cleanup, dir);
}

static inline int tst_fs_fill_subdirs(void (*cleanup)(void), const char *dir)
{
	return tst_fs_fill_subdirs_(cleanup, dir);
}

static inline int tst_dir_is_empty(void (*cleanup)(void), const char *name, int verbose)
{
	return tst_dir_is_empty_(cleanup, name, verbose);
}
#endif

#endif	/* TST_FS_H__ */
