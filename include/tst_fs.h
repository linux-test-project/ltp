/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2015-2017 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2017-2022
 */

#ifndef TST_FS_H__
#define TST_FS_H__

/* man 2 statfs or kernel-source/include/linux/magic.h */
#define TST_BTRFS_MAGIC    0x9123683E
#define TST_NFS_MAGIC      0x6969
#define TST_RAMFS_MAGIC    0x858458f6
#define TST_TMPFS_MAGIC    0x01021994
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
int tst_fs_has_free_(void (*cleanup)(void), const char *path, unsigned int size,
		     unsigned int mult);

/*
 * Returns filesystem magick for a given path.
 *
 * The expected usage is:
 *
 *      if (tst_fs_type(cleanup, ".") == TST_NFS_MAGIC) {
 *		tst_brkm(TCONF, cleanup,
 *		         "Test not supported on NFS filesystem");
 *	}
 *
 * Or:
 *
 *	long type;
 *
 *	swtich ((type = tst_fs_type(cleanup, "."))) {
 *	case TST_NFS_MAGIC:
 *	case TST_TMPFS_MAGIC:
 *	case TST_RAMFS_MAGIC:
 *		tst_brkm(TCONF, cleanup, "Test not supported on %s filesystem",
 *		         tst_fs_type_name(type));
 *	break;
 *	}
 */
long tst_fs_type_(void (*cleanup)(void), const char *path);

/*
 * Returns filesystem name given magic.
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

/*
 * Search $PATH for prog_name and fills buf with absolute path if found.
 *
 * Returns -1 on failure, either command was not found or buffer was too small.
 */
int tst_get_path(const char *prog_name, char *buf, size_t buf_len);

/*
 * Fill a file with specified pattern
 * @fd: file descriptor
 * @pattern: pattern
 * @bs: block size
 * @bcount: blocks count
 */
int tst_fill_fd(int fd, char pattern, size_t bs, size_t bcount);

/*
 * Preallocate space in open file. If fallocate() fails, falls back to
 * using tst_fill_fd().
 * @fd: file descriptor
 * @bs: block size
 * @bcount: blocks count
 */
int tst_prealloc_size_fd(int fd, size_t bs, size_t bcount);

/*
 * Creates/ovewrites a file with specified pattern
 * @path: path to file
 * @pattern: pattern
 * @bs: block size
 * @bcount: blocks amount
 */
int tst_fill_file(const char *path, char pattern, size_t bs, size_t bcount);

/*
 * Creates file of specified size. Space will be only preallocated if possible.
 * @path: path to file
 * @bs: block size
 * @bcount: blocks amount
 */
int tst_prealloc_file(const char *path, size_t bs, size_t bcount);

enum tst_fs_impl {
	TST_FS_UNSUPPORTED = 0,
	TST_FS_KERNEL = 1,
	TST_FS_FUSE = 2,
};

/*
 * Returns if filesystem is supported and if driver is in kernel or FUSE.
 *
 * @fs_type A filesystem name to check the support for.
 */
enum tst_fs_impl tst_fs_is_supported(const char *fs_type);

/*
 * Returns NULL-terminated array of kernel-supported filesystems.
 *
 * @skiplist A NULL terminated array of filesystems to skip.
 */
const char **tst_get_supported_fs_types(const char *const *skiplist);

/*
 * Returns 1 if filesystem is in skiplist 0 otherwise.
 *
 * @fs_type A filesystem type to lookup.
 * @skiplist A NULL terminated array of filesystems to skip.
 */
int tst_fs_in_skiplist(const char *fs_type, const char *const *skiplist);

/*
 * Creates and writes to files on given path until write fails with ENOSPC
 */
void tst_fill_fs(const char *path, int verbose);

/*
 * test if FIBMAP ioctl is supported
 */
int tst_fibmap(const char *filename);

#ifdef TST_TEST_H__
static inline long tst_fs_type(const char *path)
{
	return tst_fs_type_(NULL, path);
}

static inline int tst_fs_has_free(const char *path, unsigned int size,
				  unsigned int mult)
{
	return tst_fs_has_free_(NULL, path, size, mult);
}

static inline int tst_fs_fill_hardlinks(const char *dir)
{
	return tst_fs_fill_hardlinks_(NULL, dir);
}

static inline int tst_fs_fill_subdirs(const char *dir)
{
	return tst_fs_fill_subdirs_(NULL, dir);
}

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
				  unsigned int size, unsigned int mult)
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
