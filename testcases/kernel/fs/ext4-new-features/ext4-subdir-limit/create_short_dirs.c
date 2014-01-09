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
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "config.h"

/* valid characters for a directory name */
char chars[] = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";

/* to store the generated directory name */
char name[10];
int names;
int parent_fd;

/* NCHARS = 10 + 26 + 26 = 62 */
#define NCHARS		62
#define MAX_LEN1	62
#define MAX_LEN2	(62 * 62)
#define MAX_LEN3	(62 * 62 * 62)
#define MAX_NAMES	(MAX_LEN1 + MAX_LEN2 + MAX_LEN3)

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
 * create_1 - create length-1 directory names
 * @n: how name names to be created
 */
void create_1(int n)
{
	int i;

	name[1] = '\0';
	for (i = 0; i < NCHARS; i++) {
		name[0] = chars[i];
		create_dir();
		if (--n == 0)
			return;
	}
}

/*
 * create_2 - generate length-2 directory names
 * @n: how many names to be created
 */
void create_2(int n)
{
	int i, j;

	name[2] = '\0';
	for (i = 0; i < NCHARS; i++) {
		name[0] = chars[i];
		for (j = 0; j < NCHARS; j++) {
			name[1] = chars[j];
			create_dir();
			if (--n == 0)
				return;
		}
	}
}

/*
 * create_3 - generate length-3 directory names
 * @n: how many names to be created
 */
void create_3(int n)
{
	int i, j, k;

	name[3] = '\0';
	for (i = 0; i < NCHARS; i++) {
		name[0] = chars[i];
		for (j = 0; j < NCHARS; j++) {
			name[1] = chars[j];
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
	fprintf(stderr, "Usage: create_short_dirs nr_dirs parent_dir\n");
}

/*
 * Create short-name directoriess
 * @argv[1]: director number
 * @argv[2]: the parent directory
 */
int main(int argc, char *argv[])
{
	if (argc != 3) {
		usage();
		return 1;
	}

	names = atoi(argv[1]);
	if (names > MAX_NAMES || names <= 0) {
		usage();
		return 1;
	}

	parent_fd = open(argv[2], O_RDONLY);
	if (parent_fd == -1) {
		perror("open parent dir");
		return 1;
	}

	create_1(names);
	if (names <= MAX_LEN1)
		return 0;

	names -= MAX_LEN1;
	create_2(names);
	if (names <= MAX_LEN2)
		return 0;

	names -= MAX_LEN2;
	create_3(names);

	return 0;
}
