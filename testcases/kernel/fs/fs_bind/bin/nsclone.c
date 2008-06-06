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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef __ia64__
#define clone2 __clone2
extern int  __clone2(int (*fn) (void *arg), void *child_stack_base,
		     size_t child_stack_size, int flags, void *arg,
		     pid_t *parent_tid, void *tls, pid_t *child_tid);
#endif

char somemem[4096];

int myfunc(void *arg){
	return system(arg);
}

static void
usage(char *cmd)
{
	printf("%s  child_script parent_script\n",cmd);
}

int main(int argc, char *argv[])
{
	char *child_cmd;
	char *parent_cmd;
	int ret=0, childret=0;

	if ( argc < 3 ) {
		usage(argv[0]);
		exit(1);
	}

	child_cmd = (char *)strdup(argv[2]);
	parent_cmd = (char *)strdup(argv[1]);

	printf("1\n");
#ifdef __ia64__
	if (clone2(myfunc, somemem, getpagesize(), CLONE_NEWNS|SIGCHLD,
		   child_cmd, NULL, NULL, NULL) != -1) {
#else
	if (clone(myfunc, somemem, CLONE_NEWNS|SIGCHLD, child_cmd) != -1) {
#endif
		system(parent_cmd);
		wait(&childret);
	} else {
		fprintf(stderr, "clone failed\n");
	}
	if (ret || !WIFEXITED(childret)){
		exit(1);
	}
	exit(0);
}
