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

/* Pored from SPIE, section2/iosuite/dup1.c, by Airong Zhang */

/*======================================================================
	=================== TESTPLAN SEGMENT ===================
>KEYS:  < dup()
>WHAT:  < Does dup return -1 on the 21st file?
>HOW:   < Create up to _NFILE (20) files and check for -1 return on the
	< next attempt
	< Should check NOFILE as well as _NFILE.  19-Jun-84 Dale.
>BUGS:  <
======================================================================*/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include "test.h"
#include "usctest.h"

char *TCID = "dup06";
int TST_TOTAL = 1;
extern int Tst_count;
int local_flag;

#define PASSED 1
#define FAILED 0

/*--------------------------------------------------------------------*/
int cnt_free_fds(int maxfd)
{
	int freefds = 0;

	for (maxfd--; maxfd >= 0; maxfd--)
		if (fcntl(maxfd, F_GETFD) == -1 && errno == EBADF)
			freefds++;

	return (freefds);
}

int main(ac, av)
int ac;
char *av[];
{
	int *fildes, j;
	int ifile;
	char pfilname[40];
	int min;
	int freefds;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_resm(TBROK, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	 /*NOTREACHED*/}

	/* pick up the nofiles */
	min = getdtablesize();
	freefds = cnt_free_fds(min);
	fildes = (int *)malloc((min + 5) * sizeof(int));
	local_flag = PASSED;

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/*  Initialize fildes[_NFILE+5]  - mailbug # 40805 */
		for (j = 0; j < min + 5; j++)
			fildes[j] = 0;

		sprintf(pfilname, "dup06.%d\n", getpid());
		unlink(pfilname);
		if ((fildes[0] = creat(pfilname, 0666)) == -1) {
			tst_resm(TFAIL, "Cannot open first file");
			local_flag = FAILED;
		} else {
			for (ifile = 1; ifile < min + 5; ifile++) {
				if ((fildes[ifile] =
				     dup(fildes[ifile - 1])) == -1) {
					break;
				}

			}	/* end for */
			if (ifile < freefds) {
				tst_resm(TFAIL, "Not enough files duped");
				local_flag = FAILED;
			} else if (ifile > freefds) {
				tst_resm(TFAIL, "Too many files duped");
				local_flag = FAILED;
			}
		}
/*-----	---------------------------------------------------------------*/
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
