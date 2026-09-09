/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2012 Cyril Hrubis chrubis@suse.cz
 */

#ifndef TST_SAFE_FILE_OPS
#define TST_SAFE_FILE_OPS

#include "safe_file_ops_fn.h"

#define FILE_SCANF(path, fmt, ...) \
	file_scanf(__FILE__, __LINE__, (path), (fmt), ## __VA_ARGS__)

/**
 * SAFE_FILE_SCANF() - Reads formatted data from a file.
 *
 * @path: Path to the file to read.
 * @fmt: scanf format string.
 * @...: Pointers to variables to store parsed values into.
 *
 * Scans formatted data from path. If opening the file fails or the number of
 * conversions does not match the format string, exits with
 * :c:enum:`TBROK <tst_res_flags>`.
 */
#define SAFE_FILE_SCANF(path, fmt, ...) \
	safe_file_scanf(__FILE__, __LINE__, NULL, \
	                (path), (fmt), ## __VA_ARGS__)

/**
 * SAFE_FILE_READ_STR() - Reads a string from a file.
 *
 * Unlike scanf("%s") this function works fine with empty files or files that
 * consist only of white spaces. In such case an empty string is stored into
 * the supplied buffer.
 *
 * It's recommended to use this for various sysfs or procfs files that may be
 * empty.
 *
 * @path: A path to a file.
 * @buf: A buffer to store the string into.
 * @buf_size: A buffer size.
 */
#define SAFE_FILE_READ_STR(path, buf, buf_size) \
	safe_file_read_str(__FILE__, __LINE__, \
	                   (path), (buf), (buf_size))

void safe_file_read_str(const char *file, const int lineno,
                        const char *path, char *buf, size_t buf_size);

#define FILE_LINES_SCANF(path, fmt, ...) \
	file_lines_scanf(__FILE__, __LINE__, NULL, 0,\
			(path), (fmt), ## __VA_ARGS__)

/**
 * SAFE_FILE_LINES_SCANF() - Searches lines of a file for formatted data.
 *
 * @path: Path to the file to read.
 * @fmt: scanf format string to match against each line.
 * @...: Pointers to variables to store parsed values into.
 *
 * Reads lines from path one by one until a line matches all format
 * conversions in fmt. If the file cannot be opened or no line matches,
 * exits with :c:enum:`TBROK <tst_res_flags>`.
 */
#define SAFE_FILE_LINES_SCANF(path, fmt, ...) \
	file_lines_scanf(__FILE__, __LINE__, NULL, 1,\
			(path), (fmt), ## __VA_ARGS__)

#define SAFE_READ_MEMINFO(item) \
       ({long tst_rval; \
        SAFE_FILE_LINES_SCANF("/proc/meminfo", item " %ld", \
                        &tst_rval); \
        tst_rval;})

#define SAFE_READ_PROC_STATUS(pid, item) \
       ({long tst_rval_; \
        char tst_path_[128]; \
        sprintf(tst_path_, "/proc/%d/status", pid); \
        SAFE_FILE_LINES_SCANF(tst_path_, item " %ld", \
                        &tst_rval_); \
        tst_rval_;})

#define FILE_PRINTF(path, fmt, ...) \
	file_printf(__FILE__, __LINE__, \
		    (path), (fmt), ## __VA_ARGS__)

/**
 * SAFE_FILE_PRINTF() - Writes formatted data to a file.
 *
 * @path: Path to the file to write.
 * @fmt: printf format string.
 * @...: Arguments for the format string.
 *
 * Writes formatted output to path. Exits with :c:enum:`TBROK <tst_res_flags>`
 * if the file cannot be opened or written.
 */
#define SAFE_FILE_PRINTF(path, fmt, ...) \
	safe_file_printf(__FILE__, __LINE__, NULL, \
	                 (path), (fmt), ## __VA_ARGS__)

/**
 * SAFE_FILE_VPRINTF() - Formats and writes a va_list argument into a file.
 *
 * Writes formatted output to the file at @path using the format string @fmt
 * and variable arguments @va. Aborts the test with TBROK on failure.
 *
 * @path: A path to a file.
 * @fmt: A printf format string.
 * @va: A variable argument list.
 */
#define SAFE_FILE_VPRINTF(path, fmt, va) \
	safe_file_vprintf(__FILE__, __LINE__, NULL, \
			  (path), (fmt), (va))

/* Same as SAFE_FILE_PRINTF() but returns quietly if the path doesn't exist */
#define SAFE_TRY_FILE_PRINTF(path, fmt, ...) \
	safe_try_file_printf(__FILE__, __LINE__, NULL, \
		(path), (fmt), ## __VA_ARGS__)

/**
 * SAFE_CP() - Copies a file from source to destination.
 *
 * @src: Source file path.
 * @dst: Destination file path.
 *
 * Copies the file from src to dst. Exits with :c:enum:`TBROK <tst_res_flags>`
 * on failure.
 */
#define SAFE_CP(src, dst) \
	safe_cp(__FILE__, __LINE__, NULL, (src), (dst))

/**
 * SAFE_TOUCH() - Creates or updates timestamp on a file.
 *
 * @pathname: Path to the file.
 * @mode: File permissions mode (0 to use default 0666 & ~umask).
 * @times: Array of two struct timespec for atime and mtime (NULL for current time).
 *
 * Creates the file if it does not exist with the specified mode, or updates
 * its access and modification times. Exits with :c:enum:`TBROK <tst_res_flags>`
 * on failure.
 */
#define SAFE_TOUCH(pathname, mode, times) \
	safe_touch(__FILE__, __LINE__, NULL, \
			(pathname), (mode), (times))

/* New API only functions */

/* helper functions to setup overlayfs mountpoint */
void tst_create_overlay_dirs(void);
int tst_mount_overlay(const char *file, const int lineno, int strict);

/**
 * SAFE_MOUNT_OVERLAY() - Mounts overlayfs at OVL_MNT mount point.
 *
 * Creates lower, upper, work, and mnt directories, then mounts overlayfs
 * at OVL_MNT. Exits with :c:enum:`TCONF <tst_res_flags>` if overlayfs is not
 * supported by kernel, or :c:enum:`TBROK <tst_res_flags>` on mount failure.
 */
#define SAFE_MOUNT_OVERLAY() \
	((void) tst_mount_overlay(__FILE__, __LINE__, 1))

#define TST_MOUNT_OVERLAY() \
	(tst_mount_overlay(__FILE__, __LINE__, 0) == 0)

#endif /* TST_SAFE_FILE_OPS */
