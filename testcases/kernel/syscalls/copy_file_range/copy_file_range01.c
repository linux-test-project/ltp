// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2019
 */

/*
 * This tests the fundamental functionalities of the copy_file_range
 * syscall. It does so by copying the contents of one file into
 * another using various different combinations for length and
 * input/output offsets.
 *
 * After a copy is done this test checks if the contents of both files
 * are equal at the given offsets. It is also inspected if the offsets
 * of the file descriptors are advanced correctly.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "copy_file_range.h"

static int page_size;
static int errcount, numcopies;
static int fd_in, fd_out, cross_sup;

static struct tcase {
	char    *path;
	int     flags;
	char    *message;
} tcases[] = {
	{FILE_DEST_PATH,  0, "non cross-device"},
	{FILE_MNTED_PATH, 1, "cross-device"},
};

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
		loff_t *off_before, loff_t *off_after)
{
	loff_t fd_off = SAFE_LSEEK(fd, 0, SEEK_CUR);
	int ret = 0;

	if (off_before) {
		/*
		 * copy_file_range offset is given:
		 * - fd offset should stay 0,
		 * - copy_file_range offset is updated
		 */
		if (fd_off != 0) {
			tst_res(TFAIL,
				"%s fd offset unexpectedly changed: %ld",
				m, (long)fd_off);
			ret = 1;

		} else if (*off_before + len != *off_after) {
			tst_res(TFAIL, "%s offset unexpected value: %ld",
				m, (long)*off_after);
			ret = 1;
		}
	}
	/*
	 * no copy_file_range offset given:
	 * - fd offset advanced by length
	 */
	else if (fd_off != len) {
		tst_res(TFAIL, "%s fd offset unexpected value: %ld",
				m, (long)fd_off);
		ret = 1;
	}

	return ret;
}

static void test_one(size_t len, loff_t *off_in, loff_t *off_out, char *path)
{
	int ret;
	size_t to_copy = len;
	loff_t off_in_value_copy, off_out_value_copy;
	loff_t *off_new_in  = &off_in_value_copy;
	loff_t *off_new_out = &off_out_value_copy;
	char str_off_in[32], str_off_out[32];

	if (off_in) {
		off_in_value_copy = *off_in;
		sprintf(str_off_in, "%ld", (long)*off_in);
	} else {
		off_new_in = NULL;
		strcpy(str_off_in, "NULL");
	}

	if (off_out) {
		off_out_value_copy = *off_out;
		sprintf(str_off_out, "%ld", (long)*off_out);
	} else {
		off_new_out = NULL;
		strcpy(str_off_out, "NULL");
	}

	/*
	 * copy_file_range() will return the number of bytes copied between
	 * files. This could be less than the length originally requested.
	 */
	do {
		TEST(sys_copy_file_range(fd_in, off_new_in, fd_out,
				off_new_out, to_copy, 0));
		if (TST_RET == -1) {
			tst_res(TFAIL | TTERRNO, "copy_file_range() failed");
			errcount++;
			return;
		}

		to_copy -= TST_RET;
	} while (to_copy > 0);

	ret = check_file_content(FILE_SRC_PATH, path,
		off_in, off_out, len);
	if (ret) {
		tst_res(TFAIL, "file contents do not match");
		errcount++;
		return;
	}

	ret |= check_file_offset("(in)", fd_in, len, off_in, off_new_in);
	ret |= check_file_offset("(out)", fd_out, len, off_out, off_new_out);

	if (ret != 0) {
		tst_res(TFAIL, "off_in: %s, off_out: %s, len: %ld",
				str_off_in, str_off_out, (long)len);
		errcount++;
	}
}

static void open_files(char *path)
{
	fd_in  = SAFE_OPEN(FILE_SRC_PATH, O_RDONLY);
	fd_out = SAFE_OPEN(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
}

static void close_files(void)
{
	if (fd_out > 0)
		SAFE_CLOSE(fd_out);
	if (fd_in  > 0)
		SAFE_CLOSE(fd_in);
}

static void copy_file_range_verify(unsigned int n)
{
	int i, j, k;
	struct tcase *tc = &tcases[n];

	if (tc->flags && !cross_sup) {
		tst_res(TCONF,
			"copy_file_range() doesn't support cross-device, skip it");
		return;
	}

	errcount = numcopies = 0;
	size_t len_arr[]	= {11, page_size-1, page_size, page_size+1};
	loff_t off_arr_values[]	= {0, 17, page_size-1, page_size, page_size+1};

	int num_offsets = ARRAY_SIZE(off_arr_values) + 1;
	loff_t *off_arr[num_offsets];

	off_arr[0] = NULL;
	for (i = 1; i < num_offsets; i++)
		off_arr[i] = &off_arr_values[i-1];

	/* Test all possible cobinations of given lengths and offsets */
	for (i = 0; i < (int)ARRAY_SIZE(len_arr); i++)
		for (j = 0; j < num_offsets; j++)
			for (k = 0; k < num_offsets; k++) {
				open_files(tc->path);
				test_one(len_arr[i], off_arr[j], off_arr[k], tc->path);
				close_files();
				numcopies++;
			}

	if (errcount == 0)
		tst_res(TPASS,
			"%s copy_file_range completed all %d copy jobs successfully!",
			tc->message, numcopies);
	else
		tst_res(TFAIL, "%s copy_file_range failed %d of %d copy jobs.",
			tc->message, errcount, numcopies);
}

static void setup(void)
{
	syscall_info();
	page_size = getpagesize();
	cross_sup = verify_cross_fs_copy_support(FILE_SRC_PATH, FILE_MNTED_PATH);
}

static void cleanup(void)
{
	close_files();
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.test = copy_file_range_verify,
	.test_variants = TEST_VARIANTS,
};
