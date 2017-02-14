/**********************************************************
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *********************************************************/

/**********************************************************
 *
 *	 OS Testing - Silicon Graphics, Inc.
 *
 *	 FUNCTION NAME	  : tst_tmpdir, tst_rmdir
 *
 *	 FUNCTION TITLE	 : Create/remove a testing temp dir
 *
 *	 SYNOPSIS:
 *		void tst_tmpdir();
 *		void tst_rmdir();
 *
 *	 AUTHOR		 : Dave Fenner
 *
 *	 INITIAL RELEASE	: UNICOS 8.0
 *
 *	 DESCRIPTION
 *		tst_tmpdir() is used to create a unique, temporary testing
 *		directory, and make it the current working directory.
 *		tst_rmdir() is used to remove the directory created by
 *		tst_tmpdir().
 *
 *	 RETURN VALUE
 *		Neither tst_tmpdir() or tst_rmdir() has a return value.
 *
 *********************************************************/
#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#include "test.h"
#include "ltp_priv.h"
#include "lapi/futex.h"

/*
 * Define some useful macros.
 */
#define DIR_MODE	(S_IRWXU|S_IRWXG|S_IRWXO)

#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX	MAXPATHLEN
#else
#define PATH_MAX	1024
#endif
#endif

/*
 * Define global variables.
 */
extern char *TCID;		/* defined/initialized in main() */
static char *TESTDIR = NULL;	/* the directory created */

static char test_start_work_dir[PATH_MAX];

/* lib/tst_checkpoint.c */
extern futex_t *tst_futexes;

int tst_tmpdir_created(void)
{
	return TESTDIR != NULL;
}

char *tst_get_tmpdir(void)
{
	if (TESTDIR == NULL) {
		tst_brkm(TBROK, NULL, "you must call tst_tmpdir() first");
		return NULL;
	}

	return strdup(TESTDIR);
}

const char *tst_get_startwd(void)
{
	return test_start_work_dir;
}

static int rmobj(char *obj, char **errmsg)
{
	int ret_val = 0;
	DIR *dir;
	struct dirent *dir_ent;
	char dirobj[PATH_MAX];
	struct stat statbuf;
	static char err_msg[1024];
	int fd;

	fd = open(obj, O_DIRECTORY | O_NOFOLLOW);
	if (fd != -1) {
		close(fd);

		/* Do NOT perform the request if the directory is "/" */
		if (!strcmp(obj, "/")) {
			if (errmsg != NULL) {
				sprintf(err_msg, "Cannot remove /");
				*errmsg = err_msg;
			}
			return -1;
		}

		/* Open the directory to get access to what is in it */
		if ((dir = opendir(obj)) == NULL) {
			if (rmdir(obj) != 0) {
				if (errmsg != NULL) {
					sprintf(err_msg,
						"rmdir(%s) failed; errno=%d: %s",
						obj, errno, tst_strerrno(errno));
					*errmsg = err_msg;
				}
				return -1;
			} else {
				return 0;
			}
		}

		/* Loop through the entries in the directory, removing each one */
		for (dir_ent = (struct dirent *)readdir(dir);
		     dir_ent != NULL; dir_ent = (struct dirent *)readdir(dir)) {

			/* Don't remove "." or ".." */
			if (!strcmp(dir_ent->d_name, ".")
			    || !strcmp(dir_ent->d_name, ".."))
				continue;

			/* Recursively call this routine to remove the current entry */
			sprintf(dirobj, "%s/%s", obj, dir_ent->d_name);
			if (rmobj(dirobj, errmsg) != 0)
				ret_val = -1;
		}

		closedir(dir);

		/* If there were problems removing an entry, don't attempt to
		   remove the directory itself */
		if (ret_val == -1)
			return -1;

		/* Get the link count, now that all the entries have been removed */
		if (lstat(obj, &statbuf) < 0) {
			if (errmsg != NULL) {
				sprintf(err_msg,
					"lstat(%s) failed; errno=%d: %s", obj,
					errno, tst_strerrno(errno));
				*errmsg = err_msg;
			}
			return -1;
		}

		/* Remove the directory itself */
		if (statbuf.st_nlink >= 3) {
			/* The directory is linked; unlink() must be used */
			if (unlink(obj) < 0) {
				if (errmsg != NULL) {
					sprintf(err_msg,
						"unlink(%s) failed; errno=%d: %s",
						obj, errno, tst_strerrno(errno));
					*errmsg = err_msg;
				}
				return -1;
			}
		} else {
			/* The directory is not linked; remove() can be used */
			if (remove(obj) < 0) {
				if (errmsg != NULL) {
					sprintf(err_msg,
						"remove(%s) failed; errno=%d: %s",
						obj, errno, tst_strerrno(errno));
					*errmsg = err_msg;
				}
				return -1;
			}
		}
	} else {
		if (unlink(obj) < 0) {
			if (errmsg != NULL) {
				sprintf(err_msg,
					"unlink(%s) failed; errno=%d: %s", obj,
					errno, tst_strerrno(errno));
				*errmsg = err_msg;
			}
			return -1;
		}
	}

	return 0;
}

void tst_tmpdir(void)
{
	char template[PATH_MAX];
	char *env_tmpdir;
	char *errmsg, *c;

	/*
	 * Create a template for the temporary directory.  Use the
	 * environment variable TMPDIR if it is available, otherwise
	 * use our default TEMPDIR.
	 */
	env_tmpdir = getenv("TMPDIR");
	if (env_tmpdir) {
		c = strchr(env_tmpdir, '/');
		/*
		 * Now we force environment variable TMPDIR to be an absolute
		 * pathname, which dose not make much sense, but it will
		 * greatly simplify code in tst_rmdir().
		 */
		if (c != env_tmpdir) {
			tst_brkm(TBROK, NULL, "You must specify an absolute "
				 "pathname for environment variable TMPDIR");
			return;
		}
		snprintf(template, PATH_MAX, "%s/%.3sXXXXXX", env_tmpdir, TCID);
	} else {
		snprintf(template, PATH_MAX, "%s/%.3sXXXXXX", TEMPDIR, TCID);
	}

	/* Make the temporary directory in one shot using mkdtemp. */
	if (mkdtemp(template) == NULL) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "%s: mkdtemp(%s) failed", __func__, template);
		return;
	}

	if ((TESTDIR = strdup(template)) == NULL) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "%s: strdup(%s) failed", __func__, template);
		return;
	}

	if (chown(TESTDIR, -1, getgid()) == -1) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "chown(%s, -1, %d) failed", TESTDIR, getgid());
		return;
	}

	if (chmod(TESTDIR, DIR_MODE) == -1) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "chmod(%s, %#o) failed", TESTDIR, DIR_MODE);
		return;
	}

	if (getcwd(test_start_work_dir, sizeof(test_start_work_dir)) == NULL) {
		tst_resm(TINFO, "Failed to record test working dir");
		test_start_work_dir[0] = '\0';
	}

	/*
	 * Change to the temporary directory.  If the chdir() fails, issue
	 * TBROK messages for all test cases, attempt to remove the
	 * directory (if it was created), and exit.  If the removal also
	 * fails, also issue a TWARN message.
	 */
	if (chdir(TESTDIR) == -1) {
		tst_resm(TERRNO, "%s: chdir(%s) failed", __func__, TESTDIR);

		/* Try to remove the directory */
		if (rmobj(TESTDIR, &errmsg) == -1) {
			tst_resm(TWARN, "%s: rmobj(%s) failed: %s",
				 __func__, TESTDIR, errmsg);
		}

		tst_exit();
	}
}

void tst_rmdir(void)
{
	char *errmsg;

	/*
	 * Check that TESTDIR is not NULL.
	 */
	if (TESTDIR == NULL) {
		tst_resm(TWARN,
			 "%s: TESTDIR was NULL; no removal attempted",
			 __func__);
		return;
	}

	/*
	 * Unmap the backend file.
	 * This is needed to overcome the NFS "silly rename" feature.
	 */
	if (tst_futexes) {
		msync((void *)tst_futexes, getpagesize(), MS_SYNC);
		munmap((void *)tst_futexes, getpagesize());
	}

	/*
	 * Attempt to remove the "TESTDIR" directory, using rmobj().
	 */
	if (rmobj(TESTDIR, &errmsg) == -1) {
		tst_resm(TWARN, "%s: rmobj(%s) failed: %s",
			 __func__, TESTDIR, errmsg);
	}
}
