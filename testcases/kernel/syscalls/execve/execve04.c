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
 *	execve04
 *
 * DESCRIPTION
 *	Testcase to check execve(2) sets errno to EMFILE correctly
 *
 * ALGORITHM
 *	Get the configured file descriptor table size. open(2) files to fill
 *	up the file descriptor table. Once file descriptor table is full,
 *	an attempt to execve(2) any other executable fails with EMFILE.
 *
 * USAGE:  <for command-line>
 *  execve04 -F <test file> [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	Must run test with the -F <test file> option.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <test.h>
#include <usctest.h>

char *TCID = "execve04";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);
void help(void);

int mypid;
char fname[40] = "";
char test_name[80] = "";
int ifile, nfile, first;

int exp_enos[] = {EMFILE, 0};

int Fflag = 0;
char *fname1;

/* for test specific parse_opts options - in this case "-F" */
option_t options[] = {
	{"F:", &Fflag, &fname1},
	{NULL, NULL, NULL}
};

int
main(int ac, char **av)
{
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, options, &help)) != (char *)NULL){
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	if (!Fflag) {
		tst_resm(TWARN, "You must specify a test executable with"
			 "the -F option.");
		tst_resm(TWARN, "Run '%s -h' for option information.", TCID);
		tst_exit();
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(execve(test_name, NULL, NULL));

		if (TEST_ERRNO != EMFILE) {
			tst_resm(TFAIL, "Expected EMFILE got %d", TEST_ERRNO);
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		tst_resm(TPASS, "Expected failure - %d : %s", 
			 TEST_ERRNO, strerror(TEST_ERRNO));

		tst_resm(TINFO, "created %d files, %d allowed", ifile, nfile);
	}
	cleanup();

	/*NOTREACHED*/
	return(0);
}

/*
 * help() - Prints out the help message for the -F option defined
 *          by this test.
 */
void
help()
{
	printf("  -F <test file> : for example, 'execve04 -F test1'\n");
}

/*
 * setup
 * 	performs all ONE TIME setup for this test.
 */
void
setup()
{
	int fd;
	char *pname = NULL;

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/*
	 * Get the current working directory and append to it the name
	 * of the executable file specified with the -F option.
	 * This way when we issue the execve() call later, we get the
	 * correct path.
	 */
	if ((pname = getcwd(pname, 0)) == NULL) {
		tst_brkm(TBROK, tst_exit, "Could not get current directory");
	}
	/*sprintf(test_name, "%s/%s", pname, fname1);*/
	sprintf(test_name, "%s/%s", pname, fname1);
	printf("test_name is %s\n",test_name);
	
	/* make temp dir and cd to it */
	tst_tmpdir();

	nfile = getdtablesize();
	mypid = getpid();

	sprintf(fname, "execve04.%d", mypid);
	unlink(fname);

	if ((first = fd = creat(fname, 0666)) == -1) {
		tst_brkm(TBROK, cleanup, "Cannot open first file");
	}

	close(fd);
	unlink(fname);
	tst_resm(TINFO, "first file is #%d", fd);

	for (ifile = first; ifile <= nfile; ifile++) {
		sprintf(fname, "execve04.%d.%d", ifile, mypid);
		if ((fd = creat(fname, 0666)) == -1) {
			tst_resm(TINFO, "couldn't creat file #%d, expected to "
				 "create %d files", ifile + 1, nfile);
			break;
		}
	}

}

/*
 * cleanup
 * 	performs all ONE TIME cleanup for this test at completion or
 * 	premature exit
 */
void
cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove the tmp dir and all files it */
	for (ifile = first; ifile < nfile; ifile++) {
		sprintf(fname, "execve04.%d.%d", ifile, mypid);
		unlink(fname);
	}
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
