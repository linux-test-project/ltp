/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_MKFS_H__
#define TST_MKFS_H__

/*
 * @dev: path to a device
 * @fs_type: filesystem type
 * @fs_opts: NULL or NULL terminated array of extra mkfs options
 * @extra_opts: NULL or NULL terminated array of extra mkfs options
 */
void tst_mkfs_(const char *file, const int lineno, void (cleanup_fn)(void),
	       const char *dev, const char *fs_type,
	       const char *const fs_opts[], const char *const extra_opts[]);

#define SAFE_MKFS(device, fs_type, fs_opts, extra_opts) \
	tst_mkfs_(__FILE__, __LINE__, NULL, device, fs_type, \
		  fs_opts, extra_opts)

#endif	/* TST_MKFS_H__ */
