/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_SYSFS_ASSERT_H__
#define TST_SYSFS_ASSERT_H__

#include <stddef.h>
#include <dirent.h>

/**
 * TST_SYSFS_ASSERT_RANGELL() - Asserts that min <= val <= max.
 *
 * The val is read from a path constructed from the fmt. Both bounds are
 * long long values, hence the LL (long long) suffix. This matches or
 * exceeds the width of any integer type the kernel exports through sysfs
 * (long, unsigned long, s64/u64), so it is safe to use even on 32bit
 * userspace with a 64bit kernel value, e.g. a millisecond ktime_t counter.
 *
 * @min: A long long minimal value.
 * @max: A long long maximal value.
 * @fmt: A printf-like format to build path to the file to read the val from.
 * @...: A printf-like parameters for the path.
 */
#define TST_SYSFS_ASSERT_RANGELL(min, max, fmt, ...) \
	tst_sysfs_assert_rangell(__FILE__, __LINE__, min, max, fmt, ##__VA_ARGS__)

void tst_sysfs_assert_rangell(const char *file, const int lineno,
	long long min, long long max, const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_RANGELL_SILENT() - Asserts that min <= val <= max.
 *
 * The val is read from a path constructed from the fmt. Both bounds are
 * long long values, hence the LL (long long) suffix.
 *
 * Silently skips the check if the file path constructed from fmt does not
 * exist.
 *
 * @min: A minimal value.
 * @max: A maximal value.
 * @fmt: A printf-like format to build path to the file to read the val from.
 * @...: A printf-like parameters for the path.
 */
#define TST_SYSFS_ASSERT_RANGELL_SILENT(min, max, fmt, ...) \
	tst_sysfs_assert_rangell_silent(__FILE__, __LINE__, min, max, fmt, ##__VA_ARGS__)

void tst_sysfs_assert_rangell_silent(const char *file, const int lineno,
	long long min, long long max, const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_RANGEUU() - Asserts that min <= val <= max.
 *
 * Like TST_SYSFS_ASSERT_RANGELL() but both the bounds and the value read
 * from sysfs are unsigned long long, hence the UU (unsigned, unsigned)
 * suffix. Use this instead of the LL variant for attributes the kernel
 * exports as an unsigned type, e.g. "unsigned long" counters: passing
 * ULONG_MAX as a bound to the LL variant would silently reinterpret it as
 * -1 on a 64bit "unsigned long" (it does not fit into a signed long long),
 * while unsigned long long is always at least as wide as unsigned long, so
 * ULONG_MAX can be used directly here without any clamping.
 *
 * @min: An unsigned long long minimal value.
 * @max: An unsigned long long maximal value.
 * @fmt: A printf-like format to build path to the file to read the val from.
 * @...: A printf-like parameters for the path.
 */
#define TST_SYSFS_ASSERT_RANGEUU(min, max, fmt, ...) \
	tst_sysfs_assert_rangeuu(__FILE__, __LINE__, min, max, fmt, ##__VA_ARGS__)

void tst_sysfs_assert_rangeuu(const char *file, const int lineno,
	unsigned long long min, unsigned long long max, const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_RANGEUU_SILENT() - Asserts that min <= val <= max.
 *
 * Unsigned counterpart of TST_SYSFS_ASSERT_RANGELL_SILENT(), see
 * TST_SYSFS_ASSERT_RANGEUU() for why an unsigned variant is needed.
 *
 * Silently skips the check if the file path constructed from fmt does not
 * exist.
 *
 * @min: A minimal value.
 * @max: A maximal value.
 * @fmt: A printf-like format to build path to the file to read the val from.
 * @...: A printf-like parameters for the path.
 */
#define TST_SYSFS_ASSERT_RANGEUU_SILENT(min, max, fmt, ...) \
	tst_sysfs_assert_rangeuu_silent(__FILE__, __LINE__, min, max, fmt, ##__VA_ARGS__)

void tst_sysfs_assert_rangeuu_silent(const char *file, const int lineno,
	unsigned long long min, unsigned long long max, const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_RANGELF() - Asserts that min <= val <= max.
 *
 * The min bound is a long long, while the max bound is read from a file,
 * hence the LF (long long, file) suffix. The value and max paths are built
 * by appending val_suffix/max_suffix to a common prefix built from fmt.
 *
 * Skips with :c:enum:`TCONF <tst_res_flags>` if either file does not exist.
 *
 * @min: A minimal value.
 * @val_suffix: A filename the val is read from.
 * @max_suffix: A filename the max is read from.
 * @fmt: A printf-like format to build the path to val and max from.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_RANGELF(min, val_suffix, max_suffix, fmt, ...) \
	tst_sysfs_assert_rangelf(__FILE__, __LINE__, min, val_suffix, \
		max_suffix, fmt, ##__VA_ARGS__)

void tst_sysfs_assert_rangelf(const char *file, const int lineno,
	long long min, const char *val_suffix, const char *max_suffix,
	const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_BOOL() - Asserts that file has boolean value.
 *
 * Reads a value from the file at the path built from fmt and asserts
 * that it is a boolean (0 or 1).
 *
 * Skips with :c:enum:`TCONF <tst_res_flags>` if the file does not exist.
 *
 * @fmt: A printf-like format to build a path to the boolean file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_BOOL(fmt, ...) \
	tst_sysfs_assert_bool(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

void tst_sysfs_assert_bool(const char *file, const int lineno,
	const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_BOOL_SILENT() - Asserts that file has boolean value.
 *
 * Reads a value from the file at the path built from fmt and asserts
 * that it is a boolean (0 or 1).
 *
 * Silently skips the check if the file path constructed from fmt does not
 * exist.
 *
 * @fmt: A printf-like format to build a path to the boolean file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_BOOL_SILENT(fmt, ...) \
	tst_sysfs_assert_bool_silent(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

void tst_sysfs_assert_bool_silent(const char *file, const int lineno,
	const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_POW2() - Asserts that file has number that is power of two.
 *
 * Reads a long value from the file at the path built from fmt and asserts
 * that it is a power of two, i.e. value > 0 and (value & (value - 1)) == 0.
 *
 * Skips with :c:enum:`TCONF <tst_res_flags>` if the file does not exist.
 *
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like params for fmt.
 */
#define TST_SYSFS_ASSERT_POW2(fmt, ...) \
	tst_sysfs_assert_pow2(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

void tst_sysfs_assert_pow2(const char *file, const int lineno,
	const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_CHOICE() - Asserts that file is a list of choices.
 *
 * Validates a bracketed-choice file such as:
 *
 *   "none mq-deadline kyber [bfq]"
 *   "[always] madvise never"
 *
 * Asserts that exactly one token is [selected]. If allowed is non-NULL (a
 * NULL-terminated array of strings) it also asserts that every token is a
 * member of the allowed set. If sel is non-NULL the selected token (with the
 * brackets stripped) is copied into sel, truncated to sel_size.
 *
 * Skips with :c:enum:`TCONF <tst_res_flags>` if the file does not exist.
 *
 * @allowed: Optional NULL terminated array of allowed choices.
 * @sel: A buffer to copy the selected choice into.
 * @sel_size: A size of the sel buffer.
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_CHOICE(allowed, sel, sel_size, fmt, ...) \
	tst_sysfs_assert_choice(__FILE__, __LINE__, allowed, sel, sel_size, \
		fmt, ##__VA_ARGS__)

void tst_sysfs_assert_choice(const char *file, const int lineno,
	const char *const allowed[], char *sel, size_t sel_size,
	const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_TOKENS() - Asserts that file is a list of tokens.
 *
 * Validates a file that lists a whitespace-separated set of tokens without any
 * bracketed "current" selection, such as /sys/power/state:
 *
 *   "freeze mem disk"
 *
 * Asserts that the file is non-empty and, if allowed is non-NULL, that every
 * token is a member of the allowed set (a NULL-terminated array of strings).
 *
 * If find is non-NULL it additionally asserts that find itself was seen
 * among the tokens, which is useful for cross-checking a "current" value
 * read from a different file against this file's list, e.g.:
 *
 *   TST_SYSFS_ASSERT_TOKENS(NULL, current_clocksource, "%s/available_clocksource", name);
 *
 * Skips with :c:enum:`TCONF <tst_res_flags>` if the file does not exist.
 *
 * @allowed: Optional NULL terminated array of allowed tokens.
 * @find: Optional token that must be present in the file, or NULL to skip
 *        this check.
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_TOKENS(allowed, find, fmt, ...) \
	tst_sysfs_assert_tokens(__FILE__, __LINE__, allowed, find, fmt, ##__VA_ARGS__)

void tst_sysfs_assert_tokens(const char *file, const int lineno,
	const char *const allowed[], const char *find, const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_ONEOF() - Asserts that file has one of allowed strings.
 *
 * Reads a single whitespace-delimited string token from the file at the path
 * built from fmt and asserts that it is a member of the allowed set (a
 * NULL-terminated array of strings), e.g. the cache "type" being one of
 * Data/Instruction/Unified.
 *
 * Skips with :c:enum:`TCONF <tst_res_flags>` if the file does not exist.
 *
 * @allowed: A NULL terminated array of allowed strings.
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_ONEOF(allowed, fmt, ...) \
	tst_sysfs_assert_oneof(__FILE__, __LINE__, allowed, fmt, ##__VA_ARGS__)

void tst_sysfs_assert_oneof(const char *file, const int lineno,
	const char *const allowed[], const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_PARSE_LIST() - Parses a cpulist/nodelist-style file.
 *
 * Parses a file such as "0-1,3", which is used by many sysfs attributes (cpu
 * online/present/possible, node online/possible, ...).
 *
 * On success it stores the number of set entries into count (if non-NULL)
 * and the highest id present into max_id (if non-NULL) and returns 0. An
 * empty list yields count == 0 and max_id == -1.
 *
 * On failure it reports the reason itself and returns non-zero:
 * :c:enum:`TCONF <tst_res_flags>` if the file does not exist, TFAIL if the
 * content cannot be parsed. The caller only needs to check the return value to
 * decide whether to skip any further checks that depend on the count/max_id
 * values.
 *
 * @count: A pointer to store the number of set entries to, or NULL.
 * @max_id: A pointer to store the highest id present to, or NULL.
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_PARSE_LIST(count, max_id, fmt, ...) \
	tst_sysfs_assert_parse_list(__FILE__, __LINE__, count, max_id, \
		fmt, ##__VA_ARGS__)

int tst_sysfs_assert_parse_list(const char *file, const int lineno,
	int *count, int *max_id, const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_LIST_SUBSET() - Asserts a cpulist/nodelist is a subset.
 *
 * Asserts that the cpulist/nodelist in sub_path is a subset of the list in
 * super_path, i.e. every id set in sub_path is also set in super_path.
 *
 * Skips with :c:enum:`TCONF <tst_res_flags>` if either file does not exist.
 *
 * @sub_path: A path to the file with the presumed subset list.
 * @super_path: A path to the file with the presumed superset list.
 */
#define TST_SYSFS_ASSERT_LIST_SUBSET(sub_path, super_path) \
	tst_sysfs_assert_list_subset(__FILE__, __LINE__, sub_path, super_path)

void tst_sysfs_assert_list_subset(const char *file, const int lineno,
	const char *sub_path, const char *super_path);

/**
 * TST_SYSFS_ASSERT_LIST_CONTAINS() - Asserts that id is in a cpulist/nodelist.
 *
 * Asserts that id is a member of the cpulist/nodelist in the file at the
 * path built from fmt, e.g. that a given CPU is listed in its own
 * thread_siblings_list.
 *
 * Skips with :c:enum:`TCONF <tst_res_flags>` if the file does not exist.
 *
 * @id: The id that must be present in the list.
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_LIST_CONTAINS(id, fmt, ...) \
	tst_sysfs_assert_list_contains(__FILE__, __LINE__, id, fmt, ##__VA_ARGS__)

void tst_sysfs_assert_list_contains(const char *file, const int lineno,
	int id, const char *fmt, ...);

enum tst_sysfs_cmp {
	TST_SYSFS_CMP_EQ,
	TST_SYSFS_CMP_LT,
	TST_SYSFS_CMP_LE,
};

void tst_sysfs_assert_cmp(const char *file, const int lineno,
	const char *path1, enum tst_sysfs_cmp op, const char *path2);

/**
 * TST_SYSFS_ASSERT_EQ() - Asserts that val1 == val2.
 *
 * Reads a long value from each of path1 and path2 and asserts that they are
 * equal.
 *
 * Skips with :c:enum:`TCONF <tst_res_flags>` if either file does not exist.
 *
 * @path1: A path to the file with val1.
 * @path2: A path to the file with val2.
 */
#define TST_SYSFS_ASSERT_EQ(path1, path2) \
	tst_sysfs_assert_cmp(__FILE__, __LINE__, path1, TST_SYSFS_CMP_EQ, path2)

/**
 * TST_SYSFS_ASSERT_LT() - Asserts that val1 < val2.
 *
 * Reads a long value from each of path1 and path2 and asserts that val1 is
 * less than val2.
 *
 * Skips with :c:enum:`TCONF <tst_res_flags>` if either file does not exist.
 *
 * @path1: A path to the file with val1.
 * @path2: A path to the file with val2.
 */
#define TST_SYSFS_ASSERT_LT(path1, path2) \
	tst_sysfs_assert_cmp(__FILE__, __LINE__, path1, TST_SYSFS_CMP_LT, path2)

/**
 * TST_SYSFS_ASSERT_LE() - Asserts that val1 <= val2.
 *
 * Reads a long value from each of path1 and path2 and asserts that val1 is
 * less than or equal to val2.
 *
 * Skips with :c:enum:`TCONF <tst_res_flags>` if either file does not exist.
 *
 * @path1: A path to the file with val1.
 * @path2: A path to the file with val2.
 */
#define TST_SYSFS_ASSERT_LE(path1, path2) \
	tst_sysfs_assert_cmp(__FILE__, __LINE__, path1, TST_SYSFS_CMP_LE, path2)

void tst_sysfs_assert_cmp_silent(const char *file, const int lineno,
	const char *path1, enum tst_sysfs_cmp op, const char *path2);

/**
 * TST_SYSFS_ASSERT_EQ_SILENT() - Asserts that val1 == val2.
 *
 * Same as TST_SYSFS_ASSERT_EQ(), but silently skips the check if either file
 * does not exist.
 *
 * @path1: A path to the file with val1.
 * @path2: A path to the file with val2.
 */
#define TST_SYSFS_ASSERT_EQ_SILENT(path1, path2) \
	tst_sysfs_assert_cmp_silent(__FILE__, __LINE__, path1, TST_SYSFS_CMP_EQ, path2)

/**
 * TST_SYSFS_ASSERT_LT_SILENT() - Asserts that val1 < val2.
 *
 * Same as TST_SYSFS_ASSERT_LT(), but silently skips the check if either file
 * does not exist.
 *
 * @path1: A path to the file with val1.
 * @path2: A path to the file with val2.
 */
#define TST_SYSFS_ASSERT_LT_SILENT(path1, path2) \
	tst_sysfs_assert_cmp_silent(__FILE__, __LINE__, path1, TST_SYSFS_CMP_LT, path2)

/**
 * TST_SYSFS_ASSERT_LE_SILENT() - Asserts that val1 <= val2.
 *
 * Same as TST_SYSFS_ASSERT_LE(), but silently skips the check if either file
 * does not exist.
 *
 * @path1: A path to the file with val1.
 * @path2: A path to the file with val2.
 */
#define TST_SYSFS_ASSERT_LE_SILENT(path1, path2) \
	tst_sysfs_assert_cmp_silent(__FILE__, __LINE__, path1, TST_SYSFS_CMP_LE, path2)

void tst_sysfs_assert_cmp_suffix(const char *file, const int lineno,
	enum tst_sysfs_cmp op, const char *lo_suffix, const char *hi_suffix,
	const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_EQ_SUFFIX() - Asserts that val1 == val2.
 *
 * Same as TST_SYSFS_ASSERT_EQ(), but path1/path2 are built by appending
 * lo_suffix/hi_suffix to a common prefix built from fmt.
 *
 * @lo_suffix: A suffix appended to fmt to build the path to val1.
 * @hi_suffix: A suffix appended to fmt to build the path to val2.
 * @fmt: A printf-like format to build the common path prefix.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_EQ_SUFFIX(lo_suffix, hi_suffix, fmt, ...) \
	tst_sysfs_assert_cmp_suffix(__FILE__, __LINE__, TST_SYSFS_CMP_EQ, \
		lo_suffix, hi_suffix, fmt, ##__VA_ARGS__)

/**
 * TST_SYSFS_ASSERT_LT_SUFFIX() - Asserts that val1 < val2.
 *
 * Same as TST_SYSFS_ASSERT_LT(), but path1/path2 are built by appending
 * lo_suffix/hi_suffix to a common prefix built from fmt.
 *
 * @lo_suffix: A suffix appended to fmt to build the path to val1.
 * @hi_suffix: A suffix appended to fmt to build the path to val2.
 * @fmt: A printf-like format to build the common path prefix.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_LT_SUFFIX(lo_suffix, hi_suffix, fmt, ...) \
	tst_sysfs_assert_cmp_suffix(__FILE__, __LINE__, TST_SYSFS_CMP_LT, \
		lo_suffix, hi_suffix, fmt, ##__VA_ARGS__)

/**
 * TST_SYSFS_ASSERT_LE_SUFFIX() - Asserts that val1 <= val2.
 *
 * Same as TST_SYSFS_ASSERT_LE(), but path1/path2 are built by appending
 * lo_suffix/hi_suffix to a common prefix built from fmt.
 *
 * @lo_suffix: A suffix appended to fmt to build the path to val1.
 * @hi_suffix: A suffix appended to fmt to build the path to val2.
 * @fmt: A printf-like format to build the common path prefix.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_LE_SUFFIX(lo_suffix, hi_suffix, fmt, ...) \
	tst_sysfs_assert_cmp_suffix(__FILE__, __LINE__, TST_SYSFS_CMP_LE, \
		lo_suffix, hi_suffix, fmt, ##__VA_ARGS__)

void tst_sysfs_assert_cmp_suffix_silent(const char *file, const int lineno,
	enum tst_sysfs_cmp op, const char *lo_suffix, const char *hi_suffix,
	const char *fmt, ...);

/**
 * TST_SYSFS_ASSERT_EQ_SUFFIX_SILENT() - Asserts that val1 == val2.
 *
 * Same as TST_SYSFS_ASSERT_EQ_SUFFIX(), but silently skips the check if
 * either file does not exist.
 *
 * @lo_suffix: A suffix appended to fmt to build the path to val1.
 * @hi_suffix: A suffix appended to fmt to build the path to val2.
 * @fmt: A printf-like format to build the common path prefix.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_EQ_SUFFIX_SILENT(lo_suffix, hi_suffix, fmt, ...) \
	tst_sysfs_assert_cmp_suffix_silent(__FILE__, __LINE__, \
		TST_SYSFS_CMP_EQ, lo_suffix, hi_suffix, fmt, ##__VA_ARGS__)

/**
 * TST_SYSFS_ASSERT_LT_SUFFIX_SILENT() - Asserts that val1 < val2.
 *
 * Same as TST_SYSFS_ASSERT_LT_SUFFIX(), but silently skips the check if
 * either file does not exist.
 *
 * @lo_suffix: A suffix appended to fmt to build the path to val1.
 * @hi_suffix: A suffix appended to fmt to build the path to val2.
 * @fmt: A printf-like format to build the common path prefix.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_LT_SUFFIX_SILENT(lo_suffix, hi_suffix, fmt, ...) \
	tst_sysfs_assert_cmp_suffix_silent(__FILE__, __LINE__, \
		TST_SYSFS_CMP_LT, lo_suffix, hi_suffix, fmt, ##__VA_ARGS__)

/**
 * TST_SYSFS_ASSERT_LE_SUFFIX_SILENT() - Asserts that val1 <= val2.
 *
 * Same as TST_SYSFS_ASSERT_LE_SUFFIX(), but silently skips the check if
 * either file does not exist.
 *
 * @lo_suffix: A suffix appended to fmt to build the path to val1.
 * @hi_suffix: A suffix appended to fmt to build the path to val2.
 * @fmt: A printf-like format to build the common path prefix.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_ASSERT_LE_SUFFIX_SILENT(lo_suffix, hi_suffix, fmt, ...) \
	tst_sysfs_assert_cmp_suffix_silent(__FILE__, __LINE__, \
		TST_SYSFS_CMP_LE, lo_suffix, hi_suffix, fmt, ##__VA_ARGS__)

/**
 * tst_sysfs_exists() - Checks whether a file exists.
 *
 *
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like parameters for fmt.
 * return: 1 if the file at the path built from fmt exists, 0 otherwise.
 */
int tst_sysfs_exists(const char *fmt, ...);

/**
 * TST_SYSFS_TRY_OPENDIR() - Opens a directory, or brk()s with TCONF if it
 * does not exist.
 *
 * Behaves like SAFE_OPENDIR() otherwise, i.e. this never returns on failure:
 * :c:enum:`TCONF <tst_res_flags>` if the directory built from fmt does not
 * exist, TBROK for any other failure to open it.
 *
 * @fmt: A printf-like format to build the path to the directory.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_TRY_OPENDIR(fmt, ...) \
	tst_sysfs_try_opendir(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

DIR *tst_sysfs_try_opendir(const char *file, const int lineno,
	const char *fmt, ...);

/**
 * TST_SYSFS_READ_STR() - Reads a string from a file.
 *
 * Reads a string from the file at the path built from fmt into buf, tolerating
 * empty/whitespace-only files like SAFE_FILE_READ_STR() does, and stripping a
 * trailing newline.
 *
 * Returns 1 and fills buf if the file exists, or 0 and leaves buf untouched.
 *
 * @buf: A buffer to read the string into.
 * @buf_size: A size of the buf buffer.
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_READ_STR(buf, buf_size, fmt, ...) \
	tst_sysfs_read_str(__FILE__, __LINE__, buf, buf_size, fmt, ##__VA_ARGS__)

int tst_sysfs_read_str(const char *file, const int lineno, char *buf,
	size_t buf_size, const char *fmt, ...);

/**
 * TST_SYSFS_EXP_EQ_LI() - Asserts that a file contains a given number.
 *
 * Reads a long value from the file at the path built from fmt and asserts that
 * it equals val. Reports TPASS/TFAIL with the resolved path and both values,
 * or TFAIL if the file does not exist or does not contain a number.
 *
 * This combines a numeric read and a TST_EXP_EQ_LI() style comparison in a
 * single call, so tests do not have to build the path and read the value
 * into a temporary variable themselves.
 *
 * @val: The expected long int value.
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_EXP_EQ_LI(val, fmt, ...) \
	tst_sysfs_exp_eq_li(__FILE__, __LINE__, val, fmt, ##__VA_ARGS__)

void tst_sysfs_exp_eq_li(const char *file, const int lineno, long val,
	const char *fmt, ...);

/**
 * TST_SYSFS_EXP_EQ_STR() - Asserts that a file contains a given string.
 *
 * Reads a single whitespace-delimited string token from the file at the path
 * built from fmt and asserts that it equals val. Reports TPASS/TFAIL with the
 * resolved path and both values, or :c:enum:`TCONF <tst_res_flags>` if the
 * file does not exist.
 *
 * This combines a string read and a TST_EXP_EQ_STR() style comparison in a
 * single call, so tests do not have to build the path and read the value
 * into a temporary buffer themselves.
 *
 * @val: The expected string value.
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_EXP_EQ_STR(val, fmt, ...) \
	tst_sysfs_exp_eq_str(__FILE__, __LINE__, val, fmt, ##__VA_ARGS__)

void tst_sysfs_exp_eq_str(const char *file, const int lineno, const char *val,
	const char *fmt, ...);

/**
 * TST_SYSFS_READ_LLI() - Reads a long long from a file.
 *
 * Reads a long long value from the file at the path built from fmt and
 * returns it.
 *
 * Aborts with TBROK if the file does not exist or does not contain a valid
 * number: unlike most of the other checks in this header, this is not
 * expected to be an optional attribute the caller should gracefully skip,
 * see SAFE_FILE_SCANF().
 *
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_READ_LLI(fmt, ...) \
	tst_sysfs_read_lli(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

long long tst_sysfs_read_lli(const char *file, const int lineno,
	const char *fmt, ...);

/**
 * TST_SYSFS_READ_LI() - Reads a long from a file.
 *
 * Reads a long value from the file at the path built from fmt and returns it.
 *
 * Aborts with TBROK if the file does not exist or does not contain a valid
 * number: unlike most of the other checks in this header, this is not
 * expected to be an optional attribute the caller should gracefully skip,
 * see SAFE_FILE_SCANF().
 *
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_READ_LI(fmt, ...) \
	tst_sysfs_read_li(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

long tst_sysfs_read_li(const char *file, const int lineno,
	const char *fmt, ...);

/**
 * TST_SYSFS_READ_LX() - Reads a hexadecimal unsigned long from a file.
 *
 * Reads an unsigned long value from the file at the path built from fmt,
 * parsed as hexadecimal (via "%lx", with or without a "0x" prefix), and
 * returns it.
 *
 * Aborts with TBROK if the file does not exist or does not contain a valid
 * number: unlike most of the other checks in this header, this is not
 * expected to be an optional attribute the caller should gracefully skip,
 * see SAFE_FILE_SCANF().
 *
 * @fmt: A printf-like format to build a path to the file.
 * @...: A printf-like parameters for fmt.
 */
#define TST_SYSFS_READ_LX(fmt, ...) \
	tst_sysfs_read_lx(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

unsigned long tst_sysfs_read_lx(const char *file, const int lineno,
	const char *fmt, ...);

#endif /* TST_SYSFS_ASSERT_H__ */
