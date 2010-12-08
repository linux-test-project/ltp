/*
 * Metadata stress testing program for file system
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; version
 * 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should find a copy of v2 of the GNU General Public License somewhere
 * on your Linux system; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 *
 * This program helps you to generate a k-tree in recusive.
 *
 * A k-tree is a tranformation of binary tree, A binary has 2 
 * children at most in each node, but a k-tree has k sub-nodes. 
 * We test both file and directory entries, So we do some changes.
 * We create k sub directories and k text file in each parent.
 *
 * Copyright (C) 2009, Intel Corp.
 * Author: Shaohui Zheng <shaohui.zheng@intel.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_PATH 8192

/*
 * k tree generator.
 *
 * parameters:
 * lvl: tree level number from button to top
 * node_nr: the maximun nodes number
 * return val: if it is leaf, return 0, or return 1
 */

int k_tree_gen(int lvl, int node_nr)
{
	int cnt;
	char dir[MAX_PATH], cwd[MAX_PATH], leaf[MAX_PATH];
	if (lvl <= 0)
		return 0;

	for (cnt = 0; cnt < node_nr; cnt++) {
		int fd = 0, fd2;
		// generate dir name or file name
		sprintf(dir, "%d-d", cnt);
		sprintf(leaf, "%d-f", cnt);

		// create an empty file
		// API: open,close,dup,read,write,lseek
		fd = open(leaf, O_CREAT | O_RDWR);
		fd2 = dup(fd);
		close(fd);
		fd = fd2;
		write(fd, leaf, 3);
		lseek(fd, SEEK_SET, 0);
		read(fd, leaf, 3);
		close(fd);

		// create directory entry
		mkdir(dir, 0777);
		getcwd(cwd, sizeof(cwd));
		chdir(dir);
		k_tree_gen(lvl - 1, node_nr);
		chdir(cwd);
	}

	return 1;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: k-tree tree_depth tree_width\n");
		return 1;
	}

	printf("Generate k tree (depth: %s, width: %s) ...\n", argv[1],
	       argv[2]);
	k_tree_gen(atoi(argv[1]), atoi(argv[2]));
	printf("Generate k tree (depth: %s, width: %s), done\n", argv[1],
	       argv[2]);
	return 0;
}
