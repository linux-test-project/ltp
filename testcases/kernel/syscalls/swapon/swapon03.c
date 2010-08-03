/******************************************************************************
 *
 *   Copyright (c) International Business Machines  Corp., 2007
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
 *
 * NAME
 *      swapon03.c
 *
 * DESCRIPTION
 *      This test case checks whether swapon(2) system call returns:
 *        - EPERM when there are more than MAX_SWAPFILES already in use.
 *
 *	Setup:
 *		Setup signal handling.
 *		Pause for SIGUSR1 if option specified.
 * 		Create MAX_SWAPFILES - 2 (to support latest kernels) swapfiles
 * 		  	
 *	Test:
 *		Loop if the proper options are given.
 *		Execute system call.
 *		Check return code, if system call fails with errno == expected errno
 *	 	Issue syscall passed with expected errno
 *		Otherwise,
 *		Issue syscall failed to produce expected errno
 *
 * 	Cleanup:
 * 		    Do cleanup for the test.
 *
 * USAGE:  <for command-line>
 *  swapon03 [-e] [-i n] [-I x] [-p x] [-t] [-h] [-f] [-p]
 *  where
 *		  -e   : Turn on errno logging.
 *		  -i n : Execute test n times.
 *		  -I x : Execute test for x seconds.
 *		  -p   : Pause for SIGUSR1 before starting
 *		  -P x : Pause for x seconds between iterations.
 *		  -t   : Turn on syscall timing.
 *
 * Author
 *	Ricardo Salveti de Araujo <rsalveti@linux.vnet.ibm.com> based on
 *	swapon02 created by Aniruddha Marathe
 *
 * History
 *      16/08/2007      Created <rsalveti@linux.vnet.ibm.com>
 *
******************************************************************************/

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"
#include "config.h"
#include "linux_syscall_numbers.h"
#include "swaponoff.h"

void setup();
void cleanup();
int setup_swap();
int clean_swap();
int check_and_swapoff(char *filename);
int create_swapfile(char *swapfile, int bs, int count);

char *TCID = "swapon03";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

static int exp_enos[] = { EPERM, 0 };
static int swapfiles;		/* Number of swapfiles turned on */

struct utsname uval;
char *kmachine;

/* Paths for files that we'll use to test */
int testfiles = 3;
static struct swap_testfile_t {
	char *filename;
} swap_testfiles[] = {
	{
	"firstswapfile"}, {
	"secondswapfile"}, {
	"thirdswapfile"}
};

int expected_errno = EPERM;	/* Expected errno when doing the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/***************************************************************
	 * parse standard options
	 ***************************************************************/
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL)
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);

	/***************************************************************
	 * perform global setup for test
	 ***************************************************************/
	uname(&uval);
	kmachine = uval.machine;
	setup();

	/***************************************************************
	 * check looping state if -i option given
	 ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		/* do the test setup */
		if (setup_swap() < 0) {
			clean_swap();
			tst_brkm(TBROK, cleanup,
				 "Setup failed, quitting the test");
		}

		/* Call swapon sys call for the first time */
		TEST(syscall(__NR_swapon, swap_testfiles[0].filename, 0));

		/* Check return code */
		if ((TEST_RETURN == -1) && (TEST_ERRNO == expected_errno)) {
			tst_resm(TPASS, "swapon(2) got expected failure;"
				 " Got errno = %d," , expected_errno);
		} else if (TEST_RETURN < 0) {
			tst_resm(TFAIL, "swapon(2) failed to produce"
				 " expected error: %d, got %d (%s)."
				 " System reboot after execution of LTP"
				 " test suite is recommended.", expected_errno,
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			/* Probably the system supports MAX_SWAPFILES > 30,
			 * let's try with MAX_SWAPFILES == 32 */

			/* Call swapon sys call once again for 32
			 * now we can't receive an error */
			TEST(syscall(__NR_swapon, swap_testfiles[1].filename, 0));

			/* Check return code (now we're expecting success) */
			if (TEST_RETURN < 0) {
				tst_resm(TFAIL,
					 "swapon(2) got an unexpected failure;"
					 " Got errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				/* Call swapon sys call once again for 33
				 * now we have to receive an error */
				TEST(syscall(__NR_swapon, swap_testfiles[2].filename, 0));

				/* Check return code (should be an error) */
				if ((TEST_RETURN == -1)
				    && (TEST_ERRNO == expected_errno)) {
					tst_resm(TPASS,
						 "swapon(2) got expected failure;"
						 " Got errno = %d, probably your"
						 " MAX_SWAPFILES is 32",
						 expected_errno);
				} else {
					tst_resm(TFAIL,
						 "swapon(2) failed to produce"
						 " expected error: %d, got %s."
						 " System reboot after execution of LTP"
						 " test suite is recommended.",
						 expected_errno,
						 strerror(TEST_ERRNO));
				}

			}
		}

		/* do the clean */
		if (clean_swap() < 0)
			tst_brkm(TBROK, cleanup,
				 "Cleanup failed, quitting the test");

		TEST_ERROR_LOG(TEST_ERRNO);

	}			/* End of TEST LOOPING */

	/***************************************************************
	 * cleanup and exit
	 ***************************************************************/
	cleanup();

	return (0);

}				/* End of main */

/***************************************************************
 * setup_swap() - Create 33 and activate 30 swapfiles.
 ***************************************************************/
int setup_swap()
{
	int j, fd;		/*j is loop counter, fd is file descriptor */
	int pid;		/* used for fork */
	int status;		/* used for fork */
	int res = 0, pagesize = getpagesize();
	int bs, count;
	char filename[15];	/* array to store new filename */
	char buf[BUFSIZ + 1];	/* temp buffer for reading /proc/swaps */

	/* Find out how many swapfiles (1 line per entry) already exist */
	swapfiles = 0;

	if (seteuid(0) < 0) {
		tst_brkm(TFAIL | TERRNO, cleanup, "Failed to call seteuid");
	}

	/* This includes the first (header) line */
	if ((fd = open("/proc/swaps", O_RDONLY)) == -1) {
		tst_brkm(TFAIL | TERRNO, cleanup,
			 "Failed to find out existing number of swap files");
	}
	do {
		char *p = buf;
		res = read(fd, buf, BUFSIZ);
		if (res < 0) {
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "Failed to find out existing number of swap files");
		}
		buf[res] = '\0';
		while ((p = strchr(p, '\n'))) {
			p++;
			swapfiles++;
		}
	} while (BUFSIZ <= res);
	close(fd);
	if (swapfiles)
		swapfiles--;	/* don't count the /proc/swaps header */

	if (swapfiles < 0) {
		tst_brkm(TFAIL, cleanup, "Failed to find existing number of swapfiles");
	}

	/* Determine how many more files are to be created */
	swapfiles = MAX_SWAPFILES - swapfiles;
	if (swapfiles > MAX_SWAPFILES) {
		swapfiles = MAX_SWAPFILES;
	}

	/* args for dd */
	if ((strncmp(kmachine, "ia64", 4)) == 0) {
		bs = 1024;
		count = 1024;
	} else if (pagesize == 65536) {
		bs = 1048;
		count = 655;
	} else {
		bs = 1048;
		count = 40;
	}

	pid = FORK_OR_VFORK();
	if (pid == 0) {
		/*create and turn on remaining swapfiles */
		for (j = 0; j < swapfiles; j++) {

			/* prepare filename for the iteration */
			if (sprintf(filename, "swapfile%02d", j + 2) < 0) {
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "sprintf() failed to create filename");
			}

			/* Create the swapfile */
			if (create_swapfile(filename, bs, count) < 0) {
				tst_brkm(TFAIL, cleanup,
					 "Failed to create swapfile for the test");
			}

			/* turn on the swap file */
			if ((res = syscall(__NR_swapon, filename, 0)) != 0) {
				if (errno == EPERM) {
					printf("Successfully created %d swapfiles\n", j);
					break;
				} else {
					tst_brkm(TFAIL | TERRNO, cleanup,
						 "Failed swapon for file %s", filename);
					/* must cleanup already swapon files */
					clean_swap();
					exit(1);
				}
			}
		}
		tst_exit();
	} else
		waitpid(pid, &status, 0);

	if (WEXITSTATUS(status)) {
		tst_resm(TFAIL, "Failed to setup swaps");
		exit(1);
	}

	/* Create all needed extra swapfiles for testing */
	for (j = 0; j < testfiles; j++) {
		if (create_swapfile(swap_testfiles[j].filename, bs, count) < 0) {
			tst_resm(TWARN,
				 "Failed to create swapfiles for the test");
			exit(1);
		}
	}

	return 0;

}

/***************************************************************
 * create_swapfile() - Create a swap file
 ***************************************************************/
int create_swapfile(char *swapfile, int bs, int count)
{

	char *cmd_buffer;
	int rc = -1;

	/* prepare the buffer. */
	if ((cmd_buffer = calloc(sysconf(_SC_ARG_MAX)+1, sizeof(char))) == NULL) {
		tst_resm(TWARN,
			"failed to allocate enough memory for the command "
			"buffer");
	/* prepare the path string for dd command */
	} else if (snprintf(cmd_buffer, sysconf(_SC_ARG_MAX),
		    "dd if=/dev/zero of=%s bs=%d "
		    "count=%d > tmpfile 2>&1", swapfile, bs, count) < 0) {
		tst_resm(TWARN,
			 "sprintf() failed to create the command string");
	}
	else if (system(cmd_buffer) != 0) {
		tst_resm(TWARN, "dd command failed to create file via "
				"command: %s", cmd_buffer);
	}
	/* make the file swapfile */
	else if (snprintf(cmd_buffer, sysconf(_SC_ARG_MAX),
		    "mkswap %s > tmpfile 2>&1", swapfile) < 0) {
		tst_resm(TWARN,
			 "snprintf() failed to create mkswap command string");
	} else if (system(cmd_buffer) != 0) {
		tst_resm(TWARN, "failed to make swap file %s via command %s",
			 swapfile, cmd_buffer);
	} else {
		rc = 0;
	}

	if (cmd_buffer != NULL) {
		free(cmd_buffer);
	}

	return rc;
}

/***************************************************************
 * clean_swap() - clearing all turned on swapfiles
 ***************************************************************/
int clean_swap()
{
	int j;			/* loop counter */
	char filename[FILENAME_MAX];

	for (j = 0; j < swapfiles; j++) {
		if (snprintf(filename, sizeof(filename),
			    "swapfile%02d", j+2) < 0) {
			tst_resm(TWARN, "sprintf() failed to create filename");
			tst_resm(TWARN, "Failed to turn off swap files. System"
				 " reboot after execution of LTP test"
				 " suite is recommended");
			return -1;
		}
		if (check_and_swapoff(filename) != 0) {
			tst_resm(TWARN, "Failed to turn off swap file %s.",
				 filename);
			return -1;
		}
	}

	for (j = 0; j < testfiles; j++) {
		if (check_and_swapoff(swap_testfiles[j].filename) != 0) {
			tst_resm(TWARN, "Failed to turn off swap file %s.",
				 swap_testfiles[j].filename);
			return -1;
		}
	}

	return 0;
}

/***************************************************************
 * check_and_swapoff() - check if the file is at /proc/swaps and
 * 			 remove it giving swapoff
 ***************************************************************/
int check_and_swapoff(char *filename)
{
	char *cmd_buffer;	/* temp buffer for commands */
	int rc = -1;

	if ((cmd_buffer = calloc(sysconf(_SC_ARG_MAX)+1, sizeof(char))) == NULL) {
		/* prepare the cmd string for grep command */
		tst_resm(TWARN,
			"failed to allocate enough memory for the command "
			"buffer");
	} else if (snprintf(cmd_buffer, sysconf(_SC_ARG_MAX),
		    "grep -q '%s.*file' /proc/swaps", filename) < 0) {
		tst_resm(TWARN,
			 "sprintf() failed to create the command string");
	} else {

		rc = 0;

		if (system(cmd_buffer) == 0) {

			/* now we need to swapoff the file */
			if (syscall(__NR_swapoff, filename) != 0) {

				tst_resm(TWARN, "Failed to turn off swap "
						"file. system reboot after "
						"execution of LTP test suite "
						"is recommended");
				rc = -1;

			} 

		} /* else nothing to clean up. */

	}
	if (cmd_buffer != NULL) {
		free(cmd_buffer);
	}

	return rc;

}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test
 ***************************************************************/
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check whether we are root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}

	/* make a temp directory and cd to it */
	tst_tmpdir();

	if (tst_is_cwd_tmpfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file located on a tmpfs filesystem");
	}

	if (tst_is_cwd_nfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file located on a nfs filesystem");
	}

	/* Pause if that option was specified */
	TEST_PAUSE;

}				/* End setup() */

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 ***************************************************************/
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove any remaining swap files */
	clean_swap();

	/* Remove tmp dir and all files inside it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();

}	/* End cleanup() */
