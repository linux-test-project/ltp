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
#include "tst_safe_stdio.h"
#include "tst_safe_sysv_ipc.h"
#include "tst_clocks.h"

#define BUFSIZE 1024

key_t getipckey(const char *file, const int lineno)
{
	char buf[BUFSIZE];
	key_t key;
	int id;
	static int count;

	safe_getcwd(file, lineno, NULL, buf, BUFSIZE);

	id = count % 26 + (int) 'a';
	count++;

	key = ftok(buf, id);
	if (key == -1) {
		tst_brk(TBROK | TERRNO,
			"ftok() failed at %s:%d", file, lineno);
	}

	return key;
}

int get_used_sysvipc(const char *file, const int lineno, const char *sysvipc_file)
{
	FILE *fp;
	int used = -1;
	char buf[BUFSIZE];

	fp = safe_fopen(file, lineno, NULL, sysvipc_file, "r");

	while (fgets(buf, BUFSIZE, fp) != NULL)
		used++;

	fclose(fp);

	if (used < 0) {
		tst_brk(TBROK, "can't read %s to get used sysvipc resource total at "
			"%s:%d", sysvipc_file, file, lineno);
	}

	return used;
}

void *probe_free_addr(const char *file, const int lineno)
{
	void *addr;
	int shm_id = -1;
	key_t probe_key = 0;

	probe_key = GETIPCKEY();

	shm_id = safe_shmget(file, lineno, probe_key, SHMLBA * 2,
			     SHM_RW | IPC_CREAT | IPC_EXCL);
	addr = safe_shmat(file, lineno, shm_id, NULL, 0);
	safe_shmdt(file, lineno, addr);
	safe_shmctl(file, lineno, shm_id, IPC_RMID, NULL);

	addr = (void *)(((unsigned long)(addr) + (SHMLBA - 1)) & ~(SHMLBA - 1));

	return addr;
}

time_t get_ipc_timestamp(void)
{
	struct timespec ts;
	int ret;

	ret = tst_clock_gettime(CLOCK_REALTIME_COARSE, &ts);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "clock_gettime(CLOCK_REALTIME_COARSE)");

	return ts.tv_sec;
}
