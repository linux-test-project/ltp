/*
* Copyright (c) 2004, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis

* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

* This sample test aims to check the following assertion:
*
* tms_{,c}{u,s}time values are set to 0 in the child process.

* The steps are:
* -> Work for 1 second and save the tms information
* -> fork
* -> Check that the child has tms values less than the saved tms.
* -> join the child process
* -> check tms_c{u,s}time are not 0 anymore.

* The test fails if one of the described checking fails.

*/

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <errno.h>

#include <sys/times.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

int main(void)
{
	struct tms ini_tms, parent_tms, child_tms;

	int status;
	pid_t child, ctl;

	clock_t st_time, cur_time;

	output_init();

	st_time = times(&ini_tms);

	if (st_time == -1)
		UNRESOLVED(errno, "times failed");

	if (ini_tms.tms_cutime != 0 || ini_tms.tms_cstime != 0)
		FAILED("The process is created with non-zero tms_cutime or "
		       "tms_cstime");

#if VERBOSE > 1
	output("Starting loop...\n");
#endif

	/* Busy loop for some times */
	do {
		cur_time = times(&parent_tms);

		if (cur_time == (clock_t) - 1)
			UNRESOLVED(errno, "times failed");
	} while ((cur_time - st_time) < sysconf(_SC_CLK_TCK));

#if VERBOSE > 1
	output("Busy loop terminated\n");
	output
	    (" Real time: %ld, User Time %ld, System Time %ld, Ticks per sec %ld\n",
	     (long)(cur_time - st_time),
	     (long)(parent_tms.tms_utime - ini_tms.tms_utime),
	     (long)(parent_tms.tms_stime - ini_tms.tms_stime),
	     sysconf(_SC_CLK_TCK));
#endif

	/* Create the child */
	child = fork();

	if (child == -1)
		UNRESOLVED(errno, "Failed to fork");

	/* child */
	if (child == 0) {

		cur_time = times(&child_tms);

		if (cur_time == (clock_t) - 1)
			UNRESOLVED(errno, "times failed");

		if ((child_tms.tms_utime + child_tms.tms_stime) >=
		    sysconf(_SC_CLK_TCK))
			FAILED("The tms struct was not reset during fork "
			       "operation");

		do {
			cur_time = times(&child_tms);

			if (cur_time == -1)
				UNRESOLVED(errno, "times failed");
		} while ((child_tms.tms_utime + child_tms.tms_stime) <= 0);

		/* We're done */
		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child)
		UNRESOLVED(errno, "Waitpid returned the wrong PID");

	if (!WIFEXITED(status) || WEXITSTATUS(status) != PTS_PASS)
		FAILED("Child exited abnormally");

	/* Check the children times were reported as expected */
	cur_time = times(&parent_tms);

#if VERBOSE > 1

	output("Child joined\n");

	output(" Real time: %ld,\n"
	       "  User Time %ld, System Time %ld,\n"
	       "  Child User Time %ld, Child System Time %ld\n",
	       (long)(cur_time - st_time),
	       (long)(parent_tms.tms_utime - ini_tms.tms_utime),
	       (long)(parent_tms.tms_stime - ini_tms.tms_stime),
	       (long)(parent_tms.tms_cutime - ini_tms.tms_cutime),
	       (long)(parent_tms.tms_cstime - ini_tms.tms_cstime)
	    );

#endif

	if (cur_time == -1)
		UNRESOLVED(errno, "times failed");

	if (parent_tms.tms_cutime == 0 && parent_tms.tms_cstime == 0)
		FAILED("The process is created with non-zero tms_cutime or "
		       "tms_cstime");

#if VERBOSE > 0

	output("Test passed\n");

#endif

	PASSED;
}
