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
/* $Id: fcntl07.c,v 1.12 2009/03/23 13:35:41 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: fcntl07
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Close-On-Exec functional test
 *
 *    PARENT DOCUMENT	: none
 *
 *    TEST CASE TOTAL	: 2
 *
 *    WALL CLOCK TIME	: 5
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: Glen Overby
 *
 *    CO-PILOT		: William Roske
 *
 *    DATE STARTED	: 08/11/93
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 *	1.) test close-on-exec with a regular file
 *	2.) test close-on-exec with a system pipe
 *
 *    INPUT SPECIFICATIONS
 *
 *	Standard arguments accepted by parse_opts(3).
 *
 *	The -t (timing) and -e options apply to the fcntl(.., F_SETFD, ..)
 *	system call.
 *
 *	-T fd	  : If this option is given, the program runs as "test_open",
 *		    testing <fd> to see if it is open or not and exiting
 *		    accordingly:
 *			0	not open (EBADF from fcntl(..., F_GETFD, ...))
 *			3	no error from fcntl
 *			errno	fcntl returned an error other than EBADF
 *
 *	-F name   : File to open.  Must be an absolute path
 *		    and the file must be writable;
 *	-n program: path to the 'test_open' program
 *
 *    OUTPUT SPECIFICATIONS
 *        This test uses the cuts-style test_res format output consisting of:
 *
 *                 test-name   PASS/FAIL/BROK	message
 *
 *        the message will tell what type of test and, if it failed, indicate
 *        what the failure was.
 *
 *    DURATION
 *	Terminates
 *
 *    SIGNALS
 *	None
 *
 *    RESOURCES
 *	None
 *
 *    ENVIRONMENTAL NEEDS
 *      No run-time environmental needs.
 *
 *	If this test is not called with a full pathname, it must be able
 *	to find itself on $PATH
 *
 *    INTERCASE DEPENDENCIES
 *	none
 *
 *    DETAILED DESCRIPTION
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Create and make current a temporary directory.
 *	  Open a regular file for writing
 *	  Create a system pipe
 *	  Create a named pipe and open it for writing
 *
 *	Test:
 *	  Set the file descriptor for close-on-exec
 *	  Fork
 *		Child execlp's the program "test_open".
 *		If the exec fails, exit "2"
 *	        Parent waits
 *	  Report results.
 *
 *	Cleanup:
 *	  Close file and pipes
 *	  Remove the temporary directory
 *
 *    BUGS
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <limits.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"
#include "search_path.h"

void setup();
void cleanup();
void help();

char *TCID = "fcntl07";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */

int fflag, Tflag;		/* binary flags: opt or not */
char *fopt, *Topt;		/* option arguments */

option_t options[] = {
	{"F:", &fflag, &fopt},	/* -F filename */
	{"T:", &Tflag, &Topt},	/* -T <fd>  exec'ed by test: test FD */
	{NULL, NULL, NULL}
};

int stat_loc;			/* for waitpid() */

int file_fd, pipe_fds[2];
	/* file descriptors for a file and a system pipe */
#define DEFAULT_FILE "DefaultFileName"
char *File1 = DEFAULT_FILE;

#define DEFAULT_SUBPROG "test_open"
char *openck = DEFAULT_SUBPROG;	/* support program name to check for open FD */

#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX 256
#endif
char subprog_path[_POSIX_PATH_MAX];	/* path to exec "openck" with */

#define STRSIZE 255

int *testfds[] = {
	&file_fd, &pipe_fds[1], 0
};

char *testfdtypes[] = {
	"regular file",
	"write side of system pipe",
};

int test_open(char *arg);
int do_exec(char *prog, int fd, char *tcd);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int exec_return;	/* return from do_exec */
	int **tcp;		/* testcase pointer (pointer to FD) */
	char **tcd;		/* testcase description pointer */

	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (fflag)
		File1 = fopt;

	if (Tflag)
		exit(test_open(Topt));

	setup(av[0]);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (tcp = testfds, tcd = testfdtypes; *tcp; tcp++, tcd++) {

			TEST(fcntl(**tcp, F_SETFD, FD_CLOEXEC));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL|TTERRNO,
					 "fcntl(%s[%d], F_SETFD, FD_CLOEXEC) "
					 "failed",
					 *tcd, **tcp);
			} else {

				if (STD_FUNCTIONAL_TEST) {

					exec_return =
					    do_exec(subprog_path, **tcp, *tcd);

					switch (exec_return) {
					case -1:
						tst_resm(TBROK,
							 "fork failed.  Errno %s [%d]",
							 strerror(errno),
							 errno);
						break;
					case 1:
						tst_resm(TBROK,
							 "waitpid return was 0%o",
							 stat_loc);
						break;
					case 2:
						tst_resm(TBROK, "exec failed");	/* errno was in child */
						break;
					case 0:
						tst_resm(TPASS,
							 "%s child exited 0, indicating that the file was closed",
							 *tcd);
						break;
					default:
						tst_resm(TFAIL,
							 "%s child exited non-zero, %d",
							 *tcd, exec_return);
						break;
					}
				}
			}
		}
	}

	cleanup();

	tst_exit();
}

void setup(char *path)
{
	search_path(path, subprog_path, X_OK, 1);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	file_fd = SAFE_OPEN(cleanup, File1, O_CREAT|O_RDWR, 0666);
	SAFE_PIPE(cleanup, pipe_fds);
}

void cleanup()
{
	TEST_CLEANUP;

	SAFE_CLOSE(NULL, file_fd);
	SAFE_CLOSE(NULL, pipe_fds[0]);
	SAFE_CLOSE(NULL, pipe_fds[1]);

	tst_rmdir();

}

void help()
{
	printf
	    ("-T fd     : If this option is given, the program runs as 'test_open'\n");
	printf
	    ("            testing <fd> to see if it is open or not and exiting accordingly\n");
	printf("-F name   : File to open.  Must be an absolute path,\n");
	printf("            and the file must be writable\n");
	printf("-n program: path to the 'test_open' program\n");
}

/*---------------------------------------------------------------------------*/
/* Perform an exec, then wait for the child to terminate.
 * The child's termination status determines the success of the test
 *
 * Return codes:
 *	-1	BROK	fork failed
 *	1	BROK	waitpid returned != exit status
 *	<else>	????	exit code from child:
 *	2	BROK	exec failed
 *	0	PASS	fd was properly closed
 *
 */

int do_exec(char *prog, int fd, char *tcd)
{
	int pid;
	char pidname[STRSIZE];
#ifdef DEBUG
	int rc, status;		/* for the fcntl */
#endif

	/* set up arguments to exec'ed child */
	sprintf(pidname, "%d", fd);

#ifdef DEBUG
	rc = fcntl(fd, F_GETFD, &status);
	printf("%s: fd = %d rc = %d status= %d, errno= %d\n", tcd, fd, rc,
	       status, errno);
#endif

	switch (pid = FORK_OR_VFORK()) {
	case -1:
		return (-1);
	case 0:		/* child */
		execlp(prog, openck, "-T", pidname, NULL);

		/* the ONLY reason to do this is to get the errno printed out */
		fprintf(stderr, "exec(%s, %s, -T, %s) failed.  Errno %s [%d]\n",
			prog, openck, pidname, strerror(errno), errno);
		exit(2);
	default:		/* parent */
		waitpid(pid, &stat_loc, 0);
		if (WIFEXITED(stat_loc)) {
			return (WEXITSTATUS(stat_loc));
		} else {
			return 1;
		}
	}
}

/*
 *    PROGRAM TITLE	: Test if a named file descriptor is open
 *    This function is called when fcntcs07 is called with the -T option.
 *    It tests if a file descriptor is open and exits accordingly.
 */
int test_open(char *arg)
{
	int fd, rc;
	int status;

	fd = atoi(arg);

	rc = fcntl(fd, F_GETFD, &status);

#ifdef DEBUG_T
	printf("%s: fd = %d rc = %d status= %d, errno= %d\n", openck, fd, rc,
	       status, errno);
#endif

	if (rc == -1 && errno == EBADF) {
		exit(0);
	}

	if (rc != -1)
		exit(3);

	exit(errno);
	return -1;		/* to remove compiler warning on IRIX */
}
