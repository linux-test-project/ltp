/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	rmdir02
 *
 * DESCRIPTION
 *	This test will verify that rmdir(2) fail in
 *      1. ENOTEMPTY
 *      2. ENAMETOOLONG
 *      3. ENOENT
 *      4. ENOTDIR
 *      5. EFAULT
 *      6. EFAULT
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Pause for SIGUSR1 if option specified.
 *		Create temporary directory.
 *
 *	Test:
 *		Loop if the proper options are given.
 *              1. create a directory tstdir1, create a file under it.
 *                 call rmdir(tstdir1), verify the return value is not 0
 *                 and the errno is ENOTEMPTY
 *              2. create a directory with long path,
 *                 call rmdir(tstdir1), verify the return value is not 0
 *                 and the errno is ENAMETOOLONG
 *              3. pass a pathname containing non-exist directory component
 *                 to rmdir() and check the return value and errno (expect
 *                 ENOENT
 *              4. pass a pathname containing a file component
 *                 to rmdir() and check the return value and errno (expect
 *                 ENOTDIR
 *	        5. Attempt to pass an invalid pathname with an address
 *                 pointing outside the address space of the process,
 *                 as the argument to rmdir(), and expect to get EFAULT.
 *		6. Attempt to pass an invalid pathname with NULL
 *                 as the argument to rmdir(), and expect to get EFAULT.
 *
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.
 *
 * USAGE
 *	rmdir02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None.
 */
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();
int create_longpath();
void remove_longpath();
void set_condition();
void do_file_setup(char *);

#define PERMS		0777

char *TCID = "rmdir02";		/* Test program identifier.    */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int exp_enos[] = { ENOTEMPTY, EBUSY, ENAMETOOLONG, ENOENT, ENOTDIR, EFAULT, 0 };

char *bad_addr = 0;

char tstfile[255];
char tstdir1[255];
char tstdir2[255];
char tstdir3[255];
char longname[255];
char longpath[2 * PATH_MAX];
char cwd[255];

struct test_case_t {
	char *dir;
	int error;
	void (*set_cond) (int);
} TC[] = {
	/* The directory is not empty - ENOTEMPTY */
	{
	tstdir1, ENOTEMPTY, set_condition},
	    /* The directory pathname is too long - ENAMETOOLONG */
	{
	longpath, ENAMETOOLONG, set_condition},
	    /* A component of the pathname does not exists - ENOENT */
	{
	tstdir2, ENOENT, set_condition},
	    /* The given argument is not a directory - ENOTDIR */
	{
	tstdir3, ENOTDIR, set_condition},
#if !defined(UCLINUX)
	    /* The argument is illegal - EFAULT */
	{
	(char *)-1, EFAULT, NULL},
#endif
	    /* The argument is illegal - EFAULT */
	{
	NULL, EFAULT, NULL}
};
int TST_TOTAL = (sizeof(TC) / sizeof(*TC));

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	/*
	 * perform global setup for test
	 */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/*
	 * check looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* save current working directory */
		getcwd(cwd, 255);

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			/* make sure we start at the same place */
			chdir(cwd);

			if (TC[i].set_cond != NULL) {
				TC[i].set_cond(i + 1);
			}

			TEST(rmdir(TC[i].dir));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - "
					 "errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s - "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), TC[i].error);
			}
		}
		/* clean up things in case we are looping */
		(void)unlink(tstfile);

		(void)rmdir(tstdir1);
		(void)rmdir(tstdir2);
		(void)rmdir(tstdir3);

	}			/* End for TEST_LOOPING */

	/*
	 * cleanup and exit
	 */
	cleanup();
	 /*NOTREACHED*/ return 0;

}				/* End main */

/*
 * set_condition - set up starting conditions for the individual tests
 */
void set_condition(int num)
{
	int fd;
	switch (num) {
	case 1:
		/* set up for first test */
		sprintf(tstdir1, "./tstdir1_%d", getpid());
		sprintf(tstfile, "%s/tstfile_%d", tstdir1, getpid());

		/* create a directory */
		if (mkdir(tstdir1, PERMS) == -1) {
			tst_brkm(TBROK, cleanup, "mkdir(%s, %#o) Failed",
				 tstdir1, PERMS);
		 /*NOTREACHED*/}

		/* create a file under tstdir1 */
		do_file_setup(tstfile);

		break;
	case 2:
		create_longpath();
		break;
	case 3:
		/* Initialize the test directory name */
		sprintf(tstdir2, "NOSUCHADIR/tstdir2.%d", getpid());

		break;
	case 4:
		/* Initialize the test directory name and file name */
		sprintf(tstdir3, "%s/tstdir3", tstfile);

		/* create a file */
		if ((fd = creat(tstfile, PERMS)) == -1) {
			tst_brkm(TBROK, cleanup, "creat() failed");
		 /*NOTREACHED*/}
		close(fd);
		break;
	default:
		tst_brkm(TBROK, cleanup, "illegal setup case - %d", num);
		break;
	}

}

int create_longpath()
{
	sprintf(longname,
		"abcdefghivwxyzabcdefgmopqrsqrsrsthmopqrsqrsrstijklmnopjklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdabcdefghijklmopqrsqrsrstmnopqrqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyz_%d/",
		getpid());

	chdir(cwd);

	while (strlen(longpath) < PATH_MAX) {
		/*
		 * if the longpath is not long enough
		 * create a sub directory under it
		 */
		if (mkdir(longname, PERMS) == -1) {
			tst_resm(TINFO, "mkdir failed in creae_longpath()");
			break;
		}
		/*
		 * save the path
		 */
		strcat(longpath, longname);

		/*
		 * cd to the sub directory
		 */
		if (chdir(longname) == -1) {
			tst_resm(TINFO, "chdir failed in create_longpath()");
			break;
		}
	}

	/* resume original working directory */
	chdir(cwd);

	return (strlen(longpath) >= PATH_MAX) ? 0 : -1;
}

void remove_longpath()
{
	int len, i, j;
	int path_len;

	chdir(cwd);

	len = strlen(longname);
	path_len = strlen(longpath);

	/*
	 * Since we can't rm directory with long pathname directly,
	 * we remove it's sub directories one by one.
	 */
	for (i = (path_len / len) - 1; i >= 0; i--) {
		for (j = 1; j <= i; j++) {
			if (chdir(longname) == -1) {
				tst_resm(TFAIL,
					 "failed in chdir %s, errno: %d ",
					 longname, errno);
				break;
			}
		}
		if (rmdir(longname) == -1) {
			tst_resm(TFAIL,
				 "failed in clean %s, errno: %d",
				 longname, errno);
			break;
		}
		chdir(cwd);
	}
	/* resume original working directory */
	chdir(cwd);
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	TC[4].dir = bad_addr;
#endif
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	remove_longpath();

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();

	/*
	 * Exit with return code appropriate for results.
	 */
	tst_exit();
}
