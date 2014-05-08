/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xing Gu <gux.fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define STR "abc"

char *TCID = "openat02_child";

int main(int argc, char **argv)
{
	int fd;
	int ret;

	if (argc != 2) {
		fprintf(stderr, "%s <fd>\n", argv[0]);
		exit(1);
	}

	fd = atoi(argv[1]);
	ret = write(fd, STR, sizeof(STR) - 1);

	return ret != -1;
}
