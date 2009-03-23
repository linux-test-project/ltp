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
 * 	fork07.c
 *
 * DESCRIPTION
 *	Check that all children inherit parent's file descriptor
 *
 * ALGORITHM
 * 	Parent opens a file, writes to it; forks Nforks children.
 * 	Each child attempts to read the file then returns.
 * 	Parent reports PASS if all children succeed.
 *
 * USAGE
 * 	fork07
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	07/2002 Limited forking and split "infinite forks" test case to
 *	        fork12.c by Nate Straz
 *
 * RESTRICTIONS
 * 	None
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

char *TCID = "fork07";
int TST_TOTAL = 1;
extern int Tst_count;

void help(void);
void setup(void);
void cleanup(void);

char pbuf[10];
char fnamebuf[40];

char *Nforkarg;
int Nflag = 0;
int Nforks = 0;
int vflag = 0;

option_t options[] = {
	{"N:", &Nflag, &Nforkarg},	/* -N #forks */
	{"v", &vflag, NULL},	/* -v (verbose) */
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int status, forks, pid1;
	int ch_r_stat;
	FILE *rea, *writ;
	int c_pass, c_fail;

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, options, &help)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	if (Nflag) {
		if (sscanf(Nforkarg, "%i", &Nforks) != 1) {
			tst_brkm(TBROK, cleanup,
				 "--N option arg is not a number");
			tst_exit();
		}
	} else {
		Nforks = 100;
	}

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

		if ((writ = fopen(fnamebuf, "w")) == NULL)
			tst_resm(TFAIL, "failed to fopen file for write");
		if ((rea = fopen(fnamebuf, "r")) == NULL)
			tst_resm(TFAIL, "failed to fopen file for read");

		fprintf(writ, "abcdefghijklmnopqrstuv");
		fflush(writ);
		sleep(1);

		if ((getc(rea)) != 'a')
			tst_resm(TFAIL, "getc from read side was confused");

		/* fork off the children */
		tst_resm(TINFO, "Forking %d children", Nforks);
		tst_flush();
		for (forks = 0; forks < Nforks; forks++) {
			if ((pid1 = fork()) == 0) {	/* child */
				ch_r_stat = getc(rea);
#ifdef DEBUG
				tst_resm(TINFO, "Child got char: %c",
					 ch_r_stat);
				tst_resm(TINFO,
					 "integer value of getc in child "
					 "expected %d got %d", 'b', ch_r_stat);
#endif
				if (ch_r_stat == 'b') {
					if (vflag) {
						tst_resm(TINFO,
							 "%6d: read correct character",
							 getpid());
					}
					exit(0);
				} else {
					if (vflag) {
						tst_resm(TINFO,
							 "%6d: read '%c' instead of 'b'",
							 getpid(),
							 (char)ch_r_stat);
					}
					exit(1);
				}
			} else if (pid1 == -1) {
				tst_brkm(TBROK, cleanup,
					 "Failed to fork child %d, %s (%d)",
					 forks + 1, strerror(errno), errno);
				tst_exit();
			}
		}
		tst_resm(TINFO, "Forked all %d children, now collecting",
			 Nforks);

		/* Collect all the kids and see how they did */

		c_pass = c_fail = 0;
		while (wait(&status) > 0) {
			if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
				c_pass++;
			} else {
				c_fail++;
			}
			--forks;
		}
		if (forks == 0) {
			tst_resm(TINFO, "Collected all %d children", Nforks);
			if (c_fail > 0) {
				tst_resm(TFAIL,
					 "%d/%d children didn't read correctly from an inheritted fd",
					 c_fail, Nforks);
			} else {
				tst_resm(TPASS,
					 "%d/%d children read correctly from an inheritted fd",
					 c_pass, Nforks);
			}
		} else if (forks > 0) {
			tst_brkm(TBROK, cleanup,
				 "There should be %d more children to collect!",
				 forks);
		} else {	/* forks < 0 */

			tst_brkm(TBROK, cleanup,
				 "Collected %d more children then I should have!",
				 abs(forks));
		}
	}
	fclose(writ);
	fclose(rea);
	cleanup();

	 /*NOTREACHED*/ return 0;
}

void help()
{
	printf("  -N n    Create n children each iteration\n");
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

	strcpy(fnamebuf, "fork07.");
	sprintf(pbuf, "%d", getpid());
	strcat(fnamebuf, pbuf);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit
 */
void cleanup()
{
	int waitstatus;
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* collect our zombies */
	while (wait(&waitstatus) > 0) ;

	/*
	 * remove tmp dir and all files in it
	 */
	unlink(fnamebuf);
	tst_rmdir();

	tst_exit();
}
