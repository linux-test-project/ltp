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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 * 
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 * 
 * http://www.sgi.com 
 * 
 * For further information regarding this notice, see: 
 * 
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */
/**********************************************************
 * 
 *    UNICOS Feature Test and Evaluation - Silicon Graphics, Inc.
 * 
 *    TEST IDENTIFIER	: Qchmod02
 * 
 *    EXECUTED BY	: anyone
 * 
 *    TEST TITLE	: Basic test for chmod(2)
 * 
 *    PARENT DOCUMENT	: usctpl01
 * 
 *    TEST CASE TOTAL	: 8
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
 *    DATE STARTED	: 03/30/92
 * 
 *    INITIAL RELEASE	: UNICOS 7.0
 * 
 *    TEST CASES
 * 
 * 	1.) chmod(2) returns...(See Description)
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
 *	This is a Phase I test for the chmod(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	chmod(2).
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

#include <sys/types.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();
void help();



char *TCID="Qchmod02"; 		/* Test program identifier.    */
int TST_TOTAL=1;    		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char fname[255];
char *buf = "file contents\n";

int Hflag;

/* for test specific parse_opts options */
option_t options[] = {
        { "h",  &Hflag, NULL },         /* -h HELP */
        { NULL, NULL, NULL }
};

int Modes[] = {0, 07, 070, 0700, 0777, 02777, 04777, 06777};


/***********************************************************************
 * Main
 ***********************************************************************/
main(int ac, char **av)
{
    int lc;		/* loop counter */
    char *msg;		/* message returned from parse_opts */
    int ind;
    int mode;
    
    TST_TOTAL = sizeof(Modes) / sizeof(int);

    /***************************************************************
     * parse standard options
     ***************************************************************/
    if ( (msg=parse_opts(ac, av, options)) != (char *) NULL ) {
	tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	tst_exit();
    }

    if(Hflag) {
        help();
        exit(0);
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

	for (ind=0; ind<TST_TOTAL; ind++) { 
	    mode=Modes[ind];

	    /* 
	     * Call chmod(2) with mode argument on fname
	     */
	    TEST(chmod(fname, mode));
	
	    /* check return code */
	    if ( TEST_RETURN == -1 ) {
	        tst_resm(TFAIL, "chmod(%s, %#o) Failed, errno=%d : %s", fname,
		     mode, TEST_ERRNO, strerror(TEST_ERRNO));
	    } else {
	    
	        /***************************************************************
	         * only perform functional verification if flag set (-f not given)
	         ***************************************************************/
	        if ( STD_FUNCTIONAL_TEST ) {
		/* No Verification test, yet... */
		    tst_resm(TPASS, "chmod(%s, %#o) returned %d", fname, 
			mode, TEST_RETURN);
	        } 
		else
		    Tst_count++;
	    }
	}

    }	/* End for TEST_LOOPING */

    /***************************************************************
     * cleanup and exit
     ***************************************************************/
    cleanup();
}	/* End main */

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void 
setup()
{
    int fd;

    /* capture signals */
    tst_sig(NOFORK, DEF_HANDLER, cleanup);

    /* make a temp directory and cd to it */
    tst_tmpdir();

    /* Pause if that option was specified */
    TEST_PAUSE;

    strcat(fname, "tfile");
    if ((fd = open(fname,O_RDWR|O_CREAT,0700)) == -1) {
	tst_brkm(TBROK, cleanup,
		 "open(%s, O_RDWR|O_CREAT,0700) Failed, errno=%d : %s",
		 fname, errno, strerror(errno));
    } else if (write(fd, &buf, strlen(buf)) == -1) {
	tst_brkm(TBROK, cleanup,
		 "write(%s, &buf, strlen(buf)) Failed, errno=%d : %s",
		 fname, errno, strerror(errno));
    } else if (close(fd) == -1) {
	tst_brkm(TBROK, cleanup,
		 "close(%s) Failed, errno=%d : %s",
		 fname, errno, strerror(errno));
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

    /* Remove tmp dir and all files in it */
    tst_rmdir();

    /* exit with return code appropriate for results */
    tst_exit();
}	/* End cleanup() */


/***************************************************************
 * help
 ***************************************************************/
void
help()
{
    char *STD_opts_help();
    printf(STD_opts_help());
    printf("  -h      : print this help message and exit.\n");
}

