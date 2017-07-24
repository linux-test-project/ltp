/*
 * Copyright (c) Linux Test Project, 2017
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
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "lapi/syscalls.h"

#define TEST_FILE_1 "copy_file_range_ltp01.txt"
#define TEST_FILE_2 "copy_file_range_ltp02.txt"
#define STR "abcdefghijklmnopqrstuvwxyz12345\n"

#define verbose 0

static size_t *len_arr;
static loff_t **off_arr;
static int len_sz, off_sz;

static void setup(void)
{
	int i, fd, page_size;

	page_size = getpagesize();

	fd = SAFE_OPEN(TEST_FILE_1, O_RDWR | O_CREAT, 0664);
	/* Writing page_size * 4 of data into test file */
	for (i = 0; i < (int)(page_size * 4); i++)
		SAFE_WRITE(1, fd, STR, strlen(STR));
	SAFE_CLOSE(fd);

	len_sz = 4;
	len_arr = malloc(sizeof(size_t) * len_sz);
	len_arr[0] = 11;
	len_arr[1] = page_size - 1;
	len_arr[2] = page_size;
	len_arr[3] = page_size + 1;

	off_sz = 6;
	off_arr = malloc(sizeof(loff_t *) * off_sz);
	for (i = 1; i < off_sz; i++)
		off_arr[i] = malloc(sizeof(loff_t));

	off_arr[0] = NULL;
	*off_arr[1] = 0;
	*off_arr[2] = 17;
	*off_arr[3] = page_size - 1;
	*off_arr[4] = page_size;
	*off_arr[5] = page_size + 1;
}

static int check_file_content(const char *fname1, const char *fname2,
	loff_t *off1, loff_t *off2, size_t len)
{
	FILE *fp1, *fp2;
	int ch1, ch2;
	size_t count = 0;

	fp1 = SAFE_FOPEN(fname1, "r");
	if (off1 && fseek(fp1, *off1, SEEK_SET))
		tst_brk(TBROK | TERRNO, "fseek() failed");

	fp2 = SAFE_FOPEN(fname2, "r");
	if (off2 && fseek(fp2, *off2, SEEK_SET))
		tst_brk(TBROK | TERRNO, "fseek() failed");

	do {
		ch1 = getc(fp1);
		ch2 = getc(fp2);
		count++;
	} while ((count < len) && (ch1 == ch2));

	SAFE_FCLOSE(fp1);
	SAFE_FCLOSE(fp2);

	return !(ch1 == ch2);
}

static int check_file_offset(const char *m, int fd, loff_t len,
	loff_t *off_ori, loff_t *off_after)
{
	int ret = 0;

	if (off_ori) {
		/* FD should stay untouched, and off_in/out is updated */
		loff_t fd_off = SAFE_LSEEK(fd, 0, SEEK_CUR);

		if (fd_off == 0) {
			if (verbose)
				tst_res(TPASS, "%s FD offset unchanged", m);
		} else {
			tst_res(TFAIL, "%s FD offset changed: %ld",
				m, (long)fd_off);
			ret = 1;
		}

		if (!off_after) {
			tst_res(TFAIL, "%s offset is NULL", m);
			ret = 1;
		}

		if ((off_after) && (*off_ori + len == *off_after)) {
			if (verbose) {
				tst_res(TPASS, "%s offset advanced as"
					" expected: %ld", m, (long)*off_after);
			}
		} else {
			tst_res(TFAIL, "%s offset unexpected value: %ld",
				m, (long)*off_after);
			ret = 1;
		}
	} else {
		/* FD offset is advanced by len */
		loff_t fd_off = SAFE_LSEEK(fd, 0, SEEK_CUR);

		if (fd_off == len) {
			if (verbose) {
				tst_res(TPASS, "%s FD offset changed as"
					" expected: %ld", m, (long)fd_off);
			}
		} else {
			tst_res(TFAIL, "%s FD offset unexpected value: %ld",
				m, (long)fd_off);
			ret = 1;
		}
	}

	return ret;
}

static void test_one(size_t len, loff_t *off_in, loff_t *off_out)
{
	size_t to_copy = len;
	int fd_in, fd_out, ret;
	loff_t *off_in_ori = off_in;
	loff_t *off_out_ori = off_out;
	loff_t off_in_copy;
	loff_t off_out_copy;
	char str_off_in[32], str_off_out[32];

	if (off_in) {
		off_in_copy = *off_in;
		off_in = &off_in_copy;
		sprintf(str_off_in, "%ld", (long)*off_in);
	} else {
		strcpy(str_off_in, "NULL");
	}

	if (off_out) {
		off_out_copy = *off_out;
		off_out = &off_out_copy;
		sprintf(str_off_out, "%ld", (long)*off_out);
	} else {
		strcpy(str_off_out, "NULL");
	}

	fd_in = SAFE_OPEN(TEST_FILE_1, O_RDONLY);
	fd_out = SAFE_OPEN(TEST_FILE_2, O_CREAT | O_WRONLY | O_TRUNC, 0644);

	/*
	 * copy_file_range() will return the number of bytes copied between
	 * files. This could be less than the length originally requested.
	 */
	do {
		TEST(tst_syscall(__NR_copy_file_range, fd_in, off_in, fd_out,
			off_out, to_copy, 0));
		if (TEST_RETURN == -1) {
			tst_res(TFAIL | TTERRNO, "copy_file_range() failed");
			SAFE_CLOSE(fd_in);
			SAFE_CLOSE(fd_out);
			return;
		}

		to_copy -= TEST_RETURN;
	} while (to_copy > 0);

	ret = check_file_content(TEST_FILE_1, TEST_FILE_2,
		off_in_ori, off_out_ori, len);
	if (ret)
		tst_res(TFAIL, "file contents do not match");

	ret |= check_file_offset("(in)", fd_in, len, off_in_ori, off_in);
	ret |= check_file_offset("(out)", fd_out, len, off_out_ori, off_out);

	tst_res(ret == 0 ? TPASS : TFAIL, "off_in: %s, off_out: %s, len: %ld",
			str_off_in, str_off_out, (long)len);

	SAFE_CLOSE(fd_in);
	SAFE_CLOSE(fd_out);
}

static void copy_file_range_verify(void)
{
	int i, j, k;

	for (i = 0; i < len_sz; i++)
		for (j = 0; j < off_sz; j++)
			for (k = 0; k < off_sz; k++)
				test_one(len_arr[i], off_arr[j], off_arr[k]);
}

static struct tst_test test = {
	.setup = setup,
	.needs_tmpdir = 1,
	.test_all = copy_file_range_verify,
};
