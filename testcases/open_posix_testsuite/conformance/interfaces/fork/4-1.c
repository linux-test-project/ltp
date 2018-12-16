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
 * The parent process ID of the child is the process ID of the parent (caller of fork())

 * The steps are:
 * -> create a child
 * -> check its parent process ID is the PID of its parent.

 * The test fails if the IDs differ.

 */


#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <errno.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

int main(void)
{
	int status;
	pid_t child, ctl;

	output_init();

	ctl = getpid();

	/* Create the child */
	child = fork();
	if (child == -1) {
		UNRESOLVED(errno, "Failed to fork");
	}

	/* child */
	if (child == 0) {
		/* Check the parent process ID */
		if (ctl != getppid()) {
			FAILED
			    ("The parent process ID is not the PID of the parent");
		}

		/* We're done */
		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);
	if (ctl != child) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}
	if ((!WIFEXITED(status)) || (WEXITSTATUS(status) != PTS_PASS)) {
		UNRESOLVED(status, "Child exited abnormally");
	}

#if VERBOSE > 0
	output("Test passed\n");
#endif

	PASSED;
}
