/******************************************************************************/
/*                                                                            */
/* Copyright (c) 2009 FUJITSU LIMITED                                         */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/* Author: Li Zefan <lizf@cn.fujitsu.com>                                     */
/*                                                                            */
/******************************************************************************/

#define _ATFILE_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
//#define __USE_ATFILE
#include <sys/stat.h>
#include "config.h"

#define NAME_LEN	255
#define NCHARS		62
#define MAX_LEN1	62
#define MAX_LEN2	(62 * 62)
#define MAX_LEN3	(62 * 62 * 62)

/* valid characters for the directory name */
char chars[NCHARS + 1] =
    "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";

/* to store the generated directory name */
char name[NAME_LEN + 1];
int names;
int parent_fd;

/*
 * init_name - initialize the directory name
 *
 * Generate a randomized directory name, and then we generate more
 * directory names based on it.
 */
void init_name(void)
{
	int i;

	srand(time(NULL));

	for (i = 0; i < NAME_LEN; i++)
		name[i] = chars[rand() % 62];
}

void create_dir(void)
{
#ifdef HAVE_MKDIRAT
	if (mkdirat(parent_fd, name, S_IRWXU)) {
		perror("mkdir");
		exit(1);
	}
#else
	fprintf(stderr, "System lacks mkdirat() call.\n");
	exit(1);
#endif
}

/*
 * create_dirs - create @names directory names
 * @n: how many names to be created
 *
 * if n <= 62,       we need to modify 1 char of the name
 * if n <= 62*62,    we need to modify 2 chars
 * if n <= 62*62*62, we need to modify 3 chars
 */
void create_dirs(int n)
{
	int i, j, k;
	int depth;

	if (n <= MAX_LEN1)
		depth = 1;
	else if (n <= MAX_LEN2)
		depth = 2;
	else
		depth = 3;

	for (i = 0; i < NCHARS; i++) {
		name[0] = chars[i];
		if (depth == 1) {
			create_dir();
			if (--n == 0)
				return;
			continue;
		}

		for (j = 0; j < NCHARS; j++) {
			name[1] = chars[j];
			if (depth == 2) {
				create_dir();
				if (--n == 0)
					return;
				continue;
			}

			for (k = 0; k < NCHARS; k++) {
				name[2] = chars[k];
				create_dir();
				if (--n == 0)
					return;
			}
		}
	}
}

void usage()
{
	fprintf(stderr, "Usage: create_long_dirs nr_dirs parent_dir\n");
}

/*
 * Create long-name directories
 * @argv[1]: directory number
 * @argv[2]: parent directory
 */
int main(int argc, char *argv[])
{
	if (argc != 3) {
		usage();
		return 1;
	}

	names = atoi(argv[1]);
	if (names > MAX_LEN3 || names <= 0) {
		usage();
		return 1;
	}

	parent_fd = open(argv[2], O_RDONLY);
	if (parent_fd == -1) {
		perror("open parent dir failed");
		return 1;
	}

	init_name();

	create_dirs(names);

	return 0;
}
