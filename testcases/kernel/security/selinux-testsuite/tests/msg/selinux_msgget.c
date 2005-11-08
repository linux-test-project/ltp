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
#include <sys/msg.h>

int main(int argc, char **argv)
{
	int ch;
	int key = 0x8888;
	int id;
	int perms = 0444;

	while ((ch = getopt(argc, argv, "k:p:")) != -1) {
		switch ((char)ch) {
		case 'k':
			key = atoi(optarg);
			break;
		case 'p':
			perms = atoi(optarg);
			break;
		}

	}

	id = msgget(key, IPC_CREAT|perms);
	if (id == -1)
	  return 1;
	printf("msgget succeeded: key = %d, id = %d\n", key, id);

	return 0;
}
