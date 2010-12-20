/*
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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */

/* $Id: tst_tmpdir.c,v 1.14 2009/07/20 10:59:32 vapier Exp $ */

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
 *		Setting the env variable "TDIRECTORY" will override the creation
 *		of a new temp dir.  The directory specified by TDIRECTORY will
 *		be used as the temporary directory, and no removal will be done
 *		in tst_rmdir().
 *
 *	 RETURN VALUE
 *		Neither tst_tmpdir() or tst_rmdir() has a return value.
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "rmobj.h"

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
 * Define function prototypes.
 */
static void tmpdir_cleanup(void);

/*
 * Define global variables.
 */
extern char *TCID;		/* defined/initialized in main() */
static char *TESTDIR = NULL;	/* the directory created */

/*
 * Return a copy of the test temp directory as seen by LTP. This is for
 * path-oriented tests like chroot, etc, that may munge the path a bit.
 *
 * FREE VARIABLE AFTER USE IF IT IS REUSED!
 */
char *
get_tst_tmpdir(void)
{
	/* Smack the user for calling things out of order. */
	if (TESTDIR == NULL)
		tst_brkm(TBROK, NULL, "you must call tst_tmpdir() first");
	return strdup(TESTDIR);
}

/*
 * tst_tmpdir() - Create a unique temporary directory and chdir() to it.
 *		It expects the caller to have defined/initialized the
 *		TCID/TST_TOTAL global variables.  The TESTDIR global
 *		variable will be set to the directory that gets used
 *		as the testing directory.
 *
 *		NOTE: This function must be called BEFORE any activity
 *		that would require CLEANUP.  If tst_tmpdir() fails, it
 *		cleans up afer itself and calls tst_exit() (i.e. does
 *		not return).
 */
void tst_tmpdir(void)
{
	char template[PATH_MAX];
	int  no_cleanup = 0;	/* !0 means TDIRECTORY env var was set */
	char *env_tmpdir;	/* temporary storage for TMPDIR env var */
	/* This is an AWFUL hack to figure out if mkdtemp() is available */
#if defined(__GLIBC_PREREQ) && __GLIBC_PREREQ(2,2)
#define HAVE_MKDTEMP
#endif

	/*
	 * If the TDIRECTORY env variable is not set, a temp dir will be
	 * created.
	 */
	if ((TESTDIR = getenv(TDIRECTORY)) != NULL) {
		/*
		 * The TDIRECTORY env. variable is set, so no temp dir is created.
		 */
		if (mkdir(TESTDIR, DIR_MODE) == -1 && errno != EEXIST) {
			tst_brkm(TBROK, NULL, "mkdir(%s, %o) failed",
			    TESTDIR, DIR_MODE);
		}
		/*
		 * Try to create the directory if it does not exist already;
		 * user might forget to create one before exporting TDIRECTORY.
		 */
		no_cleanup++;
#if UNIT_TEST
		printf("TDIRECTORY env var is set\n");
#endif
	} else {
		/*
		 * Create a template for the temporary directory.  Use the
		 * environment variable TMPDIR if it is available, otherwise
		 * use our default TEMPDIR.
		 */
		if ((env_tmpdir = getenv("TMPDIR"))) {
			snprintf(template, PATH_MAX, "%s/%.3sXXXXXX", env_tmpdir, TCID);
		} else {
			snprintf(template, PATH_MAX, "%s/%.3sXXXXXX", TEMPDIR, TCID);
		}

#ifdef HAVE_MKDTEMP
		/* Make the temporary directory in one shot using mkdtemp. */
		if (mkdtemp(template) == NULL)
			tst_brkm(TBROK|TERRNO, tmpdir_cleanup,
				"%s: mkdtemp(%s) failed", __func__, template);
		if ((TESTDIR = strdup(template)) == NULL) {
			tst_brkm(TBROK|TERRNO, tmpdir_cleanup,
				"%s: strdup(%s) failed", __func__, template);
		}
#else
		int tfd;

		/* Make the template name, then the directory */
		if ((tfd = mkstemp(template)) == -1)
			tst_brkm(TBROK|TERRNO, tmpdir_cleanup,
				"%s: mkstemp(%s) failed", __func__, template);
		if (close(tfd) == -1) {
			tst_brkm(TBROK|TERRNO, tmpdir_cleanup,
				"%s: close() failed", __func__);
		}
		if (unlink(template) == -1) {
			tst_brkm(TBROK|TERRNO, tmpdir_cleanup,
				"%s: unlink(%s) failed", __func__, template);
		}
		if ((TESTDIR = strdup(template)) == NULL) {
			tst_brkm(TBROK|TERRNO, tmpdir_cleanup,
				"%s: strdup(%s) failed", __func__, template);
		}
		if (mkdir(TESTDIR, DIR_MODE)) {
			/*
			 * If we start failing with EEXIST, wrap this section in
			 * a loop so we can try again.
			 *
			 * XXX (garrcoop): why? Hacking around broken
			 * filesystems should not be done.
			 */
			tst_brkm(TBROK|TERRNO, tmpdir_cleanup,
			    "%s: mkdir(%s, %#o) failed",
			    __func__, TESTDIR, DIR_MODE);
		}
#endif

		if (chown(TESTDIR, -1, getgid()) == -1)
			tst_brkm(TBROK|TERRNO, tmpdir_cleanup,
			    "chown(%s, -1, %d) failed", TESTDIR, getgid());
		if (chmod(TESTDIR, DIR_MODE) == -1)
			tst_brkm(TBROK|TERRNO, tmpdir_cleanup,
				"chmod(%s, %#o) failed", TESTDIR, DIR_MODE);
 	}

#if UNIT_TEST
	printf("TESTDIR = %s\n", TESTDIR);
#endif

 	/*
  	 * Change to the temporary directory.  If the chdir() fails, issue
	 * TBROK messages for all test cases, attempt to remove the
	 * directory (if it was created), and exit.  If the removal also
	 * fails, also issue a TWARN message.
	 */
	if (chdir(TESTDIR) == -1) {
		tst_brkm(TBROK|TERRNO, NULL, "%s: chdir(%s) failed",
		    __func__, TESTDIR);

		/* Try to remove the directory */
		if (!no_cleanup && rmdir(TESTDIR) == -1) {
			tst_resm(TWARN|TERRNO, "%s: rmdir(%s) failed",
			    __func__, TESTDIR);
		}

		tmpdir_cleanup();
	}

#if UNIT_TEST
	printf("CWD is %s\n", getcwd(NULL, PATH_MAX));
#endif

}  /* tst_tmpdir() */


/*
 *
 * tst_rmdir() - Recursively remove the temporary directory created by
 *			 tst_tmpdir().  This function is intended ONLY as a
 *			 companion to tst_tmpdir().  If the TDIRECTORY
 *			 environment variable is set, no cleanup will be
 *			 attempted.
 */
void tst_rmdir(void)
{
	struct stat buf1;
	struct stat buf2;
	char current_dir[PATH_MAX];
	char *errmsg;
	char *parent_dir;
	char *tdirectory;

	/*
	 * If the TDIRECTORY env variable is set, this indicates that no
	 * temp dir was created by tst_tmpdir().  Thus no cleanup will be
	 * necessary.
	 */
	if ((tdirectory = getenv(TDIRECTORY)) != NULL) {
#if UNIT_TEST
		printf("\"TDIRECORY\" env variable is set; no cleanup was performed\n");
#endif
		return;
	}

	/*
	 * Check that TESTDIR is not NULL.
	 */
	if (TESTDIR == NULL) {
		tst_resm(TWARN,
		    "%s: TESTDIR was NULL; no removal attempted", __func__);
		return;
	}

	if ((parent_dir = malloc(PATH_MAX)) == NULL) {
		/* Make sure that we exit quickly and noisily. */
		tst_brkm(TBROK|TERRNO, NULL,
		    "%s: malloc(%d) failed", __func__, PATH_MAX);
	}

	/*
	 * Check that the value of TESTDIR is not "*" or "/".  These could
	 * have disastrous effects in a test run by root.
	 */
	/* XXX: a temp directory that's '/' seems stupid and dangerous anyways. */
	if (stat(TESTDIR, &buf1) == 0 && stat("/", &buf2) == 0 &&
	    buf1.st_ino == buf2.st_ino) {
		tst_resm(TWARN, "%s: will not remove /", __func__);
		return;
	}

	/*
	 * XXX: this just reeks of bad programming; all shell characters should
	 * be escaped in invocations of rm(1)/rmdir(1).
	 */
	if (strchr(TESTDIR, '*') != NULL) {
		tst_resm(TWARN, "%s: will not remove *", __func__);
		return;
	}

	/*
	 * Get the directory name of TESTDIR.  If TESTDIR is a relative path,
	 * get full path.
	 */
	if (TESTDIR[0] != '/') {
		if (getcwd(current_dir, PATH_MAX) == NULL)
			strncpy(parent_dir, TESTDIR, sizeof(parent_dir));
		else
			sprintf(parent_dir, "%s/%s", current_dir, TESTDIR);
	} else {
		strcpy(parent_dir, TESTDIR);
	}
	if ((parent_dir = dirname(parent_dir)) == NULL) {
		tst_resm(TWARN|TERRNO, "%s: dirname failed", __func__);
		return;
	}

	/*
	 * Change directory to parent_dir (The dir above TESTDIR).
	 */
	if (chdir(parent_dir) != 0) {
		tst_resm(TWARN|TERRNO,
			 "%s: chdir(%s) failed\nAttempting to remove temp dir "
			 "anyway",
			 __func__, parent_dir);
	}

	/*
	 * Attempt to remove the "TESTDIR" directory, using rmobj().
	 */
	if (rmobj(TESTDIR, &errmsg) == -1)
		tst_resm(TWARN, "%s: rmobj(%s) failed: %s",
			 __func__, TESTDIR, errmsg);

}  /* tst_rmdir() */


/*
 * tmpdir_cleanup(void) - This function is used when tst_tmpdir()
 *			  encounters an error, and must cleanup and exit.
 *			  It prints a warning message via tst_resm(), and
 *			  then calls tst_exit().
 */
static void
tmpdir_cleanup(void)
{
	tst_brkm(TWARN, NULL,
	    "%s: no user cleanup function called before exiting", __func__);
}


#ifdef UNIT_TEST
/****************************************************************************
 * Unit test code: Takes input from stdin and can make the following
 *		 calls: tst_tmpdir(), tst_rmdir().
 ****************************************************************************/
extern int  TST_TOTAL;		/* defined/initialized in main() */

int  TST_TOTAL = 10;
char *TCID = "TESTTCID";

main()
{
	int  option;
	char *chrptr;

	printf("UNIT TEST of tst_tmpdir.c.  Options to try:\n\
		-1 : call tst_exit()\n\
		0 : call tst_tmpdir()\n\
		1 : call tst_rmdir()\n\n");

	while (1) {
		printf("Enter options (-1, 0, 1): ");
		(void)scanf("%d%c", &option, &chrptr);

		switch (option) {
		case -1:
			tst_exit();
			break;

		case 0:
			tst_tmpdir();
			break;

		case 1:
			tst_rmdir();
			break;
		}  /* switch() */
	}  /* while () */
}
#endif  /* UNIT_TEST */
