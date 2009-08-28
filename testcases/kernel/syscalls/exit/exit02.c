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
 * 	exit02.c
 *
 * DESCRIPTION
 *	Check that exit flushes output file buffers and closes files upon
 *	exitting
 *
 * ALGORITHM
 * 	Fork a process that creates a file and writes a few bytes, and
 * 	calls exit WITHOUT calling close(). The parent then reads the
 * 	file.  If everything that was written is present in the file, then
 *	the test passes.
 *
 * USAGE
 * 	exit02
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 * 	None
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

void cleanup(void);
void setup(void);

char *TCID = "exit02";
int TST_TOTAL = 1;
extern int Tst_count;

#define READ  0
#define WRITE 1
#define MODE 0666

char filen[40];

int main(int ac, char **av)
{
	int pid, npid, sig, nsig, exno, nexno, status;
	int filed;
	char wbuf[BUFSIZ], rbuf[BUFSIZ];
	int len, rlen;
	int rval = 0;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();		/* global setup for test */

	/*
	 * The following loop checks looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/*
		 * reset Tst_count in case we are looping.
		 */
		Tst_count = 0;

		strcpy(wbuf, "abcd");
		len = strlen(wbuf);

		exno = sig = 0;

		if ((pid = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "fork() failed");

		if (pid == 0) {	/* child */
			sleep(1);
			if ((filed = creat(filen, MODE)) == -1) {
				tst_resm(TINFO, "creat error: unable to"
					 "open output file");
				exit(2);
			}
			if (write(filed, wbuf, len) != len) {
				tst_resm(TINFO, "write error");
				exit(2);
			}
			exit(exno);
		} else {	/* parent */
			npid = wait(&status);

			if (npid != pid) {
				tst_resm(TFAIL, "wait error: "
					 "unexpected pid returned");
				rval = 1;
			}

			nsig = status % 256;

			/*
			 * to check if the core dump bit has been
			 * set, bit # 7
			 */
			if (nsig >= 128)
				nsig = nsig - 128;

			/*
			 * nsig is the signal number returned by
			 * wait
			 */
			if (nsig != sig) {
				tst_resm(TFAIL, "wait error: unexpected "
					 "signal returned %d", nsig);
				rval = 1;
			}

			/*
			 * nexno is the exit number returned by wait
			 */
			nexno = status / 256;
			if (nexno != exno) {
				tst_resm(TFAIL, "wait error: unexpected exit "
					 "number %d", nexno);
				rval = 1;
			}

			sleep(2);	/* let child's exit close opened file */

			filed = open(filen, O_RDONLY, READ);
			if (filed == -1) {
				tst_resm(TFAIL, "open error: "
					 "unable to open input file");
				rval = 1;
			} else {
				rlen = read(filed, rbuf, len);
				if (len != rlen) {
					tst_resm(TFAIL, "exit error: file "
						 "buffer was not flushed");
					rval = 1;
				} else if (strncmp(rbuf, wbuf, len) != 0) {
					tst_resm(TFAIL, "exit error: file "
						 "buffer was not flushed");
					rval = 1;
				}
			}
			close(filed);
			unlink(filen);
		}
		if (!rval) {
			tst_resm(TPASS, "exit() test PASSED");
		}
	}
	cleanup();
	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - perform all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	sprintf(filen, "tfile_%d", getpid());
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at completion or
 *	       premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	/*
	 * Remove tmp dir and all files in it
	 */
	tst_rmdir();

	/*
	 * exit with return code appropriate for results
	 */
	tst_exit();
}
