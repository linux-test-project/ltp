// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2020 Martin Doucha <mdoucha@suse.cz>
 */

#ifndef TST_TMPDIR_H__
#define TST_TMPDIR_H__

/**
 * tst_purge_dir - Wipe the content of given directory.
 *
 * Wipe the content of given directory but keep the directory itself.
 *
 * @path: Path of the directory to be wiped.
 */
void tst_purge_dir(const char *path);

#endif /* TST_TMPDIR_H__ */
