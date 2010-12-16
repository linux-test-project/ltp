/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
	pid_t pid, group_id;

	pid = getpid();
	if ((group_id = getpgid(pid)) < 0) {
		perror("getpgid");
		exit(-1);
	}
	printf("Group ID = %d\n",group_id);
	if (setpgid(pid,pid) < 0) {
		perror("setpgid");
		exit(1);
	}
	if ((group_id = getpgid(pid)) < 0) {
		perror("getpgid");
		exit(-1);
	}
	printf("Group ID = %d\n",group_id);
	printf("pid = %d\n",pid);
	exit(0);
}