/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2005, 2008
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * NAME
 *      testpi-0.c
 *
 * DESCRIPTION
 *      This testcase checks whether priority inheritance support is present
 *      in the running kernel
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *
 *
 * HISTORY
 *      2010-04-22 Code cleanup by Gowrishankar (gowrishankar.m@in.ibm.com)
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include "librttest.h"

void usage(void)
{
	rt_help();
	printf("testpi-0 specific options:\n");
	printf("testpi-0 doesn't require any commandline options\n");
}

int parse_args(int c, char *v)
{

	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	default:
		handled = 0;
		break;
	}
	return handled;
}

/*
 * Test pthread creation at different thread priorities.
 */
int main(int argc, char *argv[])
{
	char *pathbuf;
	size_t n;

	rt_init("h", parse_args, argc, argv);

	n = confstr(_CS_GNU_LIBC_VERSION, NULL, (size_t) 0);
	pathbuf = malloc(n);
	if (!pathbuf)
		abort();
	confstr(_CS_GNU_LIBC_VERSION, pathbuf, n);

	printf("LIBC_VERSION: %s\n", pathbuf);
	free(pathbuf);

	n = confstr(_CS_GNU_LIBPTHREAD_VERSION, NULL, (size_t) 0);
	pathbuf = malloc(n);
	if (!pathbuf)
		abort();
	confstr(_CS_GNU_LIBPTHREAD_VERSION, pathbuf, n);

	printf("LIBPTHREAD_VERSION: %s\n", pathbuf);
	free(pathbuf);

	if (sysconf(_SC_THREAD_PRIO_INHERIT) == -1)
		printf("No Prio inheritance support\n");

	printf("Prio inheritance support present\n");

	return 0;
}
