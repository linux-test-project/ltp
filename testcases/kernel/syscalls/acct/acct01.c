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

/* 12/03/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	acct01.c  -- test acct
 *
 * CALLS
 *	acct
 *
 * ALGORITHM
 *	issue calls to acct and test the returned values against
 *	expected results
 *
 * RESTRICTIONS
 *	This must run root since the acct call may only be done
 *	by root.   Use the TERM flag, to clean up files.
 */

#include <stdio.h>		/* needed by testhead.h         */
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/** LTP Port **/
#include "test.h"
#include "usctest.h"

#define FAILED 0
#define PASSED 1

char *TCID = "acct01";		/* Test program identifier.    */
int TST_TOTAL = 4;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
/**************/

char fname[80];

/*--------------------------------------------------------------*/
int main(argc, argv)
int argc;
char *argv[];
{
	register int i;
	char tmpbuf[100], tmpbuf2[100];

/*--------------------------------------------------------------*/

	/* turn off acct, so we are in a known state
	 */
	if (acct(NULL) == -1) {
		if (errno == ENOSYS) {
			tst_resm(TCONF,
				 "BSD process accounting is not configured in this kernel.");
			tst_resm(TCONF, "Test will not run.");
			tst_exit();
		} else {
			tst_resm(TBROK|TERRNO, "Attempting to disable acct failed");
			tst_exit();
		}
	}

	tst_tmpdir();

	/* now try to use a device, and it should fail */
	if (acct("/dev/null") != -1) {
		tst_resm(TFAIL,
			 "attempting to assign acct file to device: expected failure but got okay return");
		tst_exit();
	} else
		tst_resm(TPASS, "Received expected error: -1");

	/* check the errno, for EACCESS */
	if (errno != EACCES) {
		tst_resm(TFAIL|TERRNO,
			 "Attempt to use non-ordinary file didn't receive EACCESS error");
		tst_exit();
	} else
		tst_resm(TPASS, "Received expected error: EACCESS");

	/* check for error on non-existent file name */
	sprintf(tmpbuf, "./%s.%d", TCID, getpid());

	unlink(tmpbuf);

	if (acct(tmpbuf) != -1) {
		tst_resm(TBROK,
			 "attempt to set acct to non-existent file didn't fail as it should");
		tst_exit();
	}

	/* check the errno */
	if (errno != ENOENT) {
		tst_resm(TFAIL|TERRNO,
			 "Attempt to set acct to non-existent file failed as we wanted ENOENT");
	} else
		tst_resm(TPASS, "Received expected error: ENOENT");

	/* now do a valid acct enable call
	 */
	sprintf(tmpbuf, "./%s.%d", TCID, getpid());

	if ((i = creat(tmpbuf, 0777)) == -1) {
		tst_resm(TBROK|TERRNO, "creat(%s, 0777) failed", tmpbuf);
		tst_exit();
	}

	close(i);

	if (acct(tmpbuf) == -1) {
		tst_resm(TBROK|TERRNO, "acct(%s) failed", tmpbuf);
		tst_exit();
	}

	/* now check for busy error return
	 */

	sprintf(tmpbuf2, "./%s.%d.2", TCID, getpid());

	if (acct(tmpbuf2) != -1) {
		tst_resm(TBROK,
			 "Second try on enabling acct did not fail as it should");
		tst_exit();
	}

	/*
	 * acct() confirms to SVr4, but not POSIX in LINUX as of 03 Dec 2002
	 * In the above case, file doesn't exist and should get ENOENT.
	 */
	if (errno != ENOENT) {
		tst_resm(TFAIL,
			 "Second try on enabling acct failed but with errno= %d expected= %d",
			 errno, ENOENT);
		tst_exit();
	} else
		tst_resm(TPASS, "Received expected error: ENOENT");

	/* now disable accting */
	if (acct(NULL) == -1) {
		tst_resm(TBROK|TERRNO,
			 "Attempt to do final disable of acct failed");
		tst_exit();
	}

	unlink(tmpbuf);

/*--------------------------------------------------------------*/
/* Clean up any files created by test before call to tst_exit.	*/
	tst_rmdir();
	tst_exit();		/* THIS CALL DOES NOT RETURN - EXITS!!  */
/*--------------------------------------------------------------*/
	return 0;
}
