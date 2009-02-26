/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

int main(int argc, char *argv[])
{
	pid_t pid;
	int policy;
	struct sched_param p;

	if (argc != 4) {
		fprintf(stderr,"Usage: %s pid policy priority\n",argv[0]);
		exit(-1);
	}
	pid = atol(argv[1]);
	policy = atol(argv[2]);
	if (policy > 2) {
		policy = SCHED_OTHER;	/* the default scheduler */
	}
	p.sched_priority = atol(argv[3]);
	printf("pid = %d \t policy = %d \t priority = %d\n",
		pid,policy,p.sched_priority);
	if (sched_setscheduler(pid,policy,&p) < 0) {
		perror("sched_setscheduler");
		exit(1);
	}
	
	exit(0);
}
