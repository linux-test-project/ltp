/* $Header: /cvsroot/ltp/ltp/tests/Attic/setpgrp01.c,v 1.1 2000/11/15 15:18:33 nstraz Exp $ */

/*
 *  (c) Copyright Cray Research, Inc.  Unpublished Proprietary Information.
 *  All Rights Reserved.
 */
/**********************************************************
 * 
 *    UNICOS Feature Test and Evaluation - Cray Research, Inc.
 * 
 *    TEST IDENTIFIER	: setpgrp01
 * 
 *    EXECUTED BY	: anyone
 * 
 *    TEST TITLE	: Basic test for setpgrp(2)
 * 
 *    PARENT DOCUMENT	: usctpl01
 * 
 *    TEST CASE TOTAL	: 1
 * 
 *    WALL CLOCK TIME	: 1
 * 
 *    CPU TYPES		: ALL
 * 
 *    BINARY LOCATION	: CUTS_BIN/rf_tests/sys
 * 
 *    SOURCE LOCATION	: CUTS_SRC/src/tests/sys
 * 
 *    AUTHOR		: William Roske
 * 
 *    CO-PILOT		: Dave Fenner
 * 
 *    DATE STARTED	: 05/13/92
 * 
 *    INITIAL RELEASE	: UNICOS 7.0
 * 
 *    TEST CASES
 * 
 * 	1.) setpgrp(2) returns...(See Description)
 *	
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 * 
 *    OUTPUT SPECIFICATIONS
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
 * 	The libcuts.a and libsys.a libraries must be included in 
 *	the compilation of this test.
 * 
 *    SPECIAL PROCEDURAL REQUIREMENTS
 * 	None
 * 
 *    INTERCASE DEPENDENCIES
 * 	None
 * 
 *    DETAILED DESCRIPTION
 *	This is a Phase I test for the setpgrp(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	setpgrp(2).
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
 * 
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();


char *TCID="setpgrp01";		/* Test program identifier.    */
int TST_TOTAL=1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int
main(int ac, char **av)
{
    int lc;		/* loop counter */
    char *msg;		/* message returned from parse_opts */
    
    /***************************************************************
     * parse standard options
     ***************************************************************/
    if ( (msg=parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *) NULL ) {
	tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	tst_exit(0);
    }

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
    setup();

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
    for (lc=0; TEST_LOOPING(lc); lc++) {

	/* reset Tst_count in case we are looping. */
	Tst_count=0;

		
	/* 
	 * TEST CASE:
	 *  Call the setpgrp system call
	 */

	/* Call setpgrp(2) */
	TEST(setpgrp( ));
	
	/* check return code */
#ifdef linux
	if ( TEST_RETURN != 0 ) {
#else
	if ( TEST_RETURN <= 0 ) {
#endif
	    tst_resm(TFAIL, "setpgrp -  Call the setpgrp system call failed, errno=%d : %s",
		     TEST_ERRNO, strerror(TEST_ERRNO));
	} else {
	    /***************************************************************
	     * only perform functional verification if flag set (-f not given)
	     ***************************************************************/
	    if ( STD_FUNCTIONAL_TEST ) {
		/* No Verification test, yet... */
		tst_resm(TPASS, "setpgrp -  Call the setpgrp system call returned %d", TEST_RETURN);
	    } 
	}
	

    }	/* End for TEST_LOOPING */

    /***************************************************************
     * cleanup and exit
     ***************************************************************/
    cleanup();

    return 0;
}	/* End main */

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void 
setup()
{
    int pid, status;

    /* capture signals */
    tst_sig(FORK, DEF_HANDLER, cleanup);

    /* Pause if that option was specified */
    TEST_PAUSE;

    /*
     * Make sure current process is NOT a session or pgrp leader
     */

    if (getpgrp() == getpid()) {
	if ((pid = fork()) == -1) {
	    tst_brkm(TBROK, cleanup, "fork() in setup() failed - errno %d",
		     errno);
	}

	if (pid != 0) {	    	    /* parent - sits and waits */
	    wait(&status);
	    exit(WEXITSTATUS(status));
	}

        /* child - continues with test */
    }
}	/* End setup() */


/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void 
cleanup()
{
    /*
     * print timing stats if that option was specified.
     */
    TEST_CLEANUP;

    /* exit with return code appropriate for results */
    tst_exit();
}	/* End cleanup() */


