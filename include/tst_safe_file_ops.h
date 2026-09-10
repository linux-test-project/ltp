/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2012 Cyril Hrubis chrubis@suse.cz
 */

#ifndef TST_SAFE_FILE_OPS
#define TST_SAFE_FILE_OPS

#include "safe_file_ops_fn.h"

#define FILE_SCANF(path, fmt, ...) \
	file_scanf(__FILE__, __LINE__, (path), (fmt), ## __VA_ARGS__)

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

#define SAFE_CP(src, dst) \
	safe_cp(__FILE__, __LINE__, NULL, (src), (dst))

#define SAFE_TOUCH(pathname, mode, times) \
	safe_touch(__FILE__, __LINE__, NULL, \
			(pathname), (mode), (times))

/* New API only functions */

/* helper functions to setup overlayfs mountpoint */
void tst_create_overlay_dirs(void);
int tst_mount_overlay(const char *file, const int lineno, int strict);

#define SAFE_MOUNT_OVERLAY() \
	((void) tst_mount_overlay(__FILE__, __LINE__, 1))

#define TST_MOUNT_OVERLAY() \
	(tst_mount_overlay(__FILE__, __LINE__, 0) == 0)

#endif /* TST_SAFE_FILE_OPS */
