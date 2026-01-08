/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2015-2024 Cyril Hrubis <chrubis@suse.cz>
 */

/**
 * DOC: Option parsing functions
 *
 * Implements simple helpers on the top of the strtol() and strtod() for
 * command line option parsing.
 */

#ifndef TST_PARSE_H__
#define TST_PARSE_H__

/**
 * tst_parse_int() - Parse an integer from a string.
 *
 * @str: A string with an integer number.
 * @val: A pointer to integer to store the result to.
 * @min: A lower bound, pass INT_MIN for full range.
 * @max: An upper bound, pass INT_MAX for full range.
 *
 * Return: A zero if whole string was consumed and the value was within bounds,
 * an errno otherwise.
 */
int tst_parse_int(const char *str, int *val, int min, int max);

/**
 * tst_parse_long() - Parse a long integer from a string.
 *
 * @str: A string with an integer number.
 * @val: A pointer to long integer to store the result to.
 * @min: A lower bound, pass LONG_MIN for full range.
 * @max: An upper bound, pass LONG_MAX for full range.
 *
 * Return: A zero if whole string was consumed and the value was within bounds,
 * an errno otherwise.
 */
int tst_parse_long(const char *str, long *val, long min, long max);

/**
 * tst_parse_float() - Parse a floating point number from a string.
 *
 * @str: A string with a floating point number.
 * @val: A pointer to float to store the result to.
 * @min: A lower bound.
 * @max: An upper bound.
 *
 * Return: A zero if whole string was consumed and the value was within bounds,
 * an errno otherwise.
 */
int tst_parse_float(const char *str, float *val, float min, float max);

/**
 * tst_parse_filesize() - Parse a file size from a string.
 *
 * @str: A string a positive number optionally followed by an unit, i.e. K, M,
 *       or G for kilobytes, megabytes and gigabytes.
 * @val: A pointer to long long integer to store the size in bytes to.
 * @min: A lower bound.
 * @max: An upper bound.
 *
 * Return: A zero if whole string was consumed and the value was within bounds,
 * an errno otherwise.
 */
int tst_parse_filesize(const char *str, long long *val, long long min, long long max);

#endif	/* TST_PARSE_H__ */
