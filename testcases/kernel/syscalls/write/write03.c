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
 *	write03.c
 *
 * DESCRIPTION
 *	Testcase to check that write(2) doesn't corrupt a file when it fails
 *
 * ALGORITHM
 *	Create a file for writing, write 100 bytes to it. Then make write(2)
 *	fail with some erroneous parameter, close the fd. Then reopen the
 *	file in RDONLY mode, and read the contents of the file. Compare the
 *	buffers, to see whether they are same.
 *
 * USAGE:  <for command-line>
 *      write03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -e   : Turn on errno logging.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	NONE
 */

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>
#include <sys/mman.h>

/* 0 terminated list of expected errnos */
int exp_enos[] = { 14, 0 };

char *TCID = "write03";
int TST_TOTAL = 1;
extern int Tst_count;

char *bad_addr = 0;

void setup(void);
void cleanup(void);

char filename[100];

#if !defined(UCLINUX)

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	char wbuf[BUFSIZ], rbuf[BUFSIZ];
	int fd;

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	/* global setup */
	setup();

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

//block1:
		tst_resm(TINFO, "Enter Block 1: test to check if write "
			 "corrupts the file when write fails");

		fd = creat(filename, 0644);
		if (fd < 0) {
			tst_resm(TBROK, "creating a new file failed");
			cleanup();
		 /*NOTREACHED*/}

		(void)memset(wbuf, '0', 100);

		if (write(fd, wbuf, 100) == -1) {
			tst_resm(TFAIL, "failed to write to %s", filename);
			cleanup();
		 /*NOTREACHED*/}

		if (write(fd, bad_addr, 100) != -1) {
			tst_resm(TFAIL, "write(2) failed to fail");
			cleanup();
		 /*NOTREACHED*/}
		TEST_ERROR_LOG(errno);
		close(fd);

		if ((fd = open(filename, O_RDONLY)) == -1) {
			tst_resm(TBROK, "open(2) failed, errno: %d", errno);
			cleanup();
		 /*NOTREACHED*/}

		if (read(fd, rbuf, 100) == -1) {
			tst_resm(TBROK, "read(2) failed, errno: %d", errno);
			cleanup();
		 /*NOTREACHED*/}

		if (memcmp(wbuf, rbuf, 100) == 0) {
			tst_resm(TPASS, "failure of write(2) didnot corrupt "
				 "the file");
		} else {
			tst_resm(TFAIL, "failure of write(2) corrupted the "
				 "file");
		}
		tst_resm(TINFO, "Exit block 1");
		close(fd);
	}
	cleanup();
	 /*NOTREACHED*/ return 0;
}

#else

int main()
{
	tst_resm(TINFO, "test is not available on uClinux");
	return 0;
}

#endif /* if !defined(UCLINUX) */

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* Create a unique temporary directory and chdir() to it. */
	tst_tmpdir();

	sprintf(filename, "./write03.%d", getpid());

	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		printf("mmap failed\n");
	}

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	unlink(filename);
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
 /*NOTREACHED*/}
