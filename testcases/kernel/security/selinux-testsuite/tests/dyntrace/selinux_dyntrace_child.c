/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <stdlib.h>
#include <selinux/selinux.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int rc;

	rc = setcon(argv[1]);
	if (rc < 0) {
		perror("setcon");
		exit(1);
	}
	exit(0);
}