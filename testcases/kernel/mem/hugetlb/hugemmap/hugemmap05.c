/*
 * Copyright (C) 2010-2017  Red Hat, Inc.
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
 * overcommit hugetlbfs and check the statistics.
 *
 * Description:
 *
 * hugetlbfs allows to overcommit hugepages and there are tunables in
 * sysfs and procfs. The test here want to ensure it is possible to
 * overcommit by either mmap or shared memory. Also ensure those
 * reservation can be read/write, and several statistics work correctly.
 *
 * First, it resets nr_hugepages and nr_overcommit_hugepages. Then, set
 * both to a specify value - N, and allocate N + %50 x N hugepages.
 * Finally, it reads and writes every page. There are command options to
 * choose either to manage hugepages from sysfs or procfs, and reserve
 * them by mmap or shmget.
 */

#include <sys/mount.h>
#include "mem.h"
#include "hugetlb.h"
#include "tst_safe_stdio.h"

#define PROTECTION		(PROT_READ | PROT_WRITE)
#define PATH_MEMINFO		"/proc/meminfo"

char path_sys_sz[BUFSIZ];
char path_sys_sz_over[BUFSIZ];
char path_sys_sz_free[BUFSIZ];
char path_sys_sz_resv[BUFSIZ];
char path_sys_sz_surp[BUFSIZ];
char path_sys_sz_huge[BUFSIZ];

#define PATH_PROC_VM		"/proc/sys/vm/"
#define PATH_PROC_OVER		PATH_PROC_VM "nr_overcommit_hugepages"
#define PATH_PROC_HUGE		PATH_PROC_VM "nr_hugepages"
#define PATH_SHMMAX		"/proc/sys/kernel/shmmax"

/* Only ia64 requires this */
#ifdef __ia64__
#define ADDR (void *)(0x8000000000000000UL)
#define FLAGS (MAP_SHARED | MAP_FIXED)
#define SHMAT_FLAGS (SHM_RND)
#else
#define ADDR (void *)(0x0UL)
#define FLAGS (MAP_SHARED)
#define SHMAT_FLAGS (0)
#endif

#ifndef SHM_HUGETLB
#define SHM_HUGETLB 04000
#endif

static char nr_hugepages[BUFSIZ], nr_overcommit_hugepages[BUFSIZ];
static char buf[BUFSIZ], line[BUFSIZ], path[BUFSIZ], pathover[BUFSIZ];
static char shmmax[BUFSIZ];
static int hugepagesize;	/* in Bytes */
static int shmid = -1;
static int restore_shmmax;
static size_t size = 128, length = 384;

char *opt_sysfs;
char *opt_alloc;
char *opt_shmid;
static struct tst_option options[] = {
	{"s",  &opt_sysfs, "-s        Setup hugepages from sysfs"},
	{"m",  &opt_shmid, "-m        Reserve hugepages by shmget"},
	{"a:", &opt_alloc, "-a        Number of overcommint hugepages"},
	{NULL, NULL, NULL}
};

static void write_bytes(void *addr);
static void read_bytes(void *addr);
static int lookup(char *line, char *pattern);
static int checkproc(FILE * fp, char *string, int value);
static int checksys(char *path, char *pattern, int value);
static void init_sys_sz_paths(void);

static void test_overcommit(void)
{
	void *addr = NULL, *shmaddr = NULL;
	int fd = -1, key = -1;
	char s[BUFSIZ];
	FILE *fp;

	if (shmid != -1) {
		/* Use /proc/meminfo to generate an IPC key. */
		key = ftok(PATH_MEMINFO, strlen(PATH_MEMINFO));
		if (key == -1)
			tst_brk(TBROK | TERRNO, "ftok");
		shmid = shmget(key, (long)(length / 2 * hugepagesize),
			       SHM_HUGETLB | IPC_CREAT | SHM_R | SHM_W);
		if (shmid == -1)
			tst_brk(TBROK | TERRNO, "shmget");
	} else {
		/* XXX (garrcoop): memory leak. */
		snprintf(s, BUFSIZ, "%s/hugemmap05/file", tst_get_tmpdir());
		fd = SAFE_OPEN(s, O_CREAT | O_RDWR, 0755);
		addr = mmap(ADDR, (long)(length / 2 * hugepagesize), PROTECTION,
			    FLAGS, fd, 0);
		if (addr == MAP_FAILED) {
			close(fd);
			tst_brk(TBROK | TERRNO, "mmap");
		}
	}

	if (opt_sysfs) {
		tst_res(TINFO, "check sysfs before allocation.");
		if (checksys(path_sys_sz_huge, "HugePages_Total",
			     length / 2) != 0)
			return;
		if (checksys(path_sys_sz_free, "HugePages_Free",
			     length / 2) != 0)
			return;
		if (checksys(path_sys_sz_surp, "HugePages_Surp",
			     length / 2 - size) != 0)
			return;
		if (checksys(path_sys_sz_resv, "HugePages_Rsvd",
			     length / 2) != 0)
			return;
	} else {
		tst_res(TINFO, "check /proc/meminfo before allocation.");
		fp = SAFE_FOPEN(PATH_MEMINFO, "r");
		if (checkproc(fp, "HugePages_Total", length / 2) != 0)
			return;
		if (checkproc(fp, "HugePages_Free", length / 2) != 0)
			return;
		if (checkproc(fp, "HugePages_Surp", length / 2 - size) != 0)
			return;
		if (checkproc(fp, "HugePages_Rsvd", length / 2) != 0)
			return;
		fclose(fp);
	}
	if (shmid != -1) {
		tst_res(TINFO, "shmid: 0x%x", shmid);
		shmaddr = shmat(shmid, ADDR, SHMAT_FLAGS);
		if (shmaddr == (void *)-1)
			tst_brk(TBROK | TERRNO, "shmat");
		write_bytes(shmaddr);
		read_bytes(shmaddr);
	} else {
		write_bytes(addr);
		read_bytes(addr);
	}
	if (opt_sysfs) {
		tst_res(TINFO, "check sysfs.");
		if (checksys(path_sys_sz_huge, "HugePages_Total",
			     length / 2) != 0)
			return;
		if (checksys(path_sys_sz_free, "HugePages_Free", 0)
		    != 0)
			return;
		if (checksys(path_sys_sz_surp, "HugePages_Surp",
			     length / 2 - size) != 0)
			return;
		if (checksys(path_sys_sz_resv, "HugePages_Rsvd", 0)
		    != 0)
			return;
	} else {
		tst_res(TINFO, "check /proc/meminfo.");
		fp = SAFE_FOPEN(PATH_MEMINFO, "r");
		if (checkproc(fp, "HugePages_Total", length / 2) != 0)
			return;
		if (checkproc(fp, "HugePages_Free", 0) != 0)
			return;
		if (checkproc(fp, "HugePages_Surp", length / 2 - size) != 0)
			return;
		if (checkproc(fp, "HugePages_Rsvd", 0) != 0)
			return;
		fclose(fp);
	}
	if (shmid != -1) {
		if (shmdt(shmaddr) != 0)
			tst_brk(TBROK | TERRNO, "shmdt");
	} else {
		munmap(addr, (long)(length / 2 * hugepagesize));
		close(fd);
		unlink(s);
	}

	tst_res(TPASS, "hugepages overcommit test pass");
}

static void cleanup(void)
{
	int fd;

	if (restore_shmmax) {
		fd = open(PATH_SHMMAX, O_WRONLY);
		if (fd == -1)
			tst_res(TWARN | TERRNO, "open");
		if (write(fd, shmmax, strlen(shmmax)) != (ssize_t)strlen(shmmax))
			tst_res(TWARN | TERRNO, "write");
		close(fd);
	}
	fd = open(path, O_WRONLY);
	if (fd == -1)
		tst_res(TWARN | TERRNO, "open");
	tst_res(TINFO, "restore nr_hugepages to %s.", nr_hugepages);
	if (write(fd, nr_hugepages,
		  strlen(nr_hugepages)) != (ssize_t)strlen(nr_hugepages))
		tst_res(TWARN | TERRNO, "write");
	close(fd);

	fd = open(pathover, O_WRONLY);
	if (fd == -1)
		tst_res(TWARN | TERRNO, "open");
	tst_res(TINFO, "restore nr_overcommit_hugepages to %s.",
		 nr_overcommit_hugepages);
	if (write(fd, nr_overcommit_hugepages, strlen(nr_overcommit_hugepages))
	    != (ssize_t)strlen(nr_overcommit_hugepages))
		tst_res(TWARN | TERRNO, "write");
	close(fd);

	/* XXX (garrcoop): memory leak. */
	snprintf(buf, BUFSIZ, "%s/hugemmap05", tst_get_tmpdir());
	if (umount(buf) == -1)
		tst_res(TWARN | TERRNO, "umount");
	if (shmid != -1) {
		tst_res(TINFO, "shmdt cleaning");
		shmctl(shmid, IPC_RMID, NULL);
	}
}

static void setup(void)
{
	FILE *fp;
	int fd;
	struct stat stat_buf;

	hugepagesize = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;
	init_sys_sz_paths();

	if (opt_shmid)
		shmid = 0;

	if (opt_sysfs) {
		strncpy(path, path_sys_sz_huge, strlen(path_sys_sz_huge) + 1);
		strncpy(pathover, path_sys_sz_over,
			strlen(path_sys_sz_over) + 1);
	} else {
		strncpy(path, PATH_PROC_HUGE, strlen(PATH_PROC_HUGE) + 1);
		strncpy(pathover, PATH_PROC_OVER, strlen(PATH_PROC_OVER) + 1);
	}
	if (opt_alloc) {
		size = atoi(opt_alloc);
		length = (int)(size + size * 0.5) * 2;
	}
	if (stat(pathover, &stat_buf) == -1) {
		if (errno == ENOENT || errno == ENOTDIR)
			tst_brk(TCONF,
				 "file %s does not exist in the system",
				 pathover);
	}

	if (shmid != -1) {
		fp = fopen(PATH_SHMMAX, "r");
		if (fp == NULL)
			tst_brk(TBROK | TERRNO, "fopen");
		if (fgets(shmmax, BUFSIZ, fp) == NULL)
			tst_brk(TBROK | TERRNO, "fgets");
		fclose(fp);

		if (atol(shmmax) < (long)(length / 2 * hugepagesize)) {
			restore_shmmax = 1;
			fd = open(PATH_SHMMAX, O_RDWR);
			if (fd == -1)
				tst_brk(TBROK | TERRNO, "open");
			snprintf(buf, BUFSIZ, "%ld",
				 (long)(length / 2 * hugepagesize));
			if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
				tst_brk(TBROK | TERRNO,
					 "failed to change shmmax.");
		}
	}
	fp = SAFE_FOPEN(path, "r+");
	if (fgets(nr_hugepages, BUFSIZ, fp) == NULL)
		tst_brk(TBROK | TERRNO, "fgets");
	fclose(fp);
	/* Remove trailing newline. */
	nr_hugepages[strlen(nr_hugepages) - 1] = '\0';
	tst_res(TINFO, "original nr_hugepages is %s", nr_hugepages);

	fd = SAFE_OPEN(path, O_RDWR);
	/* Reset. */
	SAFE_WRITE(1, fd, "0", 1);
	SAFE_LSEEK(fd, 0, SEEK_SET);
	snprintf(buf, BUFSIZ, "%zd", size);
	if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
		tst_brk(TBROK | TERRNO,
			"failed to change nr_hugepages.");
	close(fd);

	fp = SAFE_FOPEN(pathover, "r+");
	if (fgets(nr_overcommit_hugepages, BUFSIZ, fp) == NULL)
		tst_brk(TBROK | TERRNO, "fgets");
	fclose(fp);
	nr_overcommit_hugepages[strlen(nr_overcommit_hugepages) - 1] = '\0';
	tst_res(TINFO, "original nr_overcommit_hugepages is %s",
		 nr_overcommit_hugepages);

	fd = open(pathover, O_RDWR);
	if (fd == -1)
		tst_brk(TBROK | TERRNO, "open");
	/* Reset. */
	if (write(fd, "0", 1) != 1)
		tst_brk(TBROK | TERRNO, "write");
	if (lseek(fd, 0, SEEK_SET) == -1)
		tst_brk(TBROK | TERRNO, "lseek");
	snprintf(buf, BUFSIZ, "%zd", size);
	if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
		tst_brk(TBROK | TERRNO,
			 "failed to change nr_hugepages.");
	close(fd);

	/* XXX (garrcoop): memory leak. */
	snprintf(buf, BUFSIZ, "%s/hugemmap05", tst_get_tmpdir());
	if (mkdir(buf, 0700) == -1)
		tst_brk(TBROK | TERRNO, "mkdir");
	if (mount(NULL, buf, "hugetlbfs", 0, NULL) == -1)
		tst_brk(TBROK | TERRNO, "mount");
}

static void write_bytes(void *addr)
{
	long i;

	for (i = 0; i < (long)(length / 2 * hugepagesize); i++)
		((char *)addr)[i] = '\a';
}

static void read_bytes(void *addr)
{
	long i;

	tst_res(TINFO, "First hex is %x", *((unsigned int *)addr));
	for (i = 0; i < (long)(length / 2 * hugepagesize); i++) {
		if (((char *)addr)[i] != '\a') {
			tst_res(TFAIL, "mismatch at %ld", i);
			break;
		}
	}
}

/* Lookup a pattern and get the value from file */
static int lookup(char *line, char *pattern)
{
	char buf2[BUFSIZ];

	/* empty line */
	if (line[0] == '\0')
		return 0;

	snprintf(buf2, BUFSIZ, "%s: %%s", pattern);
	if (sscanf(line, buf2, buf) != 1)
		return 0;

	return 1;
}

static int checksys(char *path, char *string, int value)
{
	FILE *fp;

	fp = SAFE_FOPEN(path, "r");
	if (fgets(buf, BUFSIZ, fp) == NULL)
		tst_brk(TBROK | TERRNO, "fgets");
	tst_res(TINFO, "%s is %d.", string, atoi(buf));
	if (atoi(buf) != value) {
		tst_res(TFAIL, "%s is not %d but %d.", string, value,
			 atoi(buf));
		fclose(fp);
		return 1;
	}
	fclose(fp);
	return 0;
}

static int checkproc(FILE * fp, char *pattern, int value)
{
	memset(buf, -1, BUFSIZ);
	rewind(fp);
	while (fgets(line, BUFSIZ, fp) != NULL)
		if (lookup(line, pattern))
			break;

	tst_res(TINFO, "%s is %d.", pattern, atoi(buf));
	if (atoi(buf) != value) {
		tst_res(TFAIL, "%s is not %d but %d.", pattern, value,
			 atoi(buf));
		return 1;
	}
	return 0;
}

static void init_sys_sz_paths(void)
{
	sprintf(path_sys_sz, "/sys/kernel/mm/hugepages/hugepages-%dkB",
		hugepagesize / 1024);
	sprintf(path_sys_sz_over, "%s/nr_overcommit_hugepages", path_sys_sz);
	sprintf(path_sys_sz_free, "%s/free_hugepages", path_sys_sz);
	sprintf(path_sys_sz_resv, "%s/resv_hugepages", path_sys_sz);
	sprintf(path_sys_sz_surp, "%s/surplus_hugepages", path_sys_sz);
	sprintf(path_sys_sz_huge, "%s/nr_hugepages", path_sys_sz);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.options = options,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_overcommit,
};
