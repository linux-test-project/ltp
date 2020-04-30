// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */
#ifndef TST_ASSERT_H__
#define TST_ASSERT_H__

#define TST_ASSERT_INT(path, val) \
	tst_assert_int(__FILE__, __LINE__, path, val)

/*
 * Asserts that integer value stored in file pointed by path equals to the
 * value passed to this function. This is mostly useful for asserting correct
 * values in sysfs, procfs, etc.
 */
void tst_assert_int(const char *file, const int lineno,
		    const char *path, int val);

#define TST_ASSERT_FILE_INT(path, prefix, val) \
	tst_assert_file_int(__FILE__, __LINE__, path, prefix, val)

/*
 * Asserts that integer value stored in the prefix field of file pointed by path
 * equals to the value passed to this function. This is mostly useful for
 * asserting correct field values in sysfs, procfs, etc.
 */

void tst_assert_file_int(const char *file, const int lineno,
			 const char *path, const char *prefix, int val);


#define TST_ASSERT_STR(path, val) \
	tst_assert_str(__FILE__, __LINE__, path, val)

/*
 * Asserts that a string value stored in file pointed by path equals to the
 * value passed to this function. This is mostly useful for asserting correct
 * values in sysfs, procfs, etc.
 */
void tst_assert_str(const char *file, const int lineno,
		    const char *path, const char *val);

#define TST_ASSERT_FILE_STR(path, prefix, val) \
	tst_assert_file_str(__FILE__, __LINE__, path, prefix, val)

/*
 * Asserts that a string value stored in the prefix field of file pointed by path
 * equals to the value passed to this function. This is mostly useful for
 * asserting correct field values in sysfs, procfs, etc.
 */
void tst_assert_file_str(const char *file, const int lineno,
			 const char *path, const char *prefix, const char *val);

#endif /* TST_ASSERT_H__ */
