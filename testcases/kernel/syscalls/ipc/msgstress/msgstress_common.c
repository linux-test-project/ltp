// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved。
 */

#define TST_NO_DEFAULT_MAIN
#include "msgstress_common.h"

int verify(char *buf, char val, int size, int child)
{
	while (size-- > 0) {
		if (*buf++ != val) {
			tst_res(TFAIL, "Verify error in child %d, *buf = %x, "
				"val = %x, size = %d\n", child, *buf, val,
				size);
			return 1;
		}
	}
	return 0;
}

void do_reader(long key, int tid, long type, int child, int nreps)
{
	int i, size;
	int id;
	struct mbuffer buffer;

	id = SAFE_MSGGET(key, 0);
	if (id != tid) {
		tst_res(TFAIL,
			"Message queue mismatch in the reader of child group"
			" %d for message queue id %d\n", child, id);
		return;
	}
	for (i = 0; i < nreps; i++) {
		memset(&buffer, 0, sizeof(buffer));

		size = SAFE_MSGRCV(id, &buffer, 100, type, 0);
		if (buffer.type != type) {
			tst_res(TFAIL, "Type mismatch in child %d, read #%d, "
				"for message got %ld, exected %ld",
				child, (i + 1), buffer.type, type);
			return;
		}
		if (buffer.data.len + 1 != size) {
			tst_res(TFAIL, "Size mismatch in child %d, read #%d, "
				"for message got %d, expected %d",
				child, (i + 1), buffer.data.len + 1, size);
			return;
		}
		if (verify(buffer.data.pbytes, (key % 255), size - 1, child)) {
			tst_res(TFAIL, "Verify failed in child %d read # = %d, "
				"key = %lx\n", child, (i + 1), key);
			return;
		}
		key++;
	}
}

void fill_buffer(char *buf, char val, int size)
{
	int i;

	for (i = 0; i < size; i++)
		buf[i] = val;
}

void do_writer(long key, int tid, long type, int child, int nreps)
{
	int i, size;
	int id;
	struct mbuffer buffer;

	id = SAFE_MSGGET(key, 0);
	if (id != tid) {
		tst_res(TFAIL, "Message queue mismatch in the reader of child"
			" group %d for message queue id %d\n", child, id);
		return;
	}

	for (i = 0; i < nreps; i++) {
		memset(&buffer, 0, sizeof(buffer));

		do {
			size = (lrand48() % 99);
		} while (size == 0);
		fill_buffer(buffer.data.pbytes, (key % 255), size);
		buffer.data.len = size;
		buffer.type = type;
		SAFE_MSGSND(id, &buffer, size + 1, 0);
		key++;
	}
}
