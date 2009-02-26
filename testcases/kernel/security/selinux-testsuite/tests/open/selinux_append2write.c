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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int fd, rc;

	if (argc != 2) {
		fprintf(stderr, "usage:  %s path", argv[0]);
		exit(1);
	}

	fd = open(argv[1], O_WRONLY | O_APPEND);
	if (fd < 0) {
		perror(argv[1]);
		exit(1);
	}

	rc = fcntl(fd, F_SETFL, 0);
	if (rc < 0) {
		perror("fcntl");
		exit(1);
	}
	close (fd);
	exit(0);
}
