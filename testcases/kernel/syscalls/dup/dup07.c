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

/* Pored from SPIE, section2/iosuite/dup3.c, by Airong Zhang */

			   /*dup3.c */
/*======================================================================
    =================== TESTPLAN SEGMENT ===================
>KEYS:  < dup()
>WHAT:  < Is the access mode the same for both file descriptors?
        < 0: read only?
        < 1: write only?
        < 2: read/write?
>HOW:   < Creat a file with each access mode; dup each file descriptor;
        < stat each file descriptor and compare mode of each pair
>BUGS:  <
======================================================================*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

char *TCID = "dup07";
int TST_TOTAL = 1;
int local_flag;

#define PASSED 1
#define FAILED 0

char testfile[40] = "";

/*--------------------------------------------------------------------*/
int main(int ac, char **av)
{
	struct stat retbuf;
	struct stat dupbuf;
	int rdoret, wroret, rdwret;
	int duprdo, dupwro, duprdwr;

/*--------------------------------------------------------------------*/

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_resm(TBROK, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	 }

	local_flag = PASSED;

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		sprintf(testfile, "dup07.%d", getpid());
		if ((rdoret = creat(testfile, 0444)) == -1) {
			tst_resm(TFAIL, "Unable to creat file '%s'", testfile);
			local_flag = FAILED;
		} else {
			if ((duprdo = dup(rdoret)) == -1) {
				tst_resm(TFAIL, "Unable to dup '%s'", testfile);
				local_flag = FAILED;
			} else {
				fstat(rdoret, &retbuf);
				fstat(duprdo, &dupbuf);
				if (retbuf.st_mode != dupbuf.st_mode) {
					tst_resm(TFAIL,
						 "rdonly and dup do not match");
					local_flag = FAILED;
				}
			}
		}
		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in read mode.");
		} else {
			tst_resm(TFAIL, "Test failed in read mode.");
		}

/*--------------------------------------------------------------------*/

		unlink(testfile);
		if ((wroret = creat(testfile, 0222)) == -1) {
			tst_resm(TFAIL, "Unable to creat file '%s'", testfile);
			local_flag = FAILED;
		} else {
			if ((dupwro = dup(wroret)) == -1) {
				tst_resm(TFAIL, "Unable to dup '%s'", testfile);
				local_flag = FAILED;
			} else {
				fstat(wroret, &retbuf);
				fstat(dupwro, &dupbuf);
				if (retbuf.st_mode != dupbuf.st_mode) {
					tst_resm(TFAIL,
						 "wronly and dup do not match");
					local_flag = FAILED;
				}
			}
		}

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in write mode.");
		} else {
			tst_resm(TFAIL, "Test failed in write mode.");
		}
/*--------------------------------------------------------------------*/
		unlink(testfile);
		if ((rdwret = creat(testfile, 0666)) == -1) {
			tst_resm(TFAIL, "Unable to creat file '%s'", testfile);
			local_flag = FAILED;
		} else {
			if ((duprdwr = dup(rdwret)) == -1) {
				tst_resm(TFAIL, "Unable to dup '%s'", testfile);
				local_flag = FAILED;
			} else {
				fstat(rdwret, &retbuf);
				fstat(duprdwr, &dupbuf);
				if (retbuf.st_mode != dupbuf.st_mode) {
					tst_resm(TFAIL,
						 "rdwr and dup do not match");
					local_flag = FAILED;
				}
			}
		}
		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed in read/write mode.");
		} else {
			tst_resm(TFAIL, "Test failed in read/write mode.");
		}
/*--------------------------------------------------------------------*/
		unlink(testfile);

		if (local_flag == PASSED) {
			tst_resm(TPASS, "Test passed");
		} else {
			tst_resm(TFAIL, "Test failed");
		}

		tst_exit();
	}			/* end for */
	tst_exit();
}