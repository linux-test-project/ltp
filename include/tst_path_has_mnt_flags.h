// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#ifndef TST_PATH_HAS_MNT_FLAGS_H__
#define TST_PATH_HAS_MNT_FLAGS_H__

#ifdef TST_TEST_H__
# define tst_path_has_mnt_flags(...) tst_path_has_mnt_flags_(NULL, __VA_ARGS__)
#else
# define tst_path_has_mnt_flags tst_path_has_mnt_flags_
#endif

/* lib/tst_path_has_mnt_flags.c
 *
 * Check whether a path is on a filesystem that is mounted with
 * specified flags
 * @path: path to file, if path is NULL tst_tmpdir is used.
 * @flags: NULL or NULL terminated array of mount flags
 *
 * Return: 0..n - number of flags matched
 */
int tst_path_has_mnt_flags_(void (*cleanup_fn)(void),
		const char *path, const char *flags[]);

#endif	/* TST_PATH_HAS_MNT_FLAGS_H__ */
