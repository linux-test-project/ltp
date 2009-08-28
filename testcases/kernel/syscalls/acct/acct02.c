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
 *	acct02.c  -- test acct
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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

/** LTP Port **/
#include "test.h"
#include "usctest.h"

#define FAILED 0
#define PASSED 1

char *TCID = "acct02";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
/**************/

char fname[80];
struct passwd *ltpuser;

/*--------------------------------------------------------------*/
int main(argc, argv)
int argc;
char *argv[];
{

	/* Get the user id "nobody" */
	if ((ltpuser = getpwnam("nobody")) == NULL) {
		tst_resm(TBROK, "nobody not found in /etc/passwd");
		tst_exit();
	}

	/* Switch to "nobody" */
	setuid(ltpuser->pw_uid);

/*--------------------------------------------------------------*/

	/* Attempt to turn off acct as non-root
	 */
	if (acct(NULL) != -1) {
		tst_resm(TBROK|TERRNO,
			 "Non-root attempting to disable acct: didn't fail");
		tst_exit();
	}

	if (errno != EPERM) {
		if (errno == ENOSYS) {
			tst_resm(TCONF,
				 "BSD process accounting is not configured in this kernel.");
			tst_resm(TCONF, "Test will not run.");
			tst_exit();
		} else {
			tst_resm(TBROK|TERRNO,
				 "Non-root acct disable failed as we wanted EPERM errno");
			tst_exit();
		}
	} else
		tst_resm(TPASS, "Received expected error: EPERM");

//-------------------------------------------------
	if (acct("/anystring") != -1) {
		tst_resm(TBROK|TERRNO,
			 "Non-root attempting to enable acct: didn't fail");
		tst_exit();
	}

	if (errno != EPERM) {
		tst_resm(TFAIL|TERRNO,
			 "Non-root acct enable failed as we wanted EPERM errno");
		tst_exit();
	} else
		tst_resm(TPASS, "Received expected error: EPERM");

//-------------------------------------------------

	tst_exit();		/* THIS CALL DOES NOT RETURN - EXITS!!  */
/*--------------------------------------------------------------*/
	return 0;
}
