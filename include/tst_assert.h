// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */
#ifndef TST_ASSERT_H__
#define TST_ASSERT_H__

/**
 * TST_ASSERT_INT() - Asserts that integer value in file equals val.
 *
 * @path: Path to the file to check.
 * @val: Expected integer value.
 *
 * Reads an integer from the file at path and compares it to val.
 * Reports :c:enum:`TPASS <tst_res_flags>` on match, or
 * :c:enum:`TFAIL <tst_res_flags>` on mismatch.
 */
#define TST_ASSERT_INT(path, val) \
	tst_assert_int(__FILE__, __LINE__, path, val)

void tst_assert_int(const char *file, const int lineno,
		    const char *path, int val);

/**
 * TST_ASSERT_FILE_INT() - Asserts that integer value for field in file equals val.
 *
 * @path: Path to the file to check.
 * @prefix: Field name or line prefix preceding the integer value.
 * @val: Expected integer value.
 *
 * Scans lines in path for prefix followed by an integer.
 * Reports :c:enum:`TPASS <tst_res_flags>` on match, or
 * :c:enum:`TFAIL <tst_res_flags>` on mismatch.
 */
#define TST_ASSERT_FILE_INT(path, prefix, val) \
	tst_assert_file_int(__FILE__, __LINE__, path, prefix, val)

void tst_assert_file_int(const char *file, const int lineno,
			 const char *path, const char *prefix, int val);

/**
 * TST_ASSERT_ULONG() - Asserts that unsigned long value in file equals val.
 *
 * @path: Path to the file to check.
 * @val: Expected unsigned long value.
 *
 * Reads an unsigned long from the file at path and compares it to val.
 * Reports :c:enum:`TPASS <tst_res_flags>` on match, or
 * :c:enum:`TFAIL <tst_res_flags>` on mismatch.
 */
#define TST_ASSERT_ULONG(path, val) \
	tst_assert_ulong(__FILE__, __LINE__, path, val)

void tst_assert_ulong(const char *file, const int lineno,
		      const char *path, unsigned long val);

/**
 * TST_ASSERT_STR() - Asserts that string value in file equals val.
 *
 * @path: Path to the file to check.
 * @val: Expected string value.
 *
 * Reads a whitespace-delimited string from path and compares it to val.
 * Reports :c:enum:`TPASS <tst_res_flags>` on match, or
 * :c:enum:`TFAIL <tst_res_flags>` on mismatch.
 */
#define TST_ASSERT_STR(path, val) \
	tst_assert_str(__FILE__, __LINE__, path, val)

void tst_assert_str(const char *file, const int lineno,
		    const char *path, const char *val);

/**
 * TST_ASSERT_FILE_STR() - Asserts that string value for field in file equals val.
 *
 * @path: Path to the file to check.
 * @prefix: Field name or prefix preceding the string value.
 * @val: Expected string value.
 *
 * Scans lines in path for prefix followed by ``:`` followed by whitespace and a string value.
 * Reports :c:enum:`TPASS <tst_res_flags>` on match, or
 * :c:enum:`TFAIL <tst_res_flags>` on mismatch.
 */
#define TST_ASSERT_FILE_STR(path, prefix, val) \
	tst_assert_file_str(__FILE__, __LINE__, path, prefix, val)

void tst_assert_file_str(const char *file, const int lineno,
			 const char *path, const char *prefix, const char *val);

#endif /* TST_ASSERT_H__ */
