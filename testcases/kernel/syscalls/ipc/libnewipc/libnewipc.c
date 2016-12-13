/*
 * Copyright (c) 2016 Xiao Yang <yangx.jy@cn.fujitsu.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
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
