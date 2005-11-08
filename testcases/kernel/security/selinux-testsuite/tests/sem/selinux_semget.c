/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/sem.h>

int main(int argc, char **argv)
{
	int ch;
	int num = 1;
	int key = 0x8888;
	int id;

	while ((ch = getopt(argc, argv, "k:-n:")) != -1) {
		switch ((char)ch) {
		case 'k':
			key = atoi(optarg);
			break;
		case 'n':
			num = atoi(optarg);
			break;
		}
	}

	id = semget(key, num, IPC_CREAT|0777);
	if (id == -1)
		return 1;
	printf("semget succeeded: key = %d, id = %d\n", key, id);

	return 0;
}
