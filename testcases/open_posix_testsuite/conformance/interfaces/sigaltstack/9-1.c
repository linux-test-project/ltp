/*
 *   Copyright (c) Novell Inc. 2011
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms in version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 *   Author:  Peter W. Morreale <pmorreale AT novell DOT com>
 *   Date:    09.07.2011
 *
 *
 * Test assertion 9-1 - Prove that an established alternate signal stack
 * is not available after an exec.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "posixtest.h"

static stack_t a;

int main(int argc, char *argv[])
{
	int rc;
	char path[PATH_MAX + 1];

	/* Called with no args, do the exec, otherwise check.  */
	if (argc == 1) {
		a.ss_flags = 0;
		a.ss_size = SIGSTKSZ;
		a.ss_sp = malloc(SIGSTKSZ);
		if (!a.ss_sp) {
			printf("Failed: malloc(SIGSTKSZ) == NULL\n");
			exit(PTS_UNRESOLVED);
		}

		rc = sigaltstack(&a, NULL);
		if (rc) {
			printf("Failed: sigaltstack() rc: %d errno: %s\n",
			       rc, strerror(errno));
			exit(PTS_UNRESOLVED);
		}

		/* Get abs path if needed and exec ourself */
		if (*argv[0] != '/') {
			if (getcwd(path, PATH_MAX) == NULL) {
				perror("getcwd");
				exit(PTS_UNRESOLVED);
			}
			strcat(path, "/");
			strcat(path, argv[0]);
		} else {
			strcpy(path, argv[0]);
		}
		execl(path, argv[0], "verify", NULL);
		printf("Failed: execl() errno: %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);

	} else if (strcmp(argv[1], "verify")) {
		printf("Failed: %s called with unexpected argument: %s\n",
		       argv[0], argv[1]);
		exit(PTS_UNRESOLVED);
	}

	/* Verify the alt stack is disabled */
	rc = sigaltstack(NULL, &a);
	if (rc || a.ss_flags != SS_DISABLE) {
		printf("Failed: sigaltstack() rc: %d ss_flags: %u\n",
		       rc, a.ss_flags);
		exit(PTS_FAIL);
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
