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
 *	execve03.c
 *
 * DESCRIPTION
 *	Testcase to check execve sets the following errnos correctly:
 *	1.	ENAMETOOLONG
 *	2.	ENOENT
 *	3.	ENOTDIR
 *	4.	EFAULT
 *	5.	EACCES
 *	6.	ENOEXEC
 *
 * ALGORITHM
 *	1.	Attempt to execve(2) a file whose name is more than
 *		VFS_MAXNAMLEN fails with ENAMETOOLONG.
 *
 *	2.	Attempt to execve(2) a file which doesn't exist fails with
 *		ENOENT.
 *
 *	3.	Attempt to execve(2) a pathname (executabl) comprising of a
 *		directory, which doesn't exist fails with ENOTDIR.
 *
 *	4.	Attempt to execve(2) a filename not within the address space
 *		of the process fails with EFAULT.
 *
 *	5.	Attempt to execve(2) a filename that does not have executable
 *		permission - fails with EACCES.
 *
 *	6.	Attempt to execve(2) a zero length file with executable
 *		permissions - fails with ENOEXEC.
 *
 * USAGE:  <for command-line>
 *  execve03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	test #5 will fail with ETXTBSY not EACCES if the test is run as root
 */

#include <stdio.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>
#include <pwd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char *TCID = "execve03";
int fileHandle = 0;
extern int Tst_count;

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

char *bad_addr = 0;

void setup(void);
void cleanup(void);

int exp_enos[] = { ENAMETOOLONG, ENOENT, ENOTDIR, EFAULT, EACCES, ENOEXEC, 0 };

char long_file[] =
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyz";
char no_dir[] = "testdir";
char test_name3[1000];
char test_name5[1000];
char test_name6[1000];

struct test_case_t {
	char *tname;		/* the command name to pass to execve() */
	int error;
} TC[] = {
	/* the file name is greater than VFS_MAXNAMELEN - ENAMTOOLONG */
	{
	long_file, ENAMETOOLONG},
	    /* the filename does not exist - ENOENT */
	{
	no_dir, ENOENT},
	    /* the path contains a directory name which doesn't exist - ENOTDIR */
	{
	test_name3, ENOTDIR},
#if !defined(UCLINUX)
	    /* the filename isn't part of the process address space - EFAULT */
	{
	(char *)-1, EFAULT},
#endif
	    /* the filename does not have execute permission - EACCES */
	{
	test_name5, EACCES},
	    /* the file is zero length with execute permissions - ENOEXEC */
	{
	test_name6, ENOEXEC}
};

int TST_TOTAL = sizeof(TC) / sizeof(*TC);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* set up the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(execve(TC[i].tname, NULL, NULL));

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
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	char *cwdname = NULL;
	int fd;

	umask(0);

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	ltpuser = getpwnam(nobody_uid);
	if (setgid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setgid failed to "
			 "to set the gid to %d", ltpuser->pw_uid);
		perror("setgid");
	}
	if (setuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	/*
	 * set up a name that should generate an ENOTDIR error
	 */
	if ((cwdname = getcwd(cwdname, 0)) == NULL) {
		tst_brkm(TBROK, cleanup, "could not get currect directory");
	}

	sprintf(test_name5, "%s/fake.%d", cwdname, getpid());

	if ((fileHandle = creat(test_name5, 0444)) == -1) {
		tst_brkm(TBROK, cleanup, "creat(2) FAILED to create temp file");
	}

	sprintf(test_name3, "%s/fake.%d", test_name5, getpid());

	/* creat() and close a zero length file with executeable permission */
	sprintf(test_name6, "%s/execve03.%d", cwdname, getpid());

	if ((fd = creat(test_name6, 0755)) == -1) {
		tst_brkm(TBROK, cleanup, "creat() failed");
	}
	if (close(fd) == -1) {
		tst_brkm(TBROK, cleanup, "close() failed");
	}
#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	TC[3].tname = bad_addr;
#endif
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	close(fileHandle);

	TEST_CLEANUP;

	/* delete the test directory created in setup() */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
