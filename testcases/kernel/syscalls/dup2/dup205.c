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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

char *TCID = "dup205";
int TST_TOTAL = 1;
extern int Tst_count;
int local_flag;

#define PASSED 1
#define FAILED 0

/*--------------------------------------------------------------------*/
int main(ac, av)
int ac;
char *av[];
{
	int *fildes;
	int ifile;
	char pfilname[40];
	int min;
	int serrno;

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_resm(TBROK, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	 /*NOTREACHED*/}

/*--------------------------------------------------------------------*/
	local_flag = PASSED;

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		min = getdtablesize();	/* get number of files allowed open */

		fildes = (int *)malloc((min + 10) * sizeof(int));
		if (fildes == (int *)0) {
			tst_resm(TBROK, "malloc error");
			tst_exit();
		}

		sprintf(pfilname, "./dup205.%d\n", getpid());
		unlink(pfilname);
		serrno = 0;
		if ((fildes[0] = creat(pfilname, 0666)) == -1) {
			tst_resm(TBROK, "Cannot open first file");
			tst_exit();
		} else {
			fildes[fildes[0]] = fildes[0];
			for (ifile = fildes[0] + 1; ifile < min + 10; ifile++) {
				if ((fildes[ifile] =
				     dup2(fildes[ifile - 1], ifile)) == -1) {
					serrno = errno;
					break;
				} else {
					if (fildes[ifile] != ifile) {
						tst_resm(TFAIL,
							 "got wrong descriptor number back: %d != %d",
							 fildes[ifile], ifile);
						tst_exit();
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
			if ((serrno != EBADF) && (serrno != EMFILE)
			    && (serrno != EINVAL)) {
				tst_resm(TFAIL, "bad errno on dup2 failure");
				local_flag = FAILED;
			}
		}
/*--------------------------------------------------------------------*/
		unlink(pfilname);
		if (ifile > 0)
			close(fildes[ifile - 1]);
		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed.");
		} else {
			tst_resm(TFAIL, "Test failed.");
		}

	}			/* end for */
	tst_exit();
	return 0;
}
