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
/**********************************************************
 *
 *    TEST IDENTIFIER	: clone02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Functionality tests for clone(2)
 *
 *    TEST CASE TOTAL	: 2
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  generate a unique file name fore each test instance
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	 TEST1
 *	 -----
 *		Call clone() with all resources shared.
 *
 *		CHILD:
 *			modify the shared resources
 *			return 1 on success
 *		PARENT:
 *			wait for child to finish
 *			verify that the shared resourses are modified
 *			return 1 on success
 *		If parent & child returns successfully
 *			test passed
 *		else
 *			test failed
 *
 *	 TEST2
 *	 -----
 *		Call clone() with no resources shared.
 *
 *		CHILD:
 *			modify the resources in child's address space
 *			return 1 on success
 *		PARENT:
 *			wait for child to finish
 *			verify that the parent's resourses are not modified
 *			return 1 on success
 *		If parent & child returns successfully
 *			test passed
 *		else
 *			test failed
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *	clone02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *				where,  -c n : Run n copies concurrently.
 *				-e   : Turn on errno logging.
 *				-h   : Show help screen
 *				-f   : Turn off functional testing
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 *
 * MODIFIED: - mridge@us.ibm.com -- changed getpid to syscall(get thread ID) for unique ID on NPTL threading
 *
 *
 ****************************************************************/

#if defined UCLINUX && !__THROW
/* workaround for libc bug */
#define __THROW
#endif

#include <errno.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include "test.h"
#include "usctest.h"

#define FLAG_ALL CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | SIGCHLD
#define FLAG_NONE SIGCHLD
#define PARENT_VALUE 1
#define CHILD_VALUE 2
#define TRUE 1
#define FALSE 0

#include "clone_platform.h"

static void setup();
static int test_setup();
static void cleanup();
static void test_cleanup();
static int child_fn();
static int parent_test1();
static int parent_test2();
static int test_VM();
static int test_FS();
static int test_FILES();
static int test_SIG();
static int modified_VM();
static int modified_FS();
static int modified_FILES();
static int modified_SIG();
static void sig_child_defined_handler();
static void sig_default_handler();

static int fd_parent;
static char file_name[25];
static int parent_variable;
static char cwd_parent[FILENAME_MAX];
static int parent_got_signal, child_pid;

char *TCID = "clone02";		/* Test program identifier.    */
extern int Tst_count;		/* Test Case counter for tst_* routines */

struct test_case_t {
	int flags;
	int (*parent_fn) ();
} test_cases[] = {
	{
	FLAG_ALL, parent_test1}, {
	FLAG_NONE, parent_test2}
};

int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av)
{

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	void *child_stack;	/* stack for child */
	int status, i;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* perform global setup for test */
	setup();

	/* Allocate stack for child */
	if ((child_stack = (void *)malloc(CHILD_STACK_SIZE)) == NULL) {
		tst_brkm(TBROK, cleanup, "Cannot allocate stack for child");
	}

	/* check looping state if -c option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			/*Do test specific setup */
			if (!(test_setup())) {
				tst_resm(TWARN, "test_setup() failed,"
					 "skipping this test case");
				continue;
			}

			/* Test the system call */
			TEST(ltp_clone(test_cases[i].flags, child_fn, NULL,
				CHILD_STACK_SIZE, child_stack));

			/* check return code */
			if (TEST_RETURN == -1) {
				tst_resm(TFAIL|TTERRNO, "clone() failed");
				/* Cleanup & continue with next test case */
				test_cleanup();
				continue;
			}

			/* Wait for child to finish */
			if ((wait(&status)) < 0) {
				tst_resm(TWARN|TERRNO, "wait() failed, skipping this"
					 " test case");
				/* Cleanup & continue with next test case */
				test_cleanup();
				continue;
			}

			if (WTERMSIG(status)) {
				tst_resm(TWARN, "child exitied with signal %d",
					 WTERMSIG(status));
			}

			/*
			 * Check the return value from child function  and
			 * parent function. If both functions returned
			 * successfully, test passed, else failed
			 */
			if ((WIFEXITED(status)) && (WEXITSTATUS(status)) &&
			    (test_cases[i].parent_fn())) {
				tst_resm(TPASS, "Test Passed");
			} else {
				tst_resm(TFAIL, "Test Failed");
			}

			/* Do test specific cleanup */
			test_cleanup();
		}
	}			/* End for TEST_LOOPING */

	free(child_stack);

	/* cleanup and exit */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Create temporary directory and 'cd' to it. */
	tst_tmpdir();

	/* Get unique file name for each child process */
	if ((sprintf(file_name, "parent_file_%ld", syscall(__NR_gettid))) <= 0) {
		tst_brkm(TBROK|TERRNO, cleanup, "sprintf() failed");
	}

}				/* End setup() */

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove temperory file */
	if ((unlink(file_name)) == -1) {
		tst_resm(TWARN, "Couldn't delete file, %s", file_name);
	}
	chdir("/tmp");
	remove(cwd_parent);

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */

/*
 * test_setup() - test specific setup function
 */
int test_setup()
{

	struct sigaction def_act;

	/* Save current working directory of parent */
	if ((getcwd(cwd_parent, sizeof(cwd_parent))) == NULL) {
		tst_resm(TWARN|TERRNO, "getcwd() failed in test_setup()");
		return 0;
	}

	/*
	 * Set value for parent_variable in parent, which will be
	 * changed by child in test_VM(), for testing CLONE_VM flag
	 */
	parent_variable = PARENT_VALUE;

	/*
	 * Open file from parent, which will be closed by
	 * child in test_FILES(), used for testing CLONE_FILES flag
	 */
	if ((fd_parent = open(file_name, O_CREAT | O_RDWR, 0777)) == -1) {	//mode must be specified when O_CREATE is in the flag
		tst_resm(TWARN|TERRNO, "open() failed in test_setup()");
		return 0;
	}

	/*
	 * set parent_got_signal to FALSE, used for testing
	 * CLONE_SIGHAND flag
	 */
	parent_got_signal = FALSE;

	/* Setup signal handler for SIGUSR2 */
	def_act.sa_handler = sig_default_handler;
	def_act.sa_flags = SA_RESTART;

	if ((sigaction(SIGUSR2, &def_act, NULL)) == -1) {
		tst_resm(TWARN|TERRNO, "sigaction() failed in test_setup()");
		return 0;
	}

	/* test_setup() returns success */
	return 1;
}

/*
 * test_cleanup() - test specific cleanup function
 */
void test_cleanup()
{

	/* Restore parent's working directory */
	if ((chdir(cwd_parent)) == -1) {
		/* we have to exit here */
		tst_brkm(TBROK|TERRNO, cleanup, "chdir() failed in test_cleanup()");
	}

}

/*
 * child_fn() - child function
 */
int child_fn()
{

	/* save child pid */
	child_pid = syscall(__NR_gettid);

	/*child_pid = getpid(); changed to above to work on POSIX threads -- NPTL */

	if (test_VM() && test_FILES() && test_FS() && test_SIG()) {
		exit(1);
	}
	exit(0);
}

/*
 * parent_test1() - parent function for test1
 */
int parent_test1()
{

	/*
	 * For first test case (with all flags set), all resources are
	 * shared between parent and child. So whatever changes made by
	 * child should get reflected in parent also. modified_*()
	 * functions check this. All of them should return 1 for
	 * parent_test1() to return 1
	 */

	if (modified_VM() && modified_FILES() && modified_FS() &&
	    modified_SIG()) {
		return 1;
	}
	return 0;
}

/*
 * parent_test2 - parent function for test 2
 */
int parent_test2()
{

	/*
	 * For second test case (with no resouce shared), all of the
	 * modified_*() functions should return 0 for parent_test2()
	 * to return 1
	 */

	if (modified_VM() || modified_FILES() || modified_FS() ||
	    modified_SIG()) {
		return 0;
	}
	return 1;
}

/*
 * test_VM() - function to change parent_variable from child's
 *	       address space. If CLONE_VM flag is set, child shares
 *	       the memory space with parent so this will be visible
 *	       to parent also.
 */

int test_VM()
{
	parent_variable = CHILD_VALUE;
	return 1;
}

/*
 * test_FILES() - This function closes a file descriptor opened by
 *		  parent. If CLONE_FILES flag is set, the parent and
 *		  the child process share the same file descriptor
 *		  table. so this affects the parent also
 */
int test_FILES()
{
	if ((close(fd_parent)) == -1) {
		tst_resm(TWARN|TERRNO, "close() failed in test_FILES()");
		return 0;
	}
	return 1;
}

/*
 * test_FS() -  This function changes the current working directory
 *		of the child process. If CLONE_FS flag is set, this
 *		will be visible to parent also.
 */
int test_FS()
{
	if ((chdir("/tmp")) == -1) {
		tst_resm(TWARN|TERRNO, "chdir() failed in test_FS()");
		return 0;
	}
	return 1;
}

/*
 * test_SIG() - This function changes the signal handler for SIGUSR2
 *		signal for child. If CLONE_SIGHAND flag is set, this
 *		affects parent also.
 */
int test_SIG()
{

	struct sigaction new_act;

	new_act.sa_handler = sig_child_defined_handler;
	new_act.sa_flags = SA_RESTART;

	/* Set signal handler to sig_child_defined_handler */
	if ((sigaction(SIGUSR2, &new_act, NULL)) == -1) {
		tst_resm(TWARN|TERRNO, "signal() failed in test_SIG()");
		return 0;
	}

	/* Send SIGUSR2 signal to parent */
	if ((kill(getppid(), SIGUSR2)) == -1) {
		tst_resm(TWARN|TERRNO, "kill() failed in test_SIG()");
		return 0;
	}
	return 1;
}

/*
 * modified_VM() - This function is called by parent process to check
 *		   whether child's modification to parent_variable
 *		   is visible to parent
 */

int modified_VM()
{

	if (parent_variable == CHILD_VALUE) {
		/* child has modified parent_variable */
		return 1;
	}
	return 0;
}

/*
 * modified_FILES() - This function checks for file descriptor table
 *		      modifications done by child
 */
int modified_FILES()
{
	char buff[20];

	if (((read(fd_parent, buff, sizeof(buff))) == -1) && (errno == EBADF)) {
		/* Child has closed this file descriptor */
		return 1;
	}

	/* close fd_parent */
	if ((close(fd_parent)) == -1) {
		tst_resm(TWARN|TERRNO, "close() failed in modified_FILES()");
	}

	return 0;
}

/*
 * modified_FS() - This function checks parent's current working directory
 *		   to see whether its modified by child or not.
 */
int modified_FS()
{
	char cwd[FILENAME_MAX];

	if ((getcwd(cwd, sizeof(cwd))) == NULL) {
		tst_resm(TWARN|TERRNO, "getcwd() failed");
	}

	if (!(strcmp(cwd, cwd_parent))) {
		/* cwd hasn't changed */
		return 0;
	}
	return 1;
}

/*
 * modified_SIG() - This function checks whether child has changed
 *		    parent's signal handler for signal, SIGUSR2
 */
int modified_SIG()
{

	if (parent_got_signal) {
		/*
		 * parent came through sig_child_defined_handler()
		 * this means child has changed parent's handler
		 */
		return 1;
	}
	return 0;
}

/*
 * sig_child_defined_handler()  - Signal handler installed by child
 */
void sig_child_defined_handler(int pid)
{
	if ((syscall(__NR_gettid)) == child_pid) {
		/* Child got signal, give warning */
		tst_resm(TWARN, "Child got SIGUSR2 signal");
	} else {
		parent_got_signal = TRUE;
	}
}

/* sig_default_handler() - Default handler for parent */
void sig_default_handler()
{
}
