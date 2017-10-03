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
 *
 * NAME
 *	fork10.c
 *
 * DESCRIPTION
 *	Check inheritance of file descriptor by children, they
 *	should all be refering to the same file.
 *
 * ALGORITHM
 *	Child reads several chars and exits.
 *	Parent forks another child, have the child and parent attempt to use
 *	that location
 *
 * USAGE
 *	fork10
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
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include "test.h"
#include "safe_macros.h"

char *TCID = "fork10";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

static char pidbuf[10];
static char fnamebuf[40];

int main(int ac, char **av)
{
	int status, pid, fildes;
	char parchar[2];
	char chilchar[2];

	int lc;

	fildes = -1;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		fildes = SAFE_CREAT(cleanup, fnamebuf, 0600);
		write(fildes, "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 27);
		close(fildes);

		fildes = SAFE_OPEN(cleanup, fnamebuf, 0);

		pid = fork();
		if (pid == -1)
			tst_brkm(TBROK, cleanup, "fork() #1 failed");

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
			pid = fork();
			if (pid == -1)
				tst_brkm(TBROK, cleanup, "fork() #2 failed");

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
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	umask(0);
	TEST_PAUSE;
	tst_tmpdir();

	strcpy(fnamebuf, "fork10.");
	sprintf(pidbuf, "%d", getpid());
	strcat(fnamebuf, pidbuf);
}

static void cleanup(void)
{
	tst_rmdir();
}
