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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.

 
 * This utility software allows to run any executable file with a timeout limit.
 * The syntax is:
 * $ ./t0 n exe arglist
 *  where n is the timeout duration in seconds,
 *        exe is the executable filename to run,
 *        arglist is the arguments to be passed to executable.
 *
 * The use of this utility is intended to be "transparent", which means
 * everything is as if 
 * $ exe arglist 
 *   had been called, and a call to "alarm(n)" had been added inside exe's main.
 *
 * SPECIAL CASE:
 * $ ./t0 0
 *  Here another arg is not required. This special case will return immediatly 
 *  as if it has been timedout. This is usefull to check a timeout return code value.
 *
 */

/* This utility should compile on any POSIX-conformant implementation. */
#define _POSIX_C_SOURCE 200112L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <assert.h>

int main (int argc, char * argv[])
{
	int ret, timeout;
	
	/* Special case: t0 0 */
	if (argc==2 && (strcmp(argv[1], "0") == 0))
	{
		kill(getpid(), SIGALRM);
		sleep(1);
		return 2;
	}
	
	/* General case */
	if (argc < 3)
	{
		printf("\nUsage: \n");
		printf("  $ %s n exe arglist\n", argv[0]);
		printf("  $ %s 0\n", argv[0]);
		printf("\nWhere:\n");
		printf("  n       is the timeout duration in seconds,\n");
		printf("  exe     is the executable filename to run,\n");
		printf("  arglist is the arguments to be passed to executable.\n\n");
		printf("  The second use case will emulate an immediate timeout.\n\n");
		return 2;
	}
	
	timeout = atoi(argv[1]);
	if (timeout < 1)
	{
		fprintf(stderr, "Invalid timeout value \"%s\". Timeout must be a positive integer.\n", argv[1]);
		return 2;
	}
	
	/* Set the timeout */
	alarm(timeout);
	
	/* Execute the command */
	ret = execvp(argv[2], &argv[2]);
	if (ret == -1)
	{
		/* Application was not launched */
		perror("Unable to run child application");
		return 2;
	}
	
	assert(0);
	perror("Should not see me");
	return 2;
}
		
