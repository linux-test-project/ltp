/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**************************************************************************
 *
 *    TEST IDENTIFIER	: mount01
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Basic test for mount(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This is a Phase I test for the mount(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Create a mount point.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 *	  Delete the mount point.
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  mount01 [-T type] -D device [-e] [-i n] [-I x] [-p x] [-t]
 *			where,  -T type : specifies the type of filesystem to
 *					  be mounted. Default ext2.
 *				-D device : device to be mounted.
 *				-e   : Turn on errno logging.
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 * RESTRICTIONS
 *	test must run with the -D option
 *	test doesn't support -c option to run it in parallel, as mount
 *	syscall is not supposed to run in parallel.
 *****************************************************************************/

#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

static void help(void);
static void setup(void);
static void cleanup(void);

char *TCID = "mount01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

#define DEFAULT_FSTYPE	"ext2"
#define DIR_MODE	S_IRWXU | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP

static char *Fstype;

static char mntpoint[20];
static char *fstype;
static char *device;
static int Tflag = 0;
static int Dflag = 0;

static option_t options[] = {	/* options supported by mount01 test */
	{"T:", &Tflag, &fstype},	/* -T type of filesystem        */
	{"D:", &Dflag, &device},	/* -D device used for mounting  */
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/* Check for mandatory option of the testcase */
	if (!Dflag)
		tst_brkm(TBROK, NULL,
		    "you must specify the device used for mounting with the -D "
		    "option");

	if (Tflag) {
		Fstype = malloc(strlen(fstype) + 1);
		if (Fstype == NULL) {
			tst_brkm(TBROK|TERRNO, NULL,
			    "malloc - failed to alloc %zd", strlen(fstype));
		}
		strncpy(Fstype, fstype, strlen(fstype) + 1);
	} else {
		Fstype = malloc(strlen(DEFAULT_FSTYPE) + 1);
		if (Fstype == NULL) {
			tst_brkm(TBROK, NULL, "malloc - failed to alloc %zu",
			    strlen(DEFAULT_FSTYPE));
		}
		strncpy(Fstype, DEFAULT_FSTYPE, strlen(DEFAULT_FSTYPE) + 1);
	}

	if (STD_COPIES != 1) {
		tst_resm(TINFO, "-c option has no effect for this testcase - "
			 "%s doesn't allow running more than one instance "
			 "at a time", TCID);
		STD_COPIES = 1;
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/* Call mount(2) */
		TEST(mount(device, mntpoint, Fstype, 0, NULL));

		/* check return code */
		if (TEST_RETURN != 0) {
			tst_resm(TFAIL|TTERRNO, "mount(2) failed");
		} else {
			tst_resm(TPASS, "mount(2) passed ");
			TEST(umount(mntpoint));
			if (TEST_RETURN != 0) {
				tst_brkm(TBROK|TTERRNO, cleanup,
				    "umount(2) failed");
			}
		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();
}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Check whether we are root */
	if (geteuid() != 0) {
		if (Fstype != NULL) {
			free(Fstype);
			Fstype = NULL;
		}
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}

	/* make a temp directory */
	tst_tmpdir();

	/* Unique mount point */
	(void)sprintf(mntpoint, "mnt_%d", getpid());

	if (mkdir(mntpoint, DIR_MODE) < 0) {
		tst_brkm(TBROK|TERRNO, cleanup, "mkdir(%s, %#o) failed",
		    mntpoint, DIR_MODE);
	}

	TEST_PAUSE;

}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{
	if (Fstype) {
		free(Fstype);
		Fstype = NULL;
	}
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	tst_rmdir();
}

/*
 * issue a help message
 */
void help()
{
	printf("-T type	  : specifies the type of filesystem to be mounted."
	       " Default ext2. \n");
	printf("-D device : device used for mounting \n");
}
