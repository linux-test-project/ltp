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

/* Ported from SPIE, section2/iosuite/open1.c, by Airong Zhang */

/*======================================================================
	=================== TESTPLAN SEGMENT ===================
>KEYS:  < open()
>WHAT:  < Does a read on a file opened with oflag = O_WRONLY  return -1?
	< Does a write on a file opened with oflag = O_RDONLY  return -1?
>HOW:   < Open a file with O_WRONLY, test for -1 to pass
        < Open a file with O_RDONLY, test for -1 to pass
>BUGS:  < DUMMY functions used in BSD
>AUTHOR:< PERENNIAL
======================================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include "test.h"
#include "usctest.h"

char *TCID = "open09";
int TST_TOTAL = 1;
extern int Tst_count;
int local_flag;

#define PASSED 1
#define FAILED 0

char progname[] = "open09()";
char tempfile[40] = "";

/*--------------------------------------------------------------------*/
int main(int ac, char *av[])
{
	int fildes;
	int ret = 0;
	char pbuf[BUFSIZ];

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int fail_count = 0;

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_resm(TBROK, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	 /*NOTREACHED*/}
	tst_tmpdir();
	local_flag = PASSED;
	sprintf(tempfile, "open09.%d", getpid());
/*--------------------------------------------------------------------*/
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		local_flag = PASSED;

		//block0:
		if ((fildes = creat(tempfile, 0600)) == -1) {
			tst_resm(TBROK, "\t\t\tcan't create '%s'", tempfile);
			tst_exit();
		} else {
			close(fildes);
			if ((fildes = open(tempfile, 1)) == -1) {
				tst_resm(TFAIL, "\t\t\topen failed");
				tst_exit();
			}
		}
		ret = read(fildes, pbuf, 1);
		if (ret != -1) {
			tst_resm(TFAIL, "read should not succeed");
			local_flag = FAILED;
		}
		close(fildes);

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block0.");
		} else {
			tst_resm(TFAIL, "Test failed in block0.");
			fail_count++;
		}

		local_flag = PASSED;
	/*--------------------------------------------------------------------*/
		//block1:
		if ((fildes = open(tempfile, 0)) == -1) {
			tst_resm(TFAIL, "\t\t\topen failed");
			local_flag = FAILED;
		} else {
			ret = write(fildes, pbuf, 1);
			if (ret != -1) {
				tst_resm(TFAIL, "writeshould not succeed");
				local_flag = FAILED;
			}
		}
		close(fildes);
		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in block1.");
		} else {
			tst_resm(TFAIL, "Test failed in block1.");
			fail_count++;
		}
	/*--------------------------------------------------------------------*/
		unlink(tempfile);
		tst_rmdir();

		if (fail_count == 0) {
			tst_resm(TPASS, "Test passed.");
		} else {
			tst_resm(TFAIL, "Test failed due to above failures.");
		}
	}			/* end for */
	return 0;
}
