/*
 *   Copyright (c) Dan Kegel 2003
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

/*
 * Test Name: setegid01
 *
 * Test Description:
 *  Verify that setegid does not modify the saved gid or real gid.
 */

#define _GNU_SOURCE 1
#include <pwd.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

char *TCID = "setegid01";
int TST_TOTAL = 1;		/* is this number right? */
int verbose = 0;

option_t options[] = {
	{"v", &verbose, NULL},	/* No argument */
	{NULL, NULL, NULL}	/* NULL required to end array */
};

void help()
{
	printf("  -v       verbose\n");
}

int main(int argc, char **argv)
{
	struct passwd nobody;
	gid_t nobody_gid;
	gid_t cur_rgid, cur_egid, cur_sgid;
	gid_t orig_rgid, orig_egid, orig_sgid;
	char *msg;

	if ((msg = parse_opts(argc, argv, options, help)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "Option parsing error - %s", msg);
		tst_exit();
	}

	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be super/root for this test!");
		tst_exit();
	}

	nobody = *getpwnam("nobody");
	nobody_gid = nobody.pw_gid;

	TEST(getresgid(&orig_rgid, &orig_egid, &orig_sgid));
	if (TEST_RETURN == -1) {
		tst_resm(TBROK, "getresgid() Failed, errno=%d : %s", TEST_ERRNO,
			 strerror(TEST_ERRNO));
		tst_exit();
	}
	if (verbose) {
		printf("getresgid reports rgid %d, egid %d, sgid %d\n",
		       orig_rgid, orig_egid, orig_sgid);
		printf("calling setegid(nobody_gid %d)\n", nobody_gid);
	}
	TEST(setegid(nobody_gid));
	if (TEST_RETURN == -1) {
		tst_resm(TFAIL, "setegid() Failed, errno=%d : %s", TEST_ERRNO,
			 strerror(TEST_ERRNO));
		tst_exit();
	}

	TEST(getresgid(&cur_rgid, &cur_egid, &cur_sgid));
	if (TEST_RETURN == -1) {
		tst_resm(TBROK, "setegid() Failed, errno=%d : %s", TEST_ERRNO,
			 strerror(TEST_ERRNO));
		tst_exit();
	}
	if (verbose) {
		printf("getresgid reports rgid %d, egid %d, sgid %d\n",
		       cur_rgid, cur_egid, cur_sgid);
	}

	/* make sure it at least does what its name says */
	if (nobody_gid != cur_egid) {
		tst_resm(TFAIL,
			 "setegid() Failed functional test: it failed to change the effective gid");
		tst_exit();
	}

	/* SUSv3 says the real group ID and saved set-gid must
	 * remain unchanged by setgid.  See
	 * http://www.opengroup.org/onlinepubs/007904975/functions/setegid.html
	 */
	if (orig_sgid != cur_sgid) {
		tst_resm(TFAIL,
			 "setegid() Failed functional test: it changed the saved set-gid");
		tst_exit();
	}
	if (orig_rgid != cur_rgid) {
		tst_resm(TFAIL,
			 "setegid() Failed functional test: it changed the real gid");
		tst_exit();
	}
	tst_resm(TPASS, "setegid() passed functional test");
	tst_exit();
	return 0;
}
