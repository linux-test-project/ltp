/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2012 Cyril Hrubis chrubis@suse.cz
 */

#ifndef TST_SAFE_FILE_OPS
#define TST_SAFE_FILE_OPS

#include "safe_file_ops_fn.h"

#define SAFE_FILE_SCANF(path, fmt, ...) \
	safe_file_scanf(__FILE__, __LINE__, NULL, \
	                (path), (fmt), ## __VA_ARGS__)

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

#define FILE_PRINTF(path, fmt, ...) \
	file_printf(__FILE__, __LINE__, \
		    (path), (fmt), ## __VA_ARGS__)

#define SAFE_FILE_PRINTF(path, fmt, ...) \
	safe_file_printf(__FILE__, __LINE__, NULL, \
	                 (path), (fmt), ## __VA_ARGS__)

#define SAFE_CP(src, dst) \
	safe_cp(__FILE__, __LINE__, NULL, (src), (dst))

#define SAFE_TOUCH(pathname, mode, times) \
	safe_touch(__FILE__, __LINE__, NULL, \
			(pathname), (mode), (times))

#define SAFE_MOUNT_OVERLAY() \
	((void) mount_overlay(__FILE__, __LINE__, 1))

#define TST_MOUNT_OVERLAY() \
	(mount_overlay(__FILE__, __LINE__, 0) == 0)

#endif /* TST_SAFE_FILE_OPS */
