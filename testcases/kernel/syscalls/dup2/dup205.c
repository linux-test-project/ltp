/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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
 */

/* Ported from SPIE, section2/iosuite/dup6.c, by Airong Zhang */

/*======================================================================
	=================== TESTPLAN SEGMENT ===================
>KEYS:  < dup2()
>WHAT:  < Does dup return -1 on the 21st file?
>HOW:   < Create up to _NFILE files and check for -1 return on the
	< next attempt
	< Should check NOFILE as well as _NFILE.  19-Jun-84 Dale.
>BUGS:  <
======================================================================*/

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "test.h"

char *TCID = "dup205";
int TST_TOTAL = 1;
int *fildes;
int min;
int local_flag;

#define PASSED 1
#define FAILED 0

static void setup(void);
static void cleanup(void);

int main(int ac, char *av[])
{
	int ifile;
	char pfilname[40];
	int serrno;

	int lc;

	ifile = -1;

	tst_parse_opts(ac, av, NULL, NULL);

	local_flag = PASSED;

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		sprintf(pfilname, "./dup205.%d\n", getpid());
		unlink(pfilname);
		serrno = 0;
		if ((fildes[0] = creat(pfilname, 0666)) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "creat failed");
		else {
			fildes[fildes[0]] = fildes[0];
			for (ifile = fildes[0] + 1; ifile < min + 10; ifile++) {
				if ((fildes[ifile] = dup2(fildes[ifile - 1],
							  ifile)) == -1) {
					serrno = errno;
					break;
				} else {
					if (fildes[ifile] != ifile) {
						tst_brkm(TFAIL, cleanup,
							 "got wrong descriptor "
							 "number back (%d != %d)",
							 fildes[ifile], ifile);
					}
				}
			}	/* end for */
			if (ifile < min) {
				tst_resm(TFAIL, "Not enough files duped");
				local_flag = FAILED;
			} else if (ifile > min) {
				tst_resm(TFAIL, "Too many files duped");
				local_flag = FAILED;
			}
			if (serrno != EBADF && serrno != EMFILE &&
			    serrno != EINVAL) {
				tst_resm(TFAIL, "bad errno on dup2 failure");
				local_flag = FAILED;
			}
		}
		unlink(pfilname);
		for (ifile = fildes[0]; ifile < min + 10; ifile++)
			close(fildes[ifile]);
		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed.");
		} else {
			tst_resm(TFAIL, "Test failed.");
		}

	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_tmpdir();

	min = getdtablesize();	/* get number of files allowed open */
	fildes = malloc((min + 10) * sizeof(int));
	if (fildes == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "malloc error");
}

static void cleanup(void)
{
	if (fildes != NULL)
		free(fildes);
	tst_rmdir();
}
