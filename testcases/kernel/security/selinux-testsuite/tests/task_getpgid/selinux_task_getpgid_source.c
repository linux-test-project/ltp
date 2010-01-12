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
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	pid_t pid, group_id;

	if (argc != 2) {
		fprintf(stderr,"Usage: %s pid\n",argv[0]);
		exit(-1);
	}
	pid = (pid_t) atol(argv[1]);
	printf("pid = %d \n",pid);
	if ((group_id = getpgid(pid)) < 0) {
		perror("getpgid");
		exit(1);
	}
	printf("group ID = %d\n",group_id);
	exit(0);
}
