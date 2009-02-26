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
	int error;
	struct msqid_ds buf;

	while ((ch = getopt(argc, argv, "k:")) != -1) {
		switch ((char)ch) {
		case 'k':
			key = atoi(optarg);
			break;
		}
	}

	id = msgget(key, IPC_CREAT|0444);
	if (id == -1)
		return 1;

	/*
	 * Equivalent: IPC_STAT, MSG_STAT
	 * Tests:      MSGQ__GETATTR | MSGQ__ASSOCIATE
	 */
	error = msgctl(id, IPC_STAT, &buf);
	printf ("%d", error);

	/*
	 * Equivalent: IPC_SET
	 * Tests:      MSGQ__SETATTR
	 */
	error = msgctl(id, IPC_SET, &buf);
	printf (" %d", error);

	/*
	 * Equivalent: IPC_RMID
	 * Tests:      MSGQ__DESTROY
	 */
	error = msgctl(id, IPC_RMID, 0);
	printf (" %d", error);

	printf("\n");
	return 0;
}
