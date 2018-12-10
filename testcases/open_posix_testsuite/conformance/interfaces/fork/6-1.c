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
* The opened directory streams are copied to the child process.
* Positionning information may be shared between both processes.

* The steps are:
* -> Open the current directory,
* -> Count the directory entries, then rewind.
* -> create a child
* -> The child counts the directory entries.

* The test fails if the directory is read in the parent and not in the child.

*/


#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <errno.h>

#include <dirent.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

static int count(DIR * thedir)
{
	int counter = 0;

	struct dirent *dp;

	rewinddir(thedir);

	/* Count the directory entries */

	do {
		dp = readdir(thedir);

		if (dp != NULL)
			counter++;
	}
	while (dp != NULL);

	return counter;
}

int main(void)
{
	int ret, status;
	pid_t child, ctl;

	int counted;

	/* the '.' directory pointers */
	DIR *dotdir;

	output_init();

	/* Open the directory */
	dotdir = opendir(".");

	if (dotdir == NULL) {
		UNRESOLVED(errno, "opendir failed");
	}

	/* Count entries */
	counted = count(dotdir);

#if VERBOSE > 0

	output("Found %d entries in current dir\n", counted);

#endif

	/* Create the child */
	child = fork();

	if (child == -1) {
		UNRESOLVED(errno, "Failed to fork");
	}

	/* child */
	if (child == 0) {
		/* Count in child process */
		counted = count(dotdir);

#if VERBOSE > 0

		output("Found %d entries in current dir from child\n", counted);
#endif

		ret = closedir(dotdir);

		if (ret != 0) {
			UNRESOLVED(errno, "Failed to close dir in child");
		}

		/* We're done */
		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status) || (WEXITSTATUS(status) != PTS_PASS)) {
		FAILED("Child exited abnormally -- dir stream not copied?");
	}

	/* close the directory stream */
	ret = closedir(dotdir);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to closedir in parent");
	}

#if VERBOSE > 0
	output("Test passed\n");

#endif

	PASSED;
}
