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
#include <sys/msg.h>

#define MSGMAX 1024

struct msgbuf {
  long mtype;     /* message type, must be > 0 */
  char mtext[1024];  /* message data */
};

int main(int argc, char **argv)
{
	int opt;
	int key = 0x8888;
	int id;
	int error;
	struct msgbuf msgp;

	while ((opt = getopt(argc, argv, "k:")) != -1) {
		switch ((char)opt) {
		case 'k':
			key = atoi(optarg);
			break;
		}
	}

	id = msgget(key, IPC_CREAT|0777);
	if (id == -1)
	  return 1;

	error = msgrcv(id, &msgp, MSGMAX, 1, IPC_NOWAIT);
	printf("msgrcv: result = %d\n", error);
	return error;
}
