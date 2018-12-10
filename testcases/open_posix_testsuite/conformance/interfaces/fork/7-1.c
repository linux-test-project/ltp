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
* The child process gets a copy of the parent message catalog descriptor.

* The steps are:
* -> Create a message catalog file from the file
* -> Open this catalog
* -> fork
* -> Check that the child can read from the message catalog.

* The test fails if the message catalog is read in the parent and not in the child.

*/


#include <sys/wait.h>
#include <errno.h>
#include <nl_types.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

#define MESSCAT_IN  "messcat.txt"
#define MESSCAT_OUT "messcat.cat"

void read_catalog(nl_catd cat, char *who)
{
	char *msg = NULL;
	int i, j;
	errno = 0;

#if VERBOSE > 0
	output("Reading the message catalog from %s...\n", who);
#endif

	for (i = 1; i <= 2; i++) {
		for (j = 1; j <= 2; j++) {

			msg = catgets(cat, i, j, "not found");

			if (errno != 0)
				UNRESOLVED(errno, "catgets returned an error");
#if VERBOSE > 1
			output("set %i msg %i: %s\n", i, j, msg);
#endif
		}
	}

#if VERBOSE > 0
	output("Message catalog read successfully in %s\n", who);
#endif
}

static char *messcat_in =
    "$set 1\n1 First sentence\n2 Second sentence\n"
    "$set 2\n1 First translated sentence\n2 Second translated sentence\n";

static int create_catalog(void)
{
	FILE *f;

	if ((f = fopen(MESSCAT_IN, "w")) == NULL) {
		perror("fopen");
		return 1;
	}

	if (fputs(messcat_in, f) == EOF) {
		perror("fputs");
		fclose(f);
		return 1;
	}

	if (fclose(f) == EOF) {
		perror("fclose");
		return 1;
	}

	return 0;
}

int main(void)
{
	int ret, status;
	pid_t child, ctl;
	nl_catd messcat;

	output_init();

	/* Generate the message catalog file from the text sourcefile */
	if (system(NULL)) {

		if (create_catalog() != 0)
			UNRESOLVED(errno, "Can't create " MESSCAT_IN);

		ret = system("gencat " MESSCAT_OUT " " MESSCAT_IN);

		if (ret != 0)
			output
			    ("Could not find the source file for message catalog.\n"
			     "You may need to execute gencat yourself.\n");
	}

	messcat = catopen("./" MESSCAT_OUT, 0);

	if (messcat == (nl_catd) - 1)
		UNRESOLVED(errno, "Could not open ./" MESSCAT_OUT);

	read_catalog(messcat, "parent");

	child = fork();

	if (child == -1)
		UNRESOLVED(errno, "Failed to fork");

	if (child == 0) {
		read_catalog(messcat, "child");
		exit(PTS_PASS);
	}

	ctl = waitpid(child, &status, 0);

	if (ctl != child)
		UNRESOLVED(errno, "Waitpid returned the wrong PID");

	if ((!WIFEXITED(status)) || (WEXITSTATUS(status) != PTS_PASS))
		FAILED("Child exited abnormally");

	ret = catclose(messcat);

	if (ret != 0)
		UNRESOLVED(errno, "Failed to close the message catalog");

	system("rm -f " MESSCAT_IN " " MESSCAT_OUT);

#if VERBOSE > 0
	output("Test passed\n");
#endif
	PASSED;
}
