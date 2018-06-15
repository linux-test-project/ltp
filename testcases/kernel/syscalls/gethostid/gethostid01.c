/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: gethostid01.c,v 1.23 2009/03/23 13:35:42 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: gethostid01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for gethostid(2)
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 1
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: William Roske
 *
 *    CO-PILOT		: Dave Fenner
 *
 *    DATE STARTED	: 03/30/92
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 * 	1.) gethostid(2) returns...(See Description)
 *
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    DURATION
 * 	Terminates - with frequency and infinite modes.
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    RESOURCES
 * 	None
 *
 *    ENVIRONMENTAL NEEDS
 *      No run-time environmental needs.
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 * 	None
 *
 *    INTERCASE DEPENDENCIES
 * 	None
 *
 *    DETAILED DESCRIPTION
 *	This is a Phase I test for the gethostid(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	gethostid(2).
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * 	History:
 * 	  12/2002 Paul Larson - Added functional test to compare
 * 	  	output from hostid command and gethostid()
 *
 *        01/2003 Robbie Williamson - Added code to handle
 *              distros that add "0x" to beginning of `hostid`
 *              output.
 *
 *   01/31/2006  Marty Ridgeway - Corrected 64 bit check so
 *              the second 64 bit check doesn't clobber the first 64 bit
 *              check
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "test.h"

#define HOSTIDLEN 40
/* Bitmasks for the 64 bit operating system checks */
#define FIRST_64_CHKBIT  0x01
#define SECOND_64_CHKBIT 0x02

void setup();
void cleanup();

char *TCID = "gethostid01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc, i, j;		/* loop counters */
	int bit_64 = 0;
	char *result;
	char name[HOSTIDLEN], name2[HOSTIDLEN], hostid[HOSTIDLEN],
	    hostid2[HOSTIDLEN], *hostid3, hex[2] = "0x";
	char hex_64[8] = "ffffffff";
	FILE *fp;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(gethostid());

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "gethostid failed");
			continue;	/* next loop for MTKERNEL */
		}
		sprintf(hostid, "%08lx", TEST_RETURN);

		if (system("hostid > hostid.x") == -1)
			tst_brkm(TFAIL, cleanup,
				 "system() returned errno %d", errno);
		if ((fp = fopen("hostid.x", "r")) == NULL)
			tst_brkm(TFAIL, cleanup, "fopen failed");
		if (fgets(name, HOSTIDLEN, fp) == NULL)
			tst_brkm(TFAIL, cleanup, "fgets failed");
		fclose(fp);

		name[strlen(name) - 1] = 0;

		if (strstr(hostid, "000000"))
			tst_resm(TCONF, "Host ID has not been set.");

		if (strcmp(name, hostid) == 0) {
			tst_resm(TPASS,
				 "Hostid command and gethostid both report "
				 "hostid is %s", hostid);
		} else {

			/*
			 * Some distros add an "0x" to the front of the
			 * `hostid` output. We compare the first 2
			 * characters of the `hostid` output with "0x",
			 * if it's equal, remove these first 2
			 * characters & re-test. -RW
			 */
			if (name[0] == hex[0] && name[1] == hex[1])
				for (i = 0; i < 38; i++)
					name2[i] = name[i + 2];
			else
				strncpy(name2, name, HOSTIDLEN);

			/*
			 * This code handles situations where ffffffff
			 * is appended. Fixed to not clobber the first
			 * check with the 2nd check MR
			 */

			if (0 == strncmp(hostid, hex_64, 8))
				bit_64 |= FIRST_64_CHKBIT;

			if (0 == strncmp(name2, hex_64, 8))
				bit_64 |= SECOND_64_CHKBIT;

			if (bit_64 & FIRST_64_CHKBIT)
				for (j = 0; j < 8; j++)
					hostid2[j] = hostid[j + 8];
			else
				strncpy(hostid2, hostid,
					strlen(hostid) + 1);

			if (bit_64 & SECOND_64_CHKBIT)
				for (j = 0; j < 9; j++)
					name2[j] = name2[j + 8];

			if ((result = strstr(hostid2, name2)) != NULL) {
				hostid3 = strdup(name2);

				tst_resm(TPASS,
					 "Hostid command reports "
					 "hostid is %s, and gethostid "
					 "reports %s", name2, hostid3);
			} else
				tst_resm(TFAIL,
					 "Hostid command reports "
					 "hostid is %s, but gethostid "
					 "reports %s", name2, hostid2);
		}
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	char path[2048];

	if (tst_get_path("hostid", path, sizeof(path)))
		tst_brkm(TCONF, NULL, "Couldn't find hostid in $PATH");

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();
}

void cleanup(void)
{
	tst_rmdir();

}
