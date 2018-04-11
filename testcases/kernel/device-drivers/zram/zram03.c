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
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "zram03";
int TST_TOTAL = 1;

#define PATH_ZRAM	"/sys/block/zram0"
#define SIZE		(512 * 1024 * 1024L)
#define DEVICE		"/dev/zram0"

static int modprobe;

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

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

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
	tst_resm(TINFO, "create a zram device with %ld bytes in size.", SIZE);
	SAFE_FILE_PRINTF(cleanup, PATH_ZRAM "/disksize", "%ld", SIZE);
}

static void write_device(void)
{
	int fd;
	char *s;

	tst_resm(TINFO, "map this zram device into memory.");
	fd = SAFE_OPEN(cleanup, DEVICE, O_RDWR);
	s = SAFE_MMAP(cleanup, NULL, SIZE, PROT_READ | PROT_WRITE,
		      MAP_SHARED, fd, 0);

	tst_resm(TINFO, "write all the memory.");
	memset(s, 'a', SIZE - 1);
	s[SIZE - 1] = '\0';

	SAFE_MUNMAP(cleanup, s, SIZE);
	SAFE_CLOSE(cleanup, fd);
}

static void verify_device(void)
{
	int fd;
	long i = 0, fail = 0;
	char *s;

	tst_resm(TINFO, "verify contents from device.");
	fd = SAFE_OPEN(cleanup, DEVICE, O_RDONLY);
	s = SAFE_MMAP(cleanup, NULL, SIZE, PROT_READ, MAP_PRIVATE, fd, 0);

	while (s[i] && i < SIZE - 1) {
		if (s[i] != 'a')
			fail++;
		i++;
	}
	if (i != SIZE - 1) {
		tst_resm(TFAIL, "expect size: %ld, actual size: %ld.",
			 SIZE - 1, i);
	} else if (s[i] != '\0') {
		tst_resm(TFAIL, "zram device seems not null terminated");
	} else if (fail) {
		tst_resm(TFAIL, "%ld failed bytes found.", fail);
	} else {
		tst_resm(TPASS, "data read from zram device is consistent "
			 "with those are written");
	}

	SAFE_MUNMAP(cleanup, s, SIZE);
	SAFE_CLOSE(cleanup, fd);
}

static void reset(void)
{
	tst_resm(TINFO, "reset it.");
	SAFE_FILE_PRINTF(cleanup, PATH_ZRAM "/reset", "1");
}

static void setup(void)
{
	int retried = 0;

	tst_require_root();

retry:
	if (access(PATH_ZRAM, F_OK) == -1) {
		if (errno == ENOENT) {
			if (retried) {
				tst_brkm(TCONF, NULL,
					 "system has no zram device.");
			}
			if (system("modprobe zram") == -1) {
				tst_brkm(TBROK | TERRNO, cleanup,
					 "system(modprobe zram) failed");
			}
			modprobe = 1;
			retried = 1;
			goto retry;
		} else
			tst_brkm(TBROK | TERRNO, NULL, "access");
	}

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
}

static void cleanup(void)
{
	if (modprobe == 1 && system("rmmod zram") == -1)
		tst_resm(TWARN | TERRNO, "system(rmmod zram) failed");
}

static void print_io_stat(void)
{
	char filename[BUFSIZ];
	char  val1[BUFSIZ], val2[BUFSIZ], val3[BUFSIZ], val4[BUFSIZ];

	sprintf(filename, "%s/io_stat", PATH_ZRAM);
	SAFE_FILE_SCANF(cleanup, filename, "%s %s %s %s",
			val1, val2, val3, val4);
	tst_resm(TINFO, "%s is %s %s %s %s", filename,
			val1, val2, val3, val4);
}

static void print_mm_stat(void)
{
	char filename[BUFSIZ];
	char  val1[BUFSIZ], val2[BUFSIZ], val3[BUFSIZ], val4[BUFSIZ];
	char  val5[BUFSIZ], val6[BUFSIZ], val7[BUFSIZ];

	sprintf(filename, "%s/mm_stat", PATH_ZRAM);
	SAFE_FILE_SCANF(cleanup, filename, "%s %s %s %s %s %s %s",
			val1, val2, val3, val4, val5, val6, val7);
	tst_resm(TINFO, "%s is %s %s %s %s %s %s %s", filename,
			val1, val2, val3, val4, val5, val6, val7);
}

static void print(char *string)
{
	char filename[BUFSIZ], value[BUFSIZ];

	sprintf(filename, "%s/%s", PATH_ZRAM, string);
	SAFE_FILE_SCANF(cleanup, filename, "%s", value);
	tst_resm(TINFO, "%s is %s", filename, value);
}

static void dump_info(void)
{
	print("initstate");
	print("disksize");
	print_mm_stat();
	print_io_stat();
}
