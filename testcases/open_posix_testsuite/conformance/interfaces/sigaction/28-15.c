/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis
* Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
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
* with this program; if not, write the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
* This sample test aims to check the following assertions:
*
* If the signal action was set with the signal() function, getting it into oact
* then reinstalling it with act must be valid.
*
* The steps are:
* -> register a signal handler for SIGTTIN with signal().
* -> check this signal handler works.
* -> change the signal handler with sigaction, saving old handler in oact.
* -> check the new signal handler works.
* -> set the old signal handler back
* -> check the old signal handler still works.
*
* The test fails if a signal handler does not work as expected.
*/


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "posixtest.h"

static volatile sig_atomic_t called = 1;

static void handler_1()
{
	called++;
}

static void handler_2()
{
	called--;
}

int main(void)
{
	int ret;

	struct sigaction sa, save;

	if (signal(SIGTTIN, handler_1) == SIG_ERR) {
		perror("Failed to register signal handler");
		return PTS_UNRESOLVED;
	}

	/* As whether signal handler is restored to default when executed
	is implementation defined, we cannot check it was registered here. */

	/* Set the new signal handler with sigaction*/
	sa.sa_flags = 0;

	sa.sa_handler = handler_2;

	ret = sigemptyset(&sa.sa_mask);

	if (ret != 0) {
		perror("Failed to empty signal set");
		return PTS_UNRESOLVED;
	}

	ret = sigaction(SIGTTIN, &sa, &save);

	if (ret != 0) {
		perror("Failed to register signal handler");
		return PTS_UNRESOLVED;
	}

	/* Check the signal handler has been set up */
	ret = raise(SIGTTIN);

	if (ret != 0) {
		perror("Failed to raise signal");
		return PTS_UNRESOLVED;
	}

	if (called != 0) {
		fprintf(stderr, "Handler was not executed\n");
		return PTS_FAIL;
	}

	/* Restore the first signal handler */
	ret = sigaction(SIGTTIN, &save, 0);

	if (ret != 0) {
		perror("Failed to restore signal handler");
		return PTS_UNRESOLVED;
	}

	/* Check the signal handler has been set up */
	ret = raise(SIGTTIN);

	if (ret != 0) {
		perror("Failed to raise signal");
		return PTS_UNRESOLVED;
	}

	if (called != 1) {
		fprintf(stderr, "Handler was not executed\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
