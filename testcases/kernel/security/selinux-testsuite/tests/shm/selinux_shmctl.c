/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/shm.h>

int main(int argc, char **argv)
{
	int ch;
	int num = 1;
	int key = 0x8888;
	int id;
	int error;
	struct shmid_ds buf;

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

	id = shmget(key, 2048, IPC_CREAT|0777);
	if (id == -1)
	  return 1;

	/*
	 * Equivalent: IPC_STAT, SHM_STAT
	 * Tests:      SHM__GETATTR | SHM__ASSOCIATE
	 */
	error = shmctl(id, IPC_STAT, &buf);
	printf ("%d", error);

	/*
	 * Equivalent: IPC_SET
	 * Tests:      SHM__SETATTR
	 */
	error = shmctl(id, IPC_SET, &buf);
	printf (" %d", error);


	/*
	 * Equivalent: SHM_LOCK, SHM_UNLOCK
	 * Tests:      SHM__LOCK
	 */
	error = shmctl(id, SHM_LOCK, NULL);
	printf (" %d", error);

	/*
	 * Equivalent: IPC_RMID
	 * Tests:      SHM__DESTROY
	 */
	error = shmctl(id, IPC_RMID, NULL);
	printf (" %d", error);

	printf("\n");
	return 0;
}
