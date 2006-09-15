/* Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
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
 *    TEST IDENTIFIER	  : swapon02
 *
 * 
 *    EXECUTED BY	  : root / superuser
 * 
 *    TEST TITLE	  : Test checking for basic error conditions
 *    		   for swapon(2)
 * 
 *    TEST CASE TOTAL	  : 4 
 * 
 *    AUTHOR	 	  : Aniruddha Marathe <aniruddha.marathe@wipro.com>
 * 
 *    SIGNALS
 * 		 Uses SIGUSR1 to pause before test if option set.
 * 		  (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *		  This test case checks whether swapon(2) system call  returns 
 *		  1. ENOENT when the path exists but is invalid
 *		  2. EINVAL when the path does not exist
 *		  3. EPERM when there more than MAX_SWAPFILES are already in use.
 *		  4. EPERM when user is not a superuser
 *		  
 * 		  Setup:
 *		    Setup signal handling.
 *		    Pause for SIGUSR1 if option specified.
 * 		   1. For checking condition 3, make MAX_SWAPFILES  swapfiles
 *		   2. For testing error on invalid user, change the effective uid
 * 		  		 
 * 		  Test:
 *		    Loop if the proper options are given.
 *		    Execute system call.
 *		    Check return code, if system call fails with errno == expected errno
 *	 	    Issue syscall passed with expected errno
 *		    Otherwise, 
 *		    Issue syscall failed to produce expected errno
 * 
 * 		  Cleanup:
 * 		    Do cleanup for the test.
 * 		    
 * USAGE:  <for command-line>
 *  swapon02 [-e] [-i n] [-I x] [-p x] [-t] [-h] [-f] [-p]
 *  where 
 *		  -e   : Turn on errno logging.
 *		  -i n : Execute test n times.
 *		  -I x : Execute test for x seconds.
 *		  -p   : Pause for SIGUSR1 before starting
 *		  -P x : Pause for x seconds between iterations.
 *		  -t   : Turn on syscall timing.
 *		 		 
 *RESTRICTIONS:
 *Incompatible with kernel versions below 2.1.35.
 *Incompatible if MAX_SWAPFILES definition in later kernels is changed
 * -c option can't be used.
 *
 *CHANGES:
 * 2005/01/01  Add extra check to stop test if swap file is on tmpfs
 *             -Ricky Ng-Adam (rngadam@yahoo.com)
 * 01/02/03  Added fork to handle SIGSEGV generated when the intentional EPERM
 * error for hitting MAX_SWAPFILES is hit. 
 * -Robbie Williamson <robbiew@us.ibm.com>
 *****************************************************************************/

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/swap.h>
#ifdef OLDER_DISTRO_RELEASE
#include <linux/swap.h>
#endif
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <sys/utsname.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"

#ifndef OLDER_DISTRO_RELEASE
#define MAX_SWAPFILES 32 
#endif

static void setup();
static void cleanup();
static int setup01();
static int cleanup01();
static int setup02();
static int setup03();
static int cleanup03();
void handler(int);

char *TCID = "swapon02"; 	 /* Test program identifier.    */
int TST_TOTAL = 4;	 	 /* Total number of test cases. */
extern int Tst_count;	 	 /* Test Case counter for tst_* routines */
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

struct utsname uval;
char *kmachine;

static int swapfiles;	 	 /* Number of swapfiles turned on*/

static int exp_enos[] = {EPERM, EINVAL, ENOENT, 0};

static struct test_case_t {
  char *err_desc; 	  /* error description */
  int  exp_errno;	  /* expected error number*/
  char *exp_errval;	  /* Expected errorvalue string*/
  char *path;	 /* path to swapon */
  int (*setupfunc)();	  /* Test setup function */
  int (*cleanfunc)();	  /* Test cleanup function */
} testcase[] = {
	 {"path does not exist", ENOENT, "ENOENT", "./abcd", NULL,  NULL},
	 {"Invalid path", EINVAL, "EINVAL", "./nofile", setup02, NULL},
	 {"Permission denied: more than MAX_SWAPFILES  swapfiles in use",
	   EPERM, "EPERM", "./swapfilenext", setup03, cleanup03},
	 {"Permission denied", EPERM, "EPERM", "./swapfile01", 
	   setup01, cleanup01}
};

int
main(int ac, char **av)
{
  int lc, i;	 /* loop counter */
  char *msg;	 	  /* message returned from parse_opts */

  /* parse standard options */
  if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL))
      != (char *)NULL) {
    tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
  }

  /* perform global setup for test */
  uname(&uval);
  kmachine = uval.machine;
  setup();

  /* check looping state if -i option given */
  for (lc = 0; TEST_LOOPING(lc); lc++) {
    Tst_count = 0;
    for(i = 0; i < TST_TOTAL; i++) {
      /* reset Tst_count in case we are looping. */
      if(testcase[i].setupfunc && 
	 testcase[i].setupfunc() == -1) {
        tst_resm(TWARN, "Failed to setup test %d."
 	       " Skipping test", i);
	continue;
      } else {
        TEST(swapon(testcase[i].path, 0));
      }
    if(testcase[i].cleanfunc && 
       testcase[i].cleanfunc() == -1) {
      tst_brkm(TBROK, cleanup, "Cleanup failed,"
     	 	      " quitting the test");
    }
    /* check return code */
    if ((TEST_RETURN == -1) && (TEST_ERRNO == testcase[i].exp_errno)) {
      tst_resm(TPASS, "swapon(2) expected failure;"
  	     " Got errno - %s : %s",
	    testcase[i].exp_errval,
	    testcase[i].err_desc);
    } else {
      tst_resm(TFAIL, "swapon(2) failed to produce"
  	     " expected error: %d, errno"
	     ": %s and got %d. "
	     " System reboot after"
	     " execution of LTP"
	     " test suite is"
	     " recommended.",
	    testcase[i].exp_errno,
	    testcase[i].exp_errval,
	    TEST_ERRNO);
      /*If swapfile is turned on, turn it off*/
      if(TEST_RETURN == 0) {
        if(swapoff(testcase[i].path) != 0) {
          tst_resm(TWARN, "Failed to"
 	         " turn off swapfile"
	  	  " swapfile. System" 
		  " reboot after"
		  " execution of LTP"
		  " test suite is"
		  " recommended.");
        }
      }
    }
    TEST_ERROR_LOG(TEST_ERRNO);
   }  /*End of TEST LOOPS*/
 } /*End of TEST LOOPING*/
 /*Clean up and exit*/
 cleanup();

 /*NOTREACHED*/
  return 0;
}  /*End of main*/

/*
 * setup01() - This function sets the user as nobody
 */
int
setup01()
{
		 if((ltpuser = getpwnam(nobody_uid)) == NULL) {
		 		 tst_resm(TWARN, "\"nobody\" user not present. skipping test");
		 		 return -1;
		 }

		 if (seteuid(ltpuser->pw_uid) == -1) {
		 		 tst_resm(TWARN, "seteuid failed to "
		 		 		 "to set the effective uid to %d", ltpuser->pw_uid);
		 		 perror("seteuid");
		 		 return -1;
		 }
		 return 0;		 /* user switched to nobody*/
}

/*
 * cleanup01() - switch back to user root
 */
int
cleanup01()
{
		 if(seteuid(0) == -1) {
		 		 tst_brkm(TBROK, cleanup, "seteuid failed to set uid to root");
		 		 perror("seteuid");
		 		 return -1;
		 }
		 return 0;
}

int
setup02()
{
		 int fd;
		 fd = creat("nofile", S_IRWXU);
		 if(fd == -1)
		 		 tst_resm(TWARN, "Failed to create temporary file");
		 close(fd);
		 return 0;
}

/*
 * setup03() - Determine how many swapfiles already exist in system.
 * determine how many more swapfiles need to be created to test the limit.
 * create and turn on those many swapfiles.
 * Create one more swapfile for test.
 */
int
setup03()
{
    int j, fd;              /*j is loop counter, fd is file descriptor*/
    int pid;                    /* used for fork */
    int *status=NULL;       /* used for fork */
    int res = 0, pagesize = getpagesize();
    int  bs, count;
    char cmd_buffer[100];       /* array to hold command line*/
    char filename[15];      /* array to store new filename*/
#define BUFSZ		 1023
    char buf[BUFSZ + 1];    /* temp buffer for reading /proc/swaps */

    /*Find out how many swapfiles (1 line per entry) already exist*/
    /*This includes the first (header) line*/
    if ((fd = open("/proc/swaps", O_RDONLY)) == -1) {
        tst_resm(TWARN, "Failed to find out existing number of swap"
                 " files");
        exit(1);
    }
    while (1) {
        char *p = buf;
        res = read(fd, buf, BUFSZ);
        if (res < 0) {
            tst_resm(TWARN, "Failed to find out existing number of swap"
                     " files");
            exit(1);
		 }
        buf[res] = '\0';
        while ((p = strchr(p, '\n'))) {
            p++;
            swapfiles++;
        }
        if (res < BUFSZ)
            break;
    }
    close(fd);
    if (swapfiles)
        swapfiles--; /* don't count the /proc/swaps header */

    if (swapfiles < 0) {
        tst_resm(TWARN, "Failed to find existing number of swapfiles");
        exit(1);
    }

    /* Determine how many more files are to be created*/
    swapfiles = MAX_SWAPFILES - swapfiles;
    if (swapfiles > MAX_SWAPFILES) {
        swapfiles = MAX_SWAPFILES;
    }

    /* args for dd */
    if ((strncmp(kmachine, "ia64", 4)) == 0) {
        bs = 1024;
        count = 1024;
    } else if(pagesize ==65536) {
	bs=1048;
	count=655;
    } else {
        bs = 1048;
        count = 40;
    }

    pid=FORK_OR_VFORK();
    if (pid == 0) {
        /*create and turn on remaining swapfiles*/
        for (j = 0; j < swapfiles; j++) {

            /*prepare filename for the iteration*/
            if (sprintf(filename, "swapfile%02d", j+2) < 0) {
                tst_resm(TWARN, "sprintf() failed to create filename");
                exit(1);
            }


            /*prepare the path string for dd command and dd command*/
            if (sprintf(cmd_buffer, "dd if=/dev/zero of=%s bs=%d"
                        " count=%d > tmpfile 2>&1", filename, bs, count) < 0) {
                tst_resm(TWARN, "dd command failed to create file");
                exit(1);
            }
            if (system(cmd_buffer) != 0) {
                tst_resm(TWARN, "sprintf() failed to create swapfiles");
                exit(1);
            }

            /* make the file swapfile*/
            if (sprintf(cmd_buffer, "mkswap %s > tmpfile 2>&1", filename)
                < 0) {
                tst_resm(TWARN, "Failed to make swap %s", filename);
                exit(1);
            }

            if (system(cmd_buffer) != 0) {
                tst_resm(TWARN, "failed to make swap file %s",
                         filename); 
                exit(1);
            }

            /* turn on the swap file*/
            if (swapon(filename, 0) != 0) {
                tst_resm(TWARN, "Failed swapon for file %s"
                         " returned %d", filename);
                /* must cleanup already swapon files */
                cleanup03();
                exit(1);
            }
        }
        tst_exit();
    } else
        waitpid(pid,status,0);
    /*create extra swapfile for testing*/
    if ((strncmp(kmachine, "ia64", 4)) == 0) {
        if (system("dd if=/dev/zero of=swapfilenext bs=1024 count=65536 >tmpfile"
                   " 2>&1") != 0) {
            tst_resm(TWARN, "Failed to create extra file for swap");
            exit(1);
        }
    } else if (pagesize == 65536) {
       if(system("dd if=/dev/zero of=swapfilenext  bs=1048  count=655 > tmpfile"
             " 2>&1") != 0) {
      tst_brkm(TBROK, cleanup, "Failed to create file for swap");
      }
     }else {
        if (system("dd if=/dev/zero of=swapfilenext bs=1048 count=40 > tmpfile"
                   " 2>&1") != 0) {
            tst_resm(TWARN, "Failed to create extra file for swap");
            exit(1);
        }
    }

    if (system("mkswap ./swapfilenext > tmpfle 2>&1") != 0) {
        tst_resm(TWARN, "Failed to make extra swapfile");
        exit(1);
    }

    return 0;
}

/*
 * cleanup03() - clearing all turned on swapfiles
 */ 
int
cleanup03()
{
		 int j;		 		 		 /* loop counter*/
		 char filename[15];

		 for(j = 0; j < swapfiles; j++) {
		 		 if( sprintf(filename, "swapfile%02d", j+2) < 0) {
		 		 		 tst_resm(TWARN, "sprintf() failed to create filename");
		 		 		 tst_resm(TWARN, "Failed to turn off swap files. System"
		 		 		 		 		 " reboot after execution of LTP test"
		 		 		 		 		 " suite is recommended");
		 		 		 return -1;
		 		 }
		 		 if( swapoff(filename) != 0) {
		 		 		 tst_resm(TWARN, "Failed to turn off swap files. system"
		 		 		 		 		 " reboot after execution of LTP test"
		 		 		 		 		 " suite is recommended");
		 		 		 return -1;
		 		 }
		 }
		 return 0;
}

/* setup() - performs all ONE TIME setup for this test */
void
setup()
{

int pagesize = getpagesize();
  /* capture signals */
  tst_sig(FORK, DEF_HANDLER, cleanup);

  /* set the expected errnos... */
  TEST_EXP_ENOS(exp_enos);

  /* Check whether we are root*/
  if (geteuid() != 0) {
    tst_brkm(TBROK, tst_exit, "Test must be run as root");
  }

  /* make a temp directory and cd to it */
  tst_tmpdir();


  if(tst_is_cwd_tmpfs()) {
    tst_brkm(TBROK, cleanup, "Cannot do swapon on a file located on a tmpfs filesystem");
  }

  /*create file*/
  if ((strncmp(kmachine, "ia64", 4)) == 0) {
    if(system("dd if=/dev/zero of=swapfile01 bs=1024  count=65536 > tmpfile"
	     " 2>&1") != 0) {
     tst_brkm(TBROK, cleanup, "Failed to create file for swap");
    }
  } else if (pagesize == 65536)
     {
      if(system("dd if=/dev/zero of=swapfile01 bs=1048  count=655 > tmpfile"              " 2>&1") != 0) {
      tst_brkm(TBROK, cleanup, "Failed to create file for swap");
    	}
     }else {
    if(system("dd if=/dev/zero of=swapfile01 bs=1048  count=40 > tmpfile"
	     " 2>&1") != 0) {
      tst_brkm(TBROK, cleanup, "Failed to create file for swap");
    }
  }


  /* make above file a swap file*/
  if( system("mkswap swapfile01 > tmpfile 2>&1") != 0) {
    tst_brkm(TBROK, cleanup, "Failed to make swapfile");
  }

  /* Pause if that option was specified */
  TEST_PAUSE;

}  /* End setup() */

/*
* cleanup() - Performs one time cleanup for this test at
* completion or premature exit
*/
void
cleanup()
{
		 /*
		 * print timing stats if that option was specified.
		 * print errno log if that option was specified.
		 */
		 TEST_CLEANUP;

		 /* Remove tmp dir and all files inside it*/
		 tst_rmdir();

		 /* exit with return code appropriate for results */
		 tst_exit();
}		 /* End cleanup() */
