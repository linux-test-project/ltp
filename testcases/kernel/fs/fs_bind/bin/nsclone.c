/*
 * Copyright (c) International Business Machines  Corp., 2005
 * Author: Ram Pai (linuxram@us.ibm.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include "test.h"
#include <sys/types.h>
#include <sys/wait.h>

int myfunc(void *arg)
{
	return system(arg);
}

static void usage(char *cmd)
{
	printf("%s  child_script parent_script\n", cmd);
}

int main(int argc, char *argv[])
{
	char *child_cmd;
	char *parent_cmd;
	int ret = 0, childret = 0;

	if (argc < 3) {
		usage(argv[0]);
		exit(1);
	}

	child_cmd = (char *)strdup(argv[2]);
	parent_cmd = (char *)strdup(argv[1]);

	printf("1\n");
	ret = ltp_clone_quick(CLONE_NEWNS | SIGCHLD, myfunc, (void *)child_cmd);
	if (ret != -1) {
		system(parent_cmd);
		wait(&childret);
	} else {
		fprintf(stderr, "clone failed\n");
	}
	if (ret || !WIFEXITED(childret)) {
		exit(1);
	}
	exit(0);
}
