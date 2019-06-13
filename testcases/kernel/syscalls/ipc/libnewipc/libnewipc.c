// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * DESCRIPTION
 * common routines for the IPC system call tests.
 */

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#define	TST_NO_DEFAULT_MAIN

#include "tst_test.h"
#include "libnewipc.h"

#define BUFSIZE 1024

key_t getipckey(const char *file, const int lineno)
{
	char buf[BUFSIZE];
	key_t key;
	int id;
	static int count;

	SAFE_GETCWD(buf, BUFSIZE);

	id = count % 26 + (int) 'a';
	count++;

	key = ftok(buf, id);
	if (key == -1) {
		tst_brk(TBROK | TERRNO,
			"ftok() failed at %s:%d", file, lineno);
	}

	return key;
}

int get_used_queues(const char *file, const int lineno)
{
	FILE *fp;
	int used_queues = -1;
	char buf[BUFSIZE];

	fp = fopen("/proc/sysvipc/msg", "r");
	if (fp == NULL) {
		tst_brk(TBROK | TERRNO,
			"fopen() failed at %s:%d", file, lineno);
	}

	while (fgets(buf, BUFSIZE, fp) != NULL)
		used_queues++;

	fclose(fp);

	if (used_queues < 0) {
		tst_brk(TBROK, "can't read /proc/sysvipc/msg to get "
			"used message queues at %s:%d", file, lineno);
	}

	return used_queues;
}

void *probe_free_addr(const char *file, const int lineno)
{
	void *addr;
	int shm_id = -1;
	key_t probe_key = 0;

	probe_key = GETIPCKEY();

	shm_id = shmget(probe_key, SHMLBA * 2, SHM_RW | IPC_CREAT | IPC_EXCL);
	if (shm_id == -1)
		tst_brk(TBROK, "probe: shmget() failed at %s:%d", file, lineno);

	addr = shmat(shm_id, NULL, 0);
	if (addr == (void *) -1)
		tst_brk(TBROK, "probe: shmat() failed at %s:%d", file, lineno);

	if (shmdt(addr) == -1)
		tst_brk(TBROK, "probe: shmdt() failed at %s:%d", file, lineno);

	if (shmctl(shm_id, IPC_RMID, NULL) == -1)
		tst_brk(TBROK, "probe: shmctl() failed at %s:%d", file, lineno);

	addr = (void *)(((unsigned long)(addr) + (SHMLBA - 1)) & ~(SHMLBA - 1));

	return addr;
}
