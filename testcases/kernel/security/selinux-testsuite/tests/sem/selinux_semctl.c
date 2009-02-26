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

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
	int val;                    /* value for SETVAL */
	struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
	unsigned short int *array;  /* array for GETALL, SETALL */
	struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif

int main(int argc, char **argv)
{
	int ch;
	int num = 1;
	int key = 0x8888;
	int id;
	int error;
	union semun arg;
	struct semid_ds semid_buf;

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

	/*
	 * Equivalent: IPC_STAT, SEM_STAT
	 * Tests:      SEM__GETATTR | SEM__ASSOCIATE
	 */
	arg.buf = &semid_buf;
	error = semctl(id, 0, IPC_STAT, arg);
	printf ("%d", error);

	/*
	 * Equivalent: GETPID, GETNCNT, GETZCNT
	 * Tests:      SEM__GETATTR
	 */
	error = semctl(id, 0, GETPID, NULL);
	printf (" %d", error);

	/*
	 * Equivalent: GETVAL, GETALL
	 * Tests:      SEM__READ
	 */
	error = semctl(id, 0, GETVAL, NULL);
	printf (" %d", error);

	/*
	 * Equivalent: SETVAL, SETALL
	 * Tests:      SEM__WRITE
	 */
	arg.val = 1;
	error = semctl(id, 0, SETVAL, arg);
	printf (" %d", error);

	/*
	 * Equivalent: IPC_SET
	 * Tests:      SEM__SETATTR
	 */
	arg.buf = &semid_buf;
	error = semctl(id, 0, IPC_SET, arg);
	printf (" %d", error);

	/*
	 * Equivalent: IPC_RMID
	 * Tests:      SEM__DESTROY
	 */
	error = semctl(id, 0, IPC_RMID, NULL);
	printf (" %d", error);

	printf("\n");
	return 0;
}
