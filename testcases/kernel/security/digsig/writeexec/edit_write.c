/*
 * Written by Serge E. Hallyn <serue@us.ibm.com>
 * Copyright (c) International Business Machines  Corp., 2005
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
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void perror(const char *s);

int main(int argc, char **argv)
{
	int fd;

	if (argc < 2)
		exit(-5);
	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		perror("failed !\n");
	} else {
		printf("succeeded!\n");
		sleep(10);
		close(fd);
	}
}
