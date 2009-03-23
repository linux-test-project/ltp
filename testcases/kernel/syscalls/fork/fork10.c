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
 * 	fork10.c
 *
 * DESCRIPTION
 *	Check inheritance of file descriptor by children, they
 * 	should all be refering to the same file.
 *
 * ALGORITHM
 *	Child reads several chars and exits.
 *	Parent forks another child, have the child and parent attempt to use
 *	that location
 *
 * USAGE
 * 	fork10
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
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

char *TCID = "fork10";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);

char pidbuf[10];
char fnamebuf[40];

int main(int ac, char **av)
{
	int status, pid, fildes;
	char parchar[2];
	char chilchar[2];

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	/*
	 * perform global setup for the test
	 */
	setup();

	/*
	 * check looping state if -i option is given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/*
		 * reset Tst_count in case we are looping.
		 */
		Tst_count = 0;

		if ((fildes = creat(fnamebuf, 0600)) < 0) {
			tst_brkm(TBROK, cleanup, "Parent: cannot open %s for "
				 "write, errno = %d", fnamebuf, errno);
		 /*NOTREACHED*/}
		write(fildes, "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 27);
		close(fildes);

		if ((fildes = open(fnamebuf, 0)) == -1) {
			tst_brkm(TBROK, cleanup, "Parent: cannot open %s for "
				 "reading", fnamebuf);
		 /*NOTREACHED*/}

		if ((pid = fork()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() #1 failed");
		 /*NOTREACHED*/}

		if (pid == 0) {	/* child */
			tst_resm(TINFO, "fork child A");
			if (lseek(fildes, 10L, 0) == -1L) {
				tst_resm(TFAIL, "bad lseek by child");
				exit(1);
			}
			exit(0);
		} else {	/* parent */
			wait(&status);

			/* parent starts second child */
			if ((pid = fork()) == -1) {
				tst_brkm(TBROK, cleanup, "fork() #2 failed");
			 /*NOTREACHED*/}

			if (pid == 0) {	/* child */
				if (read(fildes, chilchar, 1) <= 0) {
					tst_resm(TFAIL, "Child can't read "
						 "file");
					exit(1);
				} else {
					if (chilchar[0] != 'K') {
						chilchar[1] = '\n';
						exit(1);
					} else {
						exit(0);
					}
				}
			} else {	/* parent */
				(void)wait(&status);
				if (status >> 8 != 0) {
					tst_resm(TFAIL, "Bad return from "
						 "second child");
					continue;
				}
				/* parent reads */
				if (read(fildes, parchar, 1) <= 0) {
					tst_resm(TFAIL, "Parent cannot read "
						 "file");
					continue;
				} else {
					write(fildes, parchar, 1);
					if (parchar[0] != 'L') {
						parchar[1] = '\n';
						tst_resm(TFAIL, "Test failed");
						continue;
					}
				}
			}
		}
		tst_resm(TPASS, "test 1 PASSED");
	}
	close(fildes);
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup()
{
	/*
	 * capture signals
	 */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	/*
	 * Pause if that option was specified
	 */
	TEST_PAUSE;

	/*
	 * make a temp directory and cd to it
	 */
	tst_tmpdir();

	strcpy(fnamebuf, "fork10.");
	sprintf(pidbuf, "%d", getpid());
	strcat(fnamebuf, pidbuf);
}

/*
 * cleanup() -	performs all ONE TIME cleanup for this test at
 *	        completion or premature exit
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * remove tmp dir and all files in it
	 */
	tst_rmdir();

	tst_exit();
}
