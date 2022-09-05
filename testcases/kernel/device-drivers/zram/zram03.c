// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2010  Red Hat, Inc.
 */

/*\
 * [Description]
 *
 * zram: generic RAM based compressed R/W block devices
 * http://lkml.org/lkml/2010/8/9/227
 *
 * This case check whether data read from zram device is consistent with
 * thoese are written.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "tst_safe_stdio.h"
#include "tst_test.h"

#define ZRAM_CONTROL_PATH	"/sys/class/zram-control"
#define HOT_ADD_PATH		ZRAM_CONTROL_PATH"/hot_add"
#define HOT_REMOVE_PATH		ZRAM_CONTROL_PATH"/hot_remove"
#define SIZE			(512 * 1024 * 1024L)

static char zram_block_path[100], zram_dev_path[100];
static int modprobe, dev_num, hot_add_flag;
static const char *const cmd_rmmod[] = {"rmmod", "zram", NULL};

static void set_disksize(void)
{
	char disksize_path[200];

	tst_res(TINFO, "create a zram device with %ld bytes in size", SIZE);
	sprintf(disksize_path, "%s/disksize", zram_block_path);
	SAFE_FILE_PRINTF(disksize_path, "%ld", SIZE);
}

static void write_device(void)
{
	int fd;
	char *s;

	tst_res(TINFO, "map this zram device into memory");
	fd = SAFE_OPEN(zram_dev_path, O_RDWR);
	s = SAFE_MMAP(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	tst_res(TINFO, "write all the memory");
	memset(s, 'a', SIZE - 1);
	s[SIZE - 1] = '\0';

	SAFE_MUNMAP(s, SIZE);
	SAFE_CLOSE(fd);
}

static void verify_device(void)
{
	int fd;
	long i = 0, fail = 0;
	char *s;

	tst_res(TINFO, "verify contents from device");
	fd = SAFE_OPEN(zram_dev_path, O_RDONLY);
	s = SAFE_MMAP(NULL, SIZE, PROT_READ, MAP_PRIVATE, fd, 0);

	while (s[i] && i < SIZE - 1) {
		if (s[i] != 'a')
			fail++;
		i++;
	}
	if (i != SIZE - 1) {
		tst_res(TFAIL, "expect size: %ld, actual size: %ld.",
			 SIZE - 1, i);
	} else if (s[i] != '\0') {
		tst_res(TFAIL, "zram device seems not null terminated");
	} else if (fail) {
		tst_res(TFAIL, "%ld failed bytes found", fail);
	} else {
		tst_res(TPASS, "data read from zram device is consistent with those are written");
	}

	SAFE_MUNMAP(s, SIZE);
	SAFE_CLOSE(fd);
}

static void reset(void)
{
	char reset_path[200];

	tst_res(TINFO, "Reset zram");
	sprintf(reset_path, "%s/reset", zram_block_path);
	SAFE_FILE_PRINTF(reset_path, "1");
}

static void print(char *string)
{
	char filename[BUFSIZ], value[BUFSIZ];

	tst_res(TINFO, "%s",  zram_block_path);
	sprintf(filename, "%s/%s", zram_block_path, string);
	SAFE_FILE_SCANF(filename, "%s", value);
	tst_res(TINFO, "%s is %s", filename, value);
}

static void print_stat(char *nread, char *nwrite)
{
	char nread_val[BUFSIZ], nwrite_val[BUFSIZ];
	char zram_stat_path[100];

	sprintf(zram_stat_path, "/sys/block/zram%d/stat", dev_num);
	SAFE_FILE_SCANF(zram_stat_path, "%s %*s %*s %*s %s", nread_val, nwrite_val);
	tst_res(TINFO, "%s from %s is %s", nread, zram_stat_path, nread_val);
	tst_res(TINFO, "%s from %s is %s", nwrite, zram_stat_path, nwrite_val);
}

static void print_mm_stat(char *orig, char *compr, char *mem, char *zero)
{
	char orig_val[BUFSIZ], compr_val[BUFSIZ];
	char mem_val[BUFSIZ], zero_val[BUFSIZ];
	char zram_mm_stat_path[100];

	sprintf(zram_mm_stat_path, "/sys/block/zram%d/mm_stat", dev_num);
	SAFE_FILE_SCANF(zram_mm_stat_path, "%s %s %s %*s %*s %s",
			orig_val, compr_val, mem_val, zero_val);
	tst_res(TINFO, "%s from %s is %s", orig, zram_mm_stat_path, orig_val);
	tst_res(TINFO, "%s from %s is %s", compr, zram_mm_stat_path, compr_val);
	tst_res(TINFO, "%s from %s is %s", mem, zram_mm_stat_path, mem_val);
	tst_res(TINFO, "%s from %s is %s", zero, zram_mm_stat_path, zero_val);
}

static void dump_info(void)
{
	char zram_obsolete_file_path[100];

	sprintf(zram_obsolete_file_path, "/sys/block/zram%d/num_reads", dev_num);
	print("initstate");
	print("disksize");
	if (!access(zram_obsolete_file_path, F_OK)) {
		print("orig_data_size");
		print("compr_data_size");
		print("mem_used_total");
		print("zero_pages");
		print("num_reads");
		print("num_writes");
	} else {
		print_mm_stat("orig_data_size", "compr_data_size",
			      "mem_used_total", "zero/same_pages");
		print_stat("num_reads", "num_writes");
	}
}

static void run(void)
{
	set_disksize();

	write_device();
	dump_info();
	verify_device();

	reset();
	dump_info();
}

static void setup(void)
{
	const char *const cmd_modprobe[] = {"modprobe", "zram", NULL};
	const char *const cmd_zramctl[] = {"zramctl", "-f", NULL};
	const char *zramctl_log_path = "zramctl.log";
	FILE *file;
	char line[PATH_MAX];
	int fd;

	/* zram module was built in or loaded on new kernel */
	if (!access(ZRAM_CONTROL_PATH, F_OK)) {
		tst_res(TINFO,
			"zram module already loaded, kernel supports zram-control interface");
		SAFE_FILE_SCANF(HOT_ADD_PATH, "%d", &dev_num);
		hot_add_flag =1;
		goto fill_path;
	}

	 /* zram module was built in or being used on old kernel */
	SAFE_CMD(cmd_modprobe, NULL, NULL);
	file = SAFE_FOPEN("/proc/modules", "r");
	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, "zram")) {
			modprobe = 1;
			break;
		}
	}
	SAFE_FCLOSE(file);
	if (access(ZRAM_CONTROL_PATH, F_OK)) {
		if (modprobe) {
			tst_res(TINFO,
				"rmmod zram before test on old kernel without zram-control interface");
			if (!tst_cmd(cmd_rmmod, NULL, NULL, TST_CMD_PASS_RETVAL)) {
				SAFE_CMD(cmd_modprobe, NULL, NULL);
				goto fill_path;
			}
		} else {
			tst_res(TINFO,
				"zram module is built in old kernel without zram-control interface");
		}

		modprobe = 0;
		tst_res(TINFO, "use zramctl -f to find free zram device");
		fd = SAFE_OPEN(zramctl_log_path, O_CREAT | O_RDWR, 0644);
		SAFE_CLOSE(fd);
		if (tst_cmd(cmd_zramctl, zramctl_log_path, NULL, TST_CMD_PASS_RETVAL))
			tst_brk(TCONF | TERRNO, "zramctl -f failed");
		else
			SAFE_FILE_SCANF(zramctl_log_path, "/dev/zram%d", &dev_num);
	}

fill_path:
	sprintf(zram_block_path, "/sys/block/zram%d", dev_num);
	sprintf(zram_dev_path, "/dev/zram%d", dev_num);
}

static void cleanup(void)
{
	if (hot_add_flag)
		SAFE_FILE_PRINTF(HOT_REMOVE_PATH, "%d", dev_num);

	if (modprobe)
		SAFE_CMD(cmd_rmmod, NULL, NULL);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.needs_drivers = (const char *const []) {
		"zram",
		NULL
	},
	.needs_cmds = (const char *[]) {
		"modprobe",
		"rmmod",
		NULL
	}
};
