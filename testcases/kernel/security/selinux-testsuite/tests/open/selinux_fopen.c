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
	FILE *fp;
	if (argc != 3) {
		fprintf(stderr, "usage:  %s path mode", argv[0]);
		exit(1);
	}
	fp = fopen(argv[1], argv[2]);
	if (!fp) {
		perror(argv[1]);
		exit(1);
	}
	fclose(fp);
	exit(0);
}