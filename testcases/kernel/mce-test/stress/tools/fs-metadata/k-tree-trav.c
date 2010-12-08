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
 * This program helps you to traverse each node in the k tree
 * Do the i-node operations on the all file entries in recursive
 *
 * Copyright (C) 2009, Intel Corp.
 * Author: Shaohui Zheng <shaohui.zheng@intel.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_PATH 8192

/*
 * Traverse a k-tree in recusive
 *
 * parameters:
 * lvl: tree level number from button to top
 * node_nr: the maximun nodes number
 * return val: if it is leaf, return 0, or return 1
 */

int k_tree_trav(int lvl, int node_nr)
{
	int cnt;
	char dir[MAX_PATH], cwd[MAX_PATH], f1[MAX_PATH], f2[MAX_PATH],
	    ln[MAX_PATH];
	if (lvl <= 0)
		return 0;

	for (cnt = 0; cnt < node_nr; cnt++) {
		sprintf(dir, "%d-d", cnt);
		sprintf(f1, "%d-f", cnt);
		sprintf(f2, "%d-f-t", cnt);
		sprintf(ln, "%d-l", cnt);

		// link and unlink testing for each file i-node
		link(f1, f2);
		unlink(f1);
		rename(f2, f1);

		// symlink testing
		symlink(ln, f1);
		unlink(ln);

		getcwd(cwd, sizeof(cwd));
		chmod(dir, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		chdir(dir);
		k_tree_trav(lvl - 1, node_nr);
		chdir(cwd);
	}

	return 1;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: %s tree_depth tree_width\n", argv[0]);
		return 1;
	}

	printf("Traverse k tree (depth: %s, width: %s)...\n", argv[1], argv[2]);
	k_tree_trav(atoi(argv[1]), atoi(argv[2]));
	printf("Traverse k tree (depth: %s, width: %s), done\n", argv[1],
	       argv[2]);
	return 0;
}
