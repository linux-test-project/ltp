/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Description: This is a program invoked as nobody that will
 *		creat a testfile and a symlink to that testfile.
 *
 *		This program exits with 0 or 1 depending upon the
 *		success or failure of each system call.
 */
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#define FILE_MODE        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

int main(int ac, char **av)
{
	int fd;			/* file handle for testfile */

	/* Create a testfile under temporary directory */
	if ((fd = open(av[1], O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		perror("creat_slink: open() failed");
		exit(1);
	}

	/* Close the testfile created */
	if (close(fd) == -1) {
		perror("creat_slink: close() failed");
		exit(1);
	}

	/* Create a symlink of testfile under temporary directory */
	if (symlink(av[1], av[2]) < 0) {
		perror("creat_slink: symlink() failed");
		exit(1);
	}

	exit(0);
}