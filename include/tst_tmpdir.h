// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2024 Cyril Hrubis <chrubis@suse.cz>
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

/**
 * tst_tmpdir_path - Returns a pointer to a tmpdir path.
 *
 * The returned path is allocated and initialized the first time this function is
 * called, each subsequent call will return the same pointer.
 *
 * return: A newly allocated path. The memory is freed automatically at the end
 * of the test. If allocation fails the function calls tst_brk() and
 * exits the test.
 */
char *tst_tmpdir_path(void);

/**
 * tst_tmpdir_genpath - Construct an absolute path pointing to a file inside tmpdir.
 *
 * Constructs a path inside tmpdir i.e. adds a prefix pointing to the current
 * test tmpdir to the string build by the printf-like format.
 *
 * @fmt: A printf-like format string.
 * @...: A printf-like parameter list.
 *
 * return: A newly allocated path. The memory is freed automatically at the end
 * of the test. If allocation fails the function calls tst_brk() and exits the
 * test.
 */
char *tst_tmpdir_genpath(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

/*
 * Make sure nobody uses old API functions in new code.
 */
#ifndef LTPLIB
# define tst_get_tmpdir #error Use tst_tmpdir_path()!
#endif

#endif /* TST_TMPDIR_H__ */
