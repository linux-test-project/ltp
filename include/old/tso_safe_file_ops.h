// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2012-2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2026
 */

 /*
   This code helps with file reading/writing files providing scanf/printf like
   interface that opens and closes the file automatically.

   This kind of interface is especially useful for reading/writing values
   from/to pseudo filesystems like procfs or sysfs.
  */

#ifndef SAFE_FILE_OPS
#define SAFE_FILE_OPS

#include "safe_file_ops_fn.h"

#define FILE_SCANF(path, fmt, ...) \
	file_scanf(__FILE__, __LINE__, \
	           (path), (fmt), ## __VA_ARGS__)

#define SAFE_FILE_SCANF(cleanup_fn, path, fmt, ...) \
	safe_file_scanf(__FILE__, __LINE__, (cleanup_fn), \
	                (path), (fmt), ## __VA_ARGS__)

#define FILE_LINES_SCANF(cleanup_fn, path, fmt, ...) \
	file_lines_scanf(__FILE__, __LINE__, (cleanup_fn), 0, \
			(path), (fmt), ## __VA_ARGS__)

#define SAFE_FILE_LINES_SCANF(cleanup_fn, path, fmt, ...) \
	file_lines_scanf(__FILE__, __LINE__, (cleanup_fn), 1, \
			(path), (fmt), ## __VA_ARGS__)

#define FILE_PRINTF(path, fmt, ...) \
	file_printf(__FILE__, __LINE__, \
	            (path), (fmt), ## __VA_ARGS__)

#define SAFE_FILE_PRINTF(cleanup_fn, path, fmt, ...) \
	safe_file_printf(__FILE__, __LINE__, (cleanup_fn), \
	                 (path), (fmt), ## __VA_ARGS__)

#define SAFE_CP(cleanup_fn, src, dst) \
	safe_cp(__FILE__, __LINE__, (cleanup_fn), (src), (dst))

#define SAFE_TOUCH(cleanup_fn, pathname, mode, times) \
	safe_touch(__FILE__, __LINE__, (cleanup_fn), \
			(pathname), (mode), (times))

#endif /* SAFE_FILE_OPS */
