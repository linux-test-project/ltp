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

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>		/* needed by testhead.h         */
#include <stdlib.h>
#include <unistd.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

char *TCID = "acct01";
int TST_TOTAL = 6;

char tmpbuf[80];
int fd;

static void cleanup(void)
{

	if (acct(NULL) == -1)
		tst_resm(TBROK|TERRNO, "acct(NULL) failed");

	tst_rmdir();
}


static void setup(void)
{

	/* 
	 * XXX: FreeBSD says you must always be superuser, but Linux says you
	 * need to have CAP_SYS_PACCT capability.
	 *
	 * Either way, it's better to do this to test out the EPERM
	 * requirement.
	 */
	tst_require_root(NULL);

	tst_tmpdir();

	/* turn off acct, so we are in a known state */
	if (acct(NULL) == -1) {
		if (errno == ENOSYS)
			tst_brkm(TCONF, cleanup,
			    "BSD process accounting is not configured in this "
			    "kernel");
		else
			tst_brkm(TBROK|TERRNO, cleanup, "acct(NULL) failed");
	}
}

int main(int argc, char *argv[])
{
	struct passwd *pwent;

	setup();

	/* EISDIR */
	if (acct("/") == -1 && errno == EISDIR) 
		tst_resm(TPASS, "Failed with EISDIR as expected");
	else
		tst_brkm(TFAIL|TERRNO, cleanup,
		    "didn't fail as expected; expected EISDIR");

	/* EACCES */
	if (acct("/dev/null") == -1 && errno == EACCES)
		tst_resm(TPASS, "Failed with EACCES as expected");
	else
		tst_brkm(TFAIL|TERRNO, cleanup,
		    "didn't fail as expected; expected EACCES");

	/* ENOENT */
	if (acct("/tmp/does/not/exist") == -1 && errno == ENOENT)
		tst_resm(TPASS, "Failed with ENOENT as expected");
	else
		tst_brkm(TBROK|TERRNO, cleanup,
		    "didn't fail as expected; expected ENOENT");

	/* ENOTDIR */
	if (acct("/etc/fstab/") == -1 && errno == ENOTDIR)
		tst_resm(TPASS, "Failed with ENOTDIR as expected");
	else
		tst_brkm(TFAIL|TERRNO, cleanup,
		    "didn't fail as expected; expected ENOTDIR");

	/* EPERM */
	sprintf(tmpbuf, "./%s.%d", TCID, getpid());
	fd = SAFE_CREAT(cleanup, tmpbuf, 0777);
	SAFE_CLOSE(cleanup, fd);

	if (acct(tmpbuf) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "acct failed unexpectedly");

	pwent = SAFE_GETPWNAM(cleanup, "nobody");
	SAFE_SETEUID(cleanup, pwent->pw_uid);

	if (acct(tmpbuf) == -1 && errno == EPERM)
		tst_resm(TPASS, "Failed with EPERM as expected");
	else
		tst_brkm(TBROK|TERRNO, cleanup,
		    "didn't fail as expected; expected EPERM");

	SAFE_SETEUID(cleanup, 0);
	SAFE_UNLINK(cleanup, tmpbuf);

	cleanup();
	tst_exit();
}
