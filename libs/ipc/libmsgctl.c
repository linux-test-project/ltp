/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "libmsgctl.h"

int doreader(long key, int tid, long type, int child, int nreps)
{
	int i, size;
	int id;
	struct mbuffer buffer;

	id = msgget(key, 0);
	if (id < 0) {
		printf("msgget() error in the reader of child group %d: %s\n",
			child, strerror(errno));

		return FAIL;
	}
	if (id != tid) {
		printf("Message queue mismatch in the reader of child group %d for message queue id %d\n",
			child, id);

		return FAIL;
	}
	for (i = 0; i < nreps; i++) {
		memset(&buffer, 0, sizeof(buffer));

		size = msgrcv(id, &buffer, 100, type, 0);
		if (size < 0) {
			printf("msgrcv() error in child %d, read # = %d: %s\n",
				child, (i + 1), strerror(errno));

			return FAIL;
		}
		if (buffer.type != type) {
			printf("Type mismatch in child %d, read #d = %d: ",
				child, (i + 1));
			printf("for message got %ld, expected - %ld\n",
				buffer.type, type);

			return FAIL;
		}
		if (buffer.data.len + 1 != size) {
			printf("Size mismatch in child %d, read # = %d: ",
				child, (i + 1));
			printf("for message got %d, expected - %d\n",
				buffer.data.len + 1, size);

			return FAIL;
		}
		if (verify(buffer.data.pbytes, (key % 255), size - 1, child)) {
			printf("Verify failed in child %d read # = %d, key = %lx\n",
				child, (i + 1), key);

			return FAIL;
		}
		key++;
	}
	return PASS;
}

int dowriter(long key, int tid, long type, int child, int nreps)
{
	int i, size;
	int id;
	struct mbuffer buffer;

	id = msgget(key, 0);
	if (id < 0) {
		printf("msgget() error in the writer of child group %d: %s\n",
			child, strerror(errno));

		return FAIL;
	}
	if (id != tid) {
		printf("Message queue mismatch in the reader of child group %d for message queue id %d\n",
			child, id);

		return FAIL;
	}

	for (i = 0; i < nreps; i++) {
		memset(&buffer, 0, sizeof(buffer));

		do {
			size = (lrand48() % 99);
		} while (size == 0);
		fill_buffer(buffer.data.pbytes, (key % 255), size);
		buffer.data.len = size;
		buffer.type = type;
		if (msgsnd(id, &buffer, size + 1, 0) < 0) {
			printf("msgsnd() error in child %d, write # = %d, key = %lx: %s\n",
				child, nreps, key, strerror(errno));

			return FAIL;
		}
		key++;
	}
	return PASS;
}

int fill_buffer(char *buf, char val, int size)
{
	int i;

	for (i = 0; i < size; i++)
		buf[i] = val;
	return 0;
}

/* Check a buffer for correct values */
int verify(char *buf, char val, int size, int child)
{
	while (size-- > 0) {
		if (*buf++ != val) {
			printf("Verify error in child %d, *buf = %x, val = %x, size = %d\n",
				child, *buf, val, size);

			return FAIL;
		}
	}
	return PASS;
}
