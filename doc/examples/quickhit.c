/*
   This is an example quickhitter test based on tests/link03.c. The comments
   have been changed to explain how the quickhit package can be used
*/
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
 *
 */
/* $Id: quickhit.c,v 1.2 2000/10/10 21:57:51 nstraz Exp $ */
/**********************************************************
 * 
 *    OS Test - Silicon Graphics, Inc.
 * 
 *    TEST IDENTIFIER	: link03
 * 
 *    EXECUTED BY	: anyone
 * 
 *    TEST TITLE	: multi links tests
 * 
 *    PARENT DOCUMENT	: usctpl01
 * 
 *    TEST CASE TOTAL	: 2
 * 
 *    WALL CLOCK TIME	: 1
 * 
 *    CPU TYPES		: ALL
 * 
 *    AUTHOR		: Richard Logan
 * 
 *    CO-PILOT		: William Roske
 * 
 *    DATE STARTED	: 03/31/94
 * 
 *    INITIAL RELEASE	: UNICOS 7.0
 * 
 *    TEST CASES
 * 
 * 	1.) link(2) returns...(See Description)
 *	
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *	-N #links : Use #links links every iteration
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
 *      No run-time environmental needs.
 * 
 *    SPECIAL PROCEDURAL REQUIREMENTS
 * 	None
 * 
 *    INTERCASE DEPENDENCIES
 * 	None
 * 
 *    DETAILED DESCRIPTION
 *	This is a Phase I test for the link(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	link(2).
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
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
 /* test.h and usctest.h are the two header files that are required by the
  * quickhit package.  They contain function and macro declarations which you
  * can use in your test programs
  */
#include "test.h"
#include "usctest.h"

 /* The setup and cleanup functions are basic parts of a test case.  These
  * steps are usually put in separate functions for clarity.  The help function
  * is only needed when you are adding new command line options.
  */
void setup(); 
void help();
void cleanup();

char *TCID="link03";		/* Test program identifier.    */
int TST_TOTAL=2;    		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
extern int Tst_nobuf;

int exp_enos[]={0, 0};

#define BASENAME	"lkfile"

char Basename[255];
char Fname[255];
int Nlinks=0;

/* To add command line options you need to declare a structure to pass to
 * parse_opts().  options is the structure used in this example.  The format is
 * the string that should be added to optstring in getopt(3), an integer that
 * will be used as a flag if the option is given, and a pointer to a string that
 * should receive the optarg parameter from getopt(3).  Here we add a -N
 * option.  Long options are not supported at this time. 
 */
char *Nlinkarg;
int Nflag=0;

/* for test specific parse_opts options */
option_t options[] = {
        { "N:",  &Nflag, &Nlinkarg },   /* -N #links */
        { NULL, NULL, NULL }
};

/***********************************************************************
 * Main
 ***********************************************************************/
int
main(int ac, char **av)
{
    int lc;		/* loop counter */
    char *msg;		/* message returned from parse_opts */
    struct stat fbuf, lbuf;
    int cnt;
    int nlinks;
    char lname[255];

    Tst_nobuf=1;

    /***************************************************************
     * parse standard options
     ***************************************************************/
    /* start off by parsing the command line options.  We provide a function
     * that understands many common options to control looping.  If you are not
     * adding any new options, pass NULL in place of options and &help.
     */
    if ( (msg=parse_opts(ac, av, options, &help)) != (char *) NULL ) {
	tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	tst_exit();
    }

    if ( Nflag ) {
	if (sscanf(Nlinkarg, "%i", &Nlinks) != 1 ) {
	    tst_brkm(TBROK, NULL, "--N option arg is not a number");
	    tst_exit();
	}
	if ( Nlinks > 1000 ) {
	    tst_resm(TWARN, "--N option arg > 1000 - may get errno:%d (EMLINK)",
		EMLINK);
	}
    }

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
    /* Next you should run a setup routine to make sure your environment is
     * sane.
     */
    setup();

    /* set the expected errnos... */
    TEST_EXP_ENOS(exp_enos);

    /***************************************************************
     * check looping state 
     ***************************************************************/
    /* TEST_LOOPING() is a macro that will make sure the test continues
     * looping according to the standard command line args. 
     */
    for (lc=0; TEST_LOOPING(lc); lc++) {

	/* reset Tst_count in case we are looping. */
	Tst_count=0;

	if ( Nlinks )
	    nlinks = Nlinks;
	else
	    /* min of 10 links and max of a 100 links */
	    nlinks = (lc%90)+10;

	for(cnt=1; cnt < nlinks; cnt++) {
	
	    sprintf(lname, "%s%d", Basename, cnt);
            /*
	     *  Call link(2)
	     */
	    /* Use the TEST() macro to wrap your syscalls.  It saves the return
	     * to TEST_RETURN and the errno to TEST_ERRNO
	     */
	    TEST(link(Fname, lname));
	
	    /* check return code */
	    if ( TEST_RETURN == -1 ) {
		/* To gather stats on errnos returned, log the errno */
	        TEST_ERROR_LOG(TEST_ERRNO);
		/* If you determine that testing shouldn't continue, report your
		 * results using tst_brkm().  The remaining test cases will be
		 * marked broken.  TFAIL is the result type for a test failure,
		 * cleanup is the cleanup routine to call, and the rest is your
		 * message in printf form.
		 */
	        tst_brkm(TFAIL, cleanup, "link(%s, %s) Failed, errno=%d : %s",
		     Fname, lname, TEST_ERRNO, strerror(TEST_ERRNO));
	    } 
	}
	    
	/***************************************************************
	 * only perform functional verification if flag set (-f not given)
	 ***************************************************************/
	if ( STD_FUNCTIONAL_TEST ) {
	    stat(Fname, &fbuf);

	    for(cnt=1; cnt < nlinks; cnt++) {
                sprintf(lname, "%s%d", Basename, cnt);

		stat(lname, &lbuf);
		if ( fbuf.st_nlink <= 1 || lbuf.st_nlink <= 1 ||
			(fbuf.st_nlink != lbuf.st_nlink) ) {

		    /* When you have results to report, and testing can
		     * continue, use tst_resm() to record those results.  Use
		     * TFAIL if the test case failed and your message in printf
		     * style.
		     */
		    tst_resm(TFAIL,
			"link(%s, %s[1-%d]) ret %d for %d files, stat values do not match %d %d",
			Fname, Basename, nlinks, TEST_RETURN, nlinks,
			fbuf.st_nlink, lbuf.st_nlink);
		    break;
		}
	    }
	    if ( cnt >= nlinks ) {
		/* Here the test case passed so we use TPASS */
		tst_resm(TPASS,
		    "link(%s, %s[1-%d]) ret %d for %d files, stat linkcounts match %d",
		    Fname, Basename, nlinks, TEST_RETURN, nlinks,
		    fbuf.st_nlink);
	    }
	} 
	else
	    Tst_count++;

	/* Here we clean up after the test case so we can do another iteration.
	 */
	for(cnt=1; cnt < nlinks; cnt++) {
        
            sprintf(lname, "%s%d", Basename, cnt);

	    if (unlink(lname) == -1) {
		tst_res(TWARN, "unlink(%s) Failed, errno=%d : %s",
			Fname, errno, strerror(errno));
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
 * help
 ***************************************************************/
/* The custom help() function is really simple.  Just write your help message to
 * standard out.  Your help function will be called after the standard options
 * have been printed
 */
void
help()
{
    printf("  -N #links : create #links hard links every iteration\n");
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void 
setup()
{
    int fd;

    /* You will want to enable some signal handling so you can capture
     * unexpected signals like SIGSEGV. 
     */
    tst_sig(NOFORK, DEF_HANDLER, cleanup);

    /* Pause if that option was specified */
    /* One cavet that hasn't been fixed yet.  TEST_PAUSE contains the code to
     * fork the test with the -c option.  You want to make sure you do this
     * before you create your temporary directory.
     */
    TEST_PAUSE;

    /* If you are doing any file work, you should use a temporary directory.  We
     * provide tst_tmpdir() which will create a uniquely named temporary
     * directory and cd into it.  You can now create files in the current
     * directory without worrying.
     */
    tst_tmpdir();

    sprintf(Fname,"%s_%d", BASENAME, getpid());
    if ((fd = open(Fname,O_RDWR|O_CREAT,0700)) == -1) {
       tst_brkm(TBROK, cleanup,
		"open(%s, O_RDWR|O_CREAT,0700) Failed, errno=%d : %s",
		Fname, errno, strerror(errno));
    } else if (close(fd) == -1) {
       tst_res(TWARN, "close(%s) Failed, errno=%d : %s",
	       Fname, errno, strerror(errno));
    }
    sprintf(Basename, "%s_%d.", BASENAME, getpid());
}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void 
cleanup()
{
    /*
     * print timing stats if that option was specified.
     * print errno log if that option was specified.
     */
    TEST_CLEANUP;

    /* If you use a temporary directory, you need to be sure you remove it. Use
     * tst_rmdir() to do it automatically.  
     */
    tst_rmdir();

    /* exit with return code appropriate for results */
    tst_exit();
}
