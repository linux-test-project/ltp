/*
 * Copyright (c) International Business Machines  Corp., 2009
 *
 * Authors:
 * Mimi Zohar <zohar@us.ibm.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 *
 * File: ima_mmap.c
 *
 * Open and mmap a file and sleep. Another process will open the
 * mmapped file in read mode, resulting in a open_writer violation.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "test.h"

char *TCID = "ima_mmap";
int TST_TOTAL = 1;

int main(int argc, char *argv[])
{
	int fd;
	void *file;
	char *filename;

	if (argc != 2)
		printf("%s: filename\n", argv[1]);
	filename = argv[1];

	fd = open(filename, O_CREAT | O_RDWR, S_IRWXU);
	if (fd < 0) {
		perror("open");
		return (-1);
	}

	file = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (file == (void *)-1) {
		perror("mmap");
		return (-1);
	}
	close(fd);
	sleep(30);
	if (munmap(file, 1024) < 0) {
		perror("unmap");
		return (-1);
	}
	tst_exit();
}
