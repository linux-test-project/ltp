/*
 * zram: generic RAM based compressed R/W block devices
 * http://lkml.org/lkml/2010/8/9/227
 *
 * Copyright (C) 2010  Red Hat, Inc.
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

char *TCID = "zram01";
int TST_TOTAL = 1;
int modprobe = 0;

#define PATH_ZRAM	"/sys/block/zram0"
#define SIZE		(512 * 1024 * 1024L)
#define DEVICE		"/dev/zram0"

static void set_disksize(void);
static void write_device(void);
static void verify_device(void);
static void reset(void);
static void setup(void);
static void cleanup(void);
static void print(char *string);
static void dump_info(void);

int main(int argc, char *argv[])
{
	int lc;
	char *msg;

	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		set_disksize();

		write_device();
		dump_info();
		verify_device();

		reset();
		dump_info();
	}
	cleanup();
	tst_exit();
}

static void set_disksize(void)
{
	int fd;
	char size[BUFSIZ];

	tst_resm(TINFO, "create a zram device with %ld bytes in size.", SIZE);
	fd = open(PATH_ZRAM "/disksize", O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open %s",
				PATH_ZRAM "/disksize");
	sprintf(size, "%ld", SIZE);
	if (write(fd, size, strlen(size)) != strlen(size))
		tst_brkm(TBROK|TERRNO, cleanup, "write %s to %s", size,
				PATH_ZRAM "/disksize");
	close(fd);
}

static void write_device(void)
{
	int fd;
	char *s;

	tst_resm(TINFO, "map it into memory.");
	fd = open(DEVICE, O_RDWR);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open %s", DEVICE);
	s = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (s == MAP_FAILED)
		tst_brkm(TBROK|TERRNO, cleanup, "mmap");

	tst_resm(TINFO, "write all the memory.");
	memset(s, 'a', SIZE-1);
	s[SIZE-1] = '\0';

	if (munmap(s, SIZE) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "munmap");

	close(fd);
}

static void verify_device(void)
{
	int fd;
	long i, fail;
	char *s;

	tst_resm(TINFO, "verify contents from device.");
	fd = open(DEVICE, O_RDONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open %s", DEVICE);
	s = mmap(NULL, SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
	if (s == MAP_FAILED)
		tst_brkm(TBROK|TERRNO, cleanup, "2nd mmap");

	i = 0;
	fail = 0;
	while (s[i] && i < SIZE - 1) {
		if (s[i] != 'a')
			fail++;
		i++;
	}
	if (i != SIZE-1)
		tst_resm(TFAIL, "expect size: %ld, actual size: %ld.",
				SIZE-1, i);
	else if (s[i] != '\0')
		tst_resm(TFAIL, "zram device seems not null terminated");
	if (fail)
		tst_resm(TFAIL, "%ld failed bytes found.", fail);
	if (munmap(s, SIZE) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "2nd munmap");

	close(fd);
}

static void reset(void)
{
	int fd;

	tst_resm(TINFO, "reset it.");
	fd = open(PATH_ZRAM "/reset", O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open %s",
				PATH_ZRAM "/reset");
	if (write(fd, "1", 1) != 1)
		tst_brkm(TBROK|TERRNO, cleanup, "write 1 to %s",
				PATH_ZRAM "/reset");
	close(fd);
}

static void setup(void)
{
	int retried = 0;

	tst_require_root(NULL);

retry:
	if (access(PATH_ZRAM, R_OK|W_OK|X_OK) == -1) {
		if (errno == ENOENT) {
			if (retried)
				tst_brkm(TCONF, NULL,
					"system has no zram device.");
			system("modprobe zram");
			modprobe = 1;
			retried  = 1;
			goto retry;
		} else
			tst_brkm(TBROK|TERRNO, NULL, "access");
	}

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
}

static void cleanup(void)
{
	if (modprobe == 1)
		system("rmmod zram");

	TEST_CLEANUP;
}

static void print(char *string)
{
	FILE *fp;
	char buf[BUFSIZ], value[BUFSIZ];

	sprintf(buf, "%s/%s", PATH_ZRAM, string);
	fp = fopen(buf, "r");
	if (fp == NULL)
		tst_brkm(TBROK|TERRNO, cleanup, "fopen %s", buf);

	if (fgets(value, BUFSIZ, fp) == NULL)
		tst_brkm(TBROK|TERRNO, cleanup, "fgets %s", buf);
	value[strlen(value) - 1] = '\0';
	fclose(fp);

	tst_resm(TINFO, "%s is %s", buf, value);
}

static void dump_info(void)
{
	print("initstate");
	print("compr_data_size");
	print("orig_data_size");
	print("disksize");
	print("mem_used_total");
	print("num_reads");
	print("num_writes");
	print("zero_pages");
}
