/*
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * NAME
 *	fork08.c
 *
 * DESCRIPTION
 *	Check if the parent's file descriptors are affected by
 *	actions in the child; they should not be.
 *
 * ALGORITHM
 *	Parent opens a file.
 *	Forks a child which closes a file.
 *	Parent forks a second child which attempts to read the (closed)
 *	file.
 *
 * USAGE
 *	fork08
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include "test.h"

char *TCID = "fork08";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

static char pbuf[10];
static char fnamebuf[40];

int main(int ac, char **av)
{
	int status, count, forks, pid1;
	int ch_r_stat;
	FILE *rea, *writ;

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		writ = fopen(fnamebuf, "w");
		if (writ == NULL)
			tst_resm(TFAIL, "failed to fopen file for write");
		rea = fopen(fnamebuf, "r");
		if (rea == NULL)
			tst_resm(TFAIL, "failed to fopen file for read");

		fprintf(writ, "abcdefghijklmnopqrstuv");
		fflush(writ);
		sleep(1);

		if ((getc(rea)) != 'a')
			tst_resm(TFAIL, "getc from read side was confused");

		forks = 0;

forkone:
		++forks;

		pid1 = fork();
		if (pid1 != 0) {
			tst_resm(TINFO, "parent forksval: %d", forks);

			if ((pid1 != (-1)) && (forks < 2))
				goto forkone;
			else if (pid1 < 0)
				tst_resm(TINFO, "Fork failed");
		} else {	/* child */
			/*
			 * If first child close the file descriptor for the
			 * read stream
			 */
			if (forks == 1) {
				if ((fclose(rea)) == -1) {
					tst_resm(TFAIL, "error in first child"
						 " closing fildes");
				}
				_exit(0);
			}

			/*
			 * If second child attempt to read from the file
			 */
			else if (forks == 2) {
				ch_r_stat = getc(rea);
				tst_resm(TINFO, "second child got char: %c",
					 ch_r_stat);
				if (ch_r_stat == 'b') {
					tst_resm(TPASS, "Test passed in child"
						 "number %d", forks);
					exit(0);
				} else if (ch_r_stat == EOF) {
					tst_resm(TFAIL, "Second child got "
						 "EOF");
					exit(-1);
				} else {
					tst_resm(TFAIL, "test failed in child"
						 "no %d", forks);
					exit(-1);
				}
			} else {	/* end of second child */
				tst_resm(TINFO, "forksnumber: %d", forks);
				exit(3);
			}
		}

		for (count = 0; count <= forks; count++) {
			wait(&status);
			tst_resm(TINFO, "exit status of wait "
				 " expected 0 got %d", status);
			status >>= 8;
			if (status == 0)
				tst_resm(TPASS, "parent test PASSED");
			else
				tst_resm(TFAIL, "parent test FAILED");
		}

		tst_resm(TINFO, "Number of processes forked is %d", forks);
		fclose(rea);
		fclose(writ);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	umask(0);
	TEST_PAUSE;
	tst_tmpdir();

	strcpy(fnamebuf, "fork07.");
	sprintf(pbuf, "%d", getpid());
	strcat(fnamebuf, pbuf);
}

static void cleanup(void)
{
	tst_rmdir();
}
