// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2026
 */

#ifndef TSO_TMPDIR_H__
#define TSO_TMPDIR_H__

/*
 * Create a unique temporary directory and chdir() to it. It expects the caller
 * to have defined/initialized the TCID/TST_TOTAL global variables.
 * The TESTDIR global variable will be set to the directory that gets used
 * as the testing directory.
 *
 * NOTE: This function must be called BEFORE any activity that would require
 * CLEANUP.  If tst_tmpdir() fails, it cleans up afer itself and calls
 * tst_exit() (i.e. does not return).
 */
void tst_tmpdir(void);

/*
 * Recursively remove the temporary directory created by tst_tmpdir().
 * This function is intended ONLY as a companion to tst_tmpdir().
 */
void tst_rmdir(void);

/* tst_get_tmpdir()
 *
 * Return a copy of the test temp directory as seen by LTP. This is for
 * path-oriented tests like chroot, etc, that may munge the path a bit.
 *
 * FREE VARIABLE AFTER USE IF IT IS REUSED!
 */
char *tst_get_tmpdir(void);

/*
 * Returns path to the test temporary directory root (TMPDIR).
 */
const char *tst_get_tmpdir_root(void);

/*
 * Returns 1 if temp directory was created.
 */
int tst_tmpdir_created(void);

/* declared in tst_tmpdir.c */
const char *tst_get_startwd(void);

#endif	/* TSO_TMPDIR_H__ */
