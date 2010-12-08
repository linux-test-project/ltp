/*
 * Test soft page offline for process pages using madvise injector.
 * Requires special injection support in the kernel.
 * 
 * Copyright 2009 Intel Corporation
 *
 * tsoftinj is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; version
 * 2.
 *
 * tinjpage is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should find a copy of v2 of the GNU General Public License somewhere
 * on your Linux system; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 *
 * Author: Andi Kleen
 */
#define _GNU_SOURCE 1
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include "hugepage.h"

#define MADV_SOFT_OFFLINE 101

#define TMPDIR "./"

int PS;
int exitcode;
char empty[4096];
int corrupted;

void *checked_mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset)
{
	void *p = mmap(addr, length, prot, flags, fd, offset);
	if (p == (void *)-1L)
		err("mmap");
	return p;
}

unsigned meminfo(char *fmt)
{
	int found = 0;
	FILE *f = fopen("/proc/meminfo", "r");
	if (!f) err("open /proc/meminfo");
	char *line = NULL;
	size_t linelen = 0;
	unsigned val = 0;
	while (getline(&line, &linelen, f) > 0) {
		if (sscanf(line, fmt, &val) == 1) { 
			found = 1;
			break;
		}
	}
	free(line);
	fclose(f);
	if (!found)  {
		printf("cannot read HardwareCorruptedPages in meminfo\n");
		exitcode = 1;
	}
	return val;
}

unsigned hardware_corrupted(void)
{
	return (meminfo("HardwareCorrupted: %u") * 1024) / PS;
}

char *ndesc(char *buf, char *a, char *b)
{
	snprintf(buf, 100, "%s %s", a, b);
	return buf;
}

void offline(char *name, void *p)
{
	char buf[100];
	if (madvise(p, PS, MADV_SOFT_OFFLINE) < 0)
		err(ndesc(buf, name, "offline"));
	corrupted++;
}

void disk_backed(char *name, int flags)
{
	char fn[100];
	snprintf(fn, sizeof fn, TMPDIR "~test%u", getpid());
	printf("shared, diskbacked\n");
	int fd = open(fn, O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (fd < 0) err("open tmpfile");
	write(fd, empty, sizeof empty);
	char *p = checked_mmap(NULL, PS, PROT_READ|PROT_WRITE, 
			MAP_SHARED|flags, fd, 0);
	*(volatile int *)p = 1;
	offline(ndesc(fn, "disk backed", name), p);
	munmap(p, PS);
}

void anonymous(char *name, int flags)
{
	char buf[100];
	char *p = checked_mmap(NULL, PS, PROT_READ|PROT_WRITE, 
			MAP_PRIVATE|MAP_ANONYMOUS|flags, 0, 0);
	printf("anonymous\n");
	*(volatile int *)p = 1;
	offline(ndesc(buf, "anonymous", name), p);
	*(volatile int *)p = 1;
	munmap(p, PS);
}

void shm_hugepage(char *name, int flags)
{
	int shmid = 0;
	char buf[100];
	char *p = alloc_shm_hugepage(&shmid, HPS);
	if (!p)
		errmsg("failed in alloc_shm_hugepage\n");
	printf("shm hugepage\n");
	*(volatile int *)p = 1;
	offline(ndesc(buf, "shm hugepage", name), p);
	*(volatile int *)p = 1;
	free_shm_hugepage(shmid, p);
}

void anonymous_hugepage(char *name, int flags)
{
	char buf[100];
	char *p = alloc_anonymous_hugepage(HPS, 1);
	printf("anonymous hugepage\n");
	*(volatile int *)p = 1;
	offline(ndesc(buf, "anonymous hugepage", name), p);
	*(volatile int *)p = 1;
	free_anonymous_hugepage(p, HPS);
}

void filebacked_hugepage(char *name, int flags)
{
	int fd;
	char path[100];
	char fn[100];
	snprintf(path, sizeof path, "%s/~test-hugepage%u",
		 hugetlbfsdir, getpid());
	char *p = alloc_filebacked_hugepage(path, HPS, 0, &fd);
	printf("file backed hugepage\n");
	*(volatile int *)p = 1;
	offline(ndesc(fn, "file backed hugepage", name), p);
	*(volatile int *)p = 1;
	free_filebacked_hugepage(p, HPS, fd, path);
}

void check(unsigned *count, char *name, unsigned expected)
{
	unsigned count2 = hardware_corrupted();
	unsigned diff = count2 - *count;
	if (diff != expected) {
		printf("%s: expected %d corrupted pages, got %u\n", name,
			expected,
			diff);	
		if (diff < expected)
			exitcode = 1;
	}
	*count = count2;
	corrupted = 0;
}

int main(void)
{
	PS = getpagesize();
	HPS = gethugepagesize();

	unsigned count = hardware_corrupted();
	if (!hugetlbfs_root(hugetlbfsdir))
		err("hugetlbfs_root");
	anonymous("anonymous", 0);
	check(&count, "anonymous", 1);
	anonymous("anonymous mlock", MAP_LOCKED);
	check(&count, "anonymous mlock", 1);
	disk_backed("disk backed", 0);
	check(&count, "disk backed", 1);
	disk_backed("disk backed mlock", 0);
	check(&count, "disk backed mlock", 1);
	shm_hugepage("shm hugepage", 0);
	check(&count, "shm hugepage", HPS / PS);
	anonymous_hugepage("anonymous hugepage", 0);
	check(&count, "anonymous hugepage", HPS / PS);
	filebacked_hugepage("file backed hugepage", 0);
	check(&count, "file backed hugepage", HPS / PS);
	// add more test cases here

	return exitcode;
}
