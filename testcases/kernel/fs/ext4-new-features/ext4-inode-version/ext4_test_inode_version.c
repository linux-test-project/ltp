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

#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

char *filename;
char *filepath;
int fd;

int old_inode_version;
int new_inode_version;

int get_inode_version(void)
{
	char buf[1024];

	sprintf(buf, "ext4_get_inode_version.sh %s 1", filename);

	/* sync before run debugfs to get inode version */
	sync();

	return WEXITSTATUS(system(buf));
}

void test_chmod(void)
{
	if (fchmod(fd, S_IRUSR | S_IWUSR)) {
		fprintf(stderr, "fchmod failed\n");
		exit(1);
	}
}

void test_chown(void)
{
	if (fchown(fd, 1, 1)) {
		fprintf(stderr, "fchown failed\n");
		exit(1);
	}
}

void test_read(void)
{
	char buf[2];

	/* write something before read */
	if (write(fd, "abc", 4) == -1) {
		perror("write");
		exit(1);
	}
	close(fd);

	if (open(filepath, O_RDONLY) == -1) {
		perror("open");
		exit(1);
	}

	old_inode_version = get_inode_version();

	if (read(fd, buf, 1) == -1) {
		perror("read");
		exit(1);
	}
}

void test_write(void)
{
	if (write(fd, "a", 1) == -1) {
		fprintf(stderr, "write failed\n");
		exit(1);
	}
}

void test_mmap_read(void)
{
	char *p;
	char c;

	/* write something before read */
	if (write(fd, "abc", 4) == -1) {
		perror("write");
		exit(1);
	}
	close(fd);

	if (open(filepath, O_RDONLY) == -1) {
		perror("open");
		exit(1);
	}

	old_inode_version = get_inode_version();

	p = mmap(NULL, 1, PROT_READ, MAP_PRIVATE | MAP_FILE, fd, 0);
	if (p == (void *)-1) {
		perror("mmap");
		exit(1);
	}
	c = *p;

	new_inode_version = get_inode_version();

	msync(p, 1, MS_SYNC);
}

void test_mmap_write(void)
{
	char *p;

	if (write(fd, "abc", 4) == -1) {
		perror("write");
		exit(1);
	}
	close(fd);

	if (open(filepath, O_RDWR) == -1) {
		perror("open");
		exit(1);
	}

	old_inode_version = get_inode_version();

	p = mmap(NULL, 1, PROT_WRITE, MAP_PRIVATE | MAP_FILE, fd, 0);
	if (p == (void *)-1) {
		perror("mmap");
		exit(1);
	}
	*p = 'x';

	new_inode_version = get_inode_version();

	msync(p, 1, MS_SYNC);
}

/**
 * argv[1]: file operation
 * argv[2]: file to test (with path)
 * argv[3]: file to test (without path)
 */
int main(int argc, char *argv[])
{
	if (argc != 4) {
		fprintf(stderr, "wrong argument number\n");
		return 1;
	}
	filepath = argv[2];
	filename = argv[3];

	/* create file and get the initial inode version */
	fd = creat(argv[2], O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "failed to create file: %s\n", argv[2]);
		return 1;
	}

	old_inode_version = get_inode_version();

	if (strcmp(argv[1], "create") == 0) {
		printf("%d\n", old_inode_version);
		return 0;
	} else if (strcmp(argv[1], "chmod") == 0) {
		test_chmod();
	} else if (strcmp(argv[1], "chown") == 0) {
		test_chown();
	} else if (strcmp(argv[1], "read") == 0) {
		test_read();
	} else if (strcmp(argv[1], "write") == 0) {
		test_write();
	} else if (strcmp(argv[1], "mmap_read") == 0) {
		test_mmap_read();
	} else if (strcmp(argv[1], "mmap_write") == 0) {
		test_mmap_write();
	} else {
		fprintf(stderr, "wrong file operation: %s\n", argv[1]);
		return 1;
	}

	new_inode_version = get_inode_version();
#if 0
	fprintf(stderr, "test_inode_version: old - %d\n", old_inode_version);
	fprintf(stderr, "test_inode_version: new - %d\n", new_inode_version);
#endif
	/* wrong inode version, test failed */
	if (new_inode_version <= old_inode_version)
		return 1;

	printf("%d\n", new_inode_version);

	close(fd);

	return 0;
}
