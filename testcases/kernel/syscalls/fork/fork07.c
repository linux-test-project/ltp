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
 * 	Parent opens a file, writes to it; forks processes until
 * 	-1 is returned. Each child attempts to read the file then returns.
 *
 * USAGE
 * 	fork07
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 * 	None
 */

#include <stdio.h>
#include "test.h"
#include "usctest.h"

char *TCID = "fork07";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);

char pbuf[10];
char fnamebuf[40];

main(int ac, char **av)
{
	int status, count, forks, pid1;
	int ch_r_stat;
	FILE *rea, *writ;

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
		/*NOTREACHED*/
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

		fprintf(writ,"abcdefghijklmnopqrstuv") ;
		fflush(writ);
		sleep(1);

		if ((getc(rea)) != 'a') 
			tst_resm(TFAIL, "getc from read side was confused");

		forks = 0;

forkone:
		++forks;
		if ((pid1 = fork()) != 0) {	/* parent */
			if (pid1 > 0) {
				goto forkone;
			} else if (pid1 < 0) {
				tst_resm(TINFO, "last child forked");
			} 
		} else {			/* child */
			ch_r_stat = getc(rea);
#ifdef DEBUG
			tst_resm(TINFO, "Child got char: %c", ch_r_stat);
			tst_resm(TINFO, "integer value of getc in child "
				 "expected %d got %d", 'b', ch_r_stat);
#endif
			if (ch_r_stat != EOF) {	/* for error or EOF */
				tst_resm(TPASS, "test passed in child no %d",
					 forks);
				exit(0);
			} else {
				tst_resm(TFAIL, "Test failed in child no. %d",
					 forks);
				exit(-1);
			}
		}

		/* parent */
		--forks;
		for (count = 0; count <= forks; count++) {
			wait(&status);
#ifdef DEBUG
			tst_resm(TINFO, " exit status of wait "
				 " expected 0 got %d", status);
#endif
			if (status == 0) {
				tst_resm(TPASS, "parent test passed");
			} else {
				tst_resm(TFAIL, "parent test failed");
			}
		}
		tst_resm(TINFO, "Number of processes forked is %d", forks);
	}
	cleanup();

	/*NOTREACHED*/
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void
setup()
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
void
cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * remove tmp dir and all files in it
	 */
	unlink(fnamebuf);
	tst_rmdir();

	tst_exit();
}
