/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 * NAME
 *	rename11
 *
 * DESCRIPTION
 *	This test will verify that rename(2) failed in EBUSY
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Create temporary directory.
 *		Pause for SIGUSR1 if option specified.
 *              create the "old" directory 
 *              create the "new" directory 
 *              make the "new" directory to be current work directory
 *
 *	Test:
 *		Loop if the proper options are given.
 *                  rename the "old" to the "new" directory
 *                  verify rename() fail and return EBUSY
 *
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.*
 * USAGE
 *	rename11 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	This tests fails, ie, rename() succeeds, when it is run under
 *	RedHat 7.1  The test will pass when run under RedHat 6.2
 */
#include <unistd.h> 
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

void setup();
void cleanup();
extern void do_file_setup(char *);

char *TCID="rename11";		/* Test program identifier.    */
int TST_TOTAL=1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int exp_enos[]={EBUSY, 0};     /* List must end with 0 */

int fd;
char fdir[255],mdir[255];
char cwd[255];
int bufsize=255;

int
main(int ac, char **av)
{
	int lc;             /* loop counter */
	char *msg;          /* message returned from parse_opts */
	pid_t pid, pid1;

	/*
	 * parse standard options
	 */
	if ((msg=parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/*
	 * perform global setup for test
	 */
	setup();
	
	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);
	
	/*
	 * check looping state if -i option given
	 */
	for (lc=0; TEST_LOOPING(lc); lc++) {
	  
		/* reset Tst_count in case we are looping. */
		Tst_count=0;

		/* rename a directory to current working directory */

		/* Call rename(2) */
		TEST(rename(fdir, mdir));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "rename(%s, %s) succeeded unexpectedly",
				 fdir, mdir);
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		/* check the error no. ,expecting EBUSY */
		if (TEST_ERRNO != EBUSY) {
			tst_resm(TFAIL, "Expected EBUSY got %d", TEST_ERRNO);
		} else {
			tst_resm(TPASS, "rename() returned EBUSY");
		}
	}   /* End for TEST_LOOPING */
	
	/*
	 * cleanup and exit
	 */
	cleanup();
	/*NOTREACHED*/	
	
}       /* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void 
setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE; 

	/* Create a temporary directory and make it current. */
	tst_tmpdir();
	
	getcwd(cwd,bufsize);

	sprintf(fdir,"%s/tdir_%d",cwd,getpid());
	sprintf(mdir,"%s/rndir_%d",cwd,getpid());	

	/* 
	 * create "old" directory
	 */
	if (mkdir(fdir, 00770) == -1) {
		tst_brkm(TBROK, cleanup, "Could not create directory %s", fdir);
		/*NOTREACHED*/
	}

	/* 
	 * create another directory
	 */
	if (mkdir(mdir, 00770) == -1) {
		tst_brkm(TBROK, cleanup, "Could not create directory %s", mdir);
		/*NOTREACHED*/
	}
		
	/* change current work directory to "new" directory */
	if (chdir(mdir) == -1) {
		tst_brkm(TBROK, cleanup, "chdir(%s) failed", mdir);
		/*NOTREACHED*/
	}

	/* 
	 * make sure the "new" directory is considered 
	 * "in use" by the process
	 */
	if (open(".", O_RDONLY) == -1) {
	       tst_brkm(TBROK, cleanup, "open(\".\") failed");
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void 
cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();
	
	/*
	 * Exit with return code appropriate for results.
	 */
	tst_exit();
}
