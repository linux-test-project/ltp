/*
 * Copyright (C) 2012 Marios Makris <marios.makris@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 */

/*
 * Test program for the bytes_by_prefix program in /lib
 *
 * This program tests a few predefined values against the expected predefined
 * results, upon sucesfull completion, it prints the message:
 * "Tests sucesfully completed!" else it prints that there were an error
 * and the value as well as the type on which it failed (int, long, long long)
 * at the time of the error in order for someone to be able to trace
 * it back as well as the total number of errors encountered along with the
 * message: "Some test(s): (number of tests) failed please review!"
 */

#include <stdio.h>
#include <stdlib.h>

#include "bytes_by_prefix.h"

struct test_vals {
	char *val;
	long long res;
};

/*
 * Array with generic values suitable for all operations.
 */
struct test_vals test[] = {
	{"1", 1},
	{"5", 5},
	{"10", 10},
	{"552558", 552558},
	{"0", 0},
	{"1b", 512},
	{"5b", 2560},
	{"0b", 0},
	{"1k", 1024},
	{"5k", 5120},
	{"552558k", 565819392},
	{"0k", 0},
	{"1m", 1048576},
	{"5m", 5242880},
	{"0m", 0},
	{"1g", 1073741824},
	{"0g", 0},
	/*
	 * Negative Test Values
	 */
	{"a", -1},
	{"k", -1},
	{"m", -1},
	{"g", -1},
	{"K", -1},
	{"M", -1},
	{"G", -1},
	{"5km", -1},
	{"1a", -1},
	{"1mabc", -1},
	{"a1", -1},
	{"k1", -1},
	{"1 k", -1},
	{"1k g", -1},
	{"-5", -1},
	{"-5b", -1},
	{"-2k", -1},
	{"-2m", -1},
	{"-2g", -1},
	{"-2K", -1},
	{"-2M", -1},
	{"-2G", -1}
};

/*
 * Specific values for int operations
 */
struct test_vals test_i[] = {
/*
 * In case of 64b system as the results of capital multipliers are multiplied
 * by the sizeof(long) or sizeof(long long) respectively.
 * Check "/lib/bytes_by_prefix.c" file for more information.
 */
#if __SIZEOF_LONG__ == 8
	{"5K", 40960},
	{"0K", 0},
	{"5M", 41943040}
/*
 * In case of 32b system as the results of capital multipliers are multiplied
 * by the sizeof(long) or sizeof(long long) respectively.
 * Check "/lib/bytes_by_prefix.c" file for more information.
 */
#else
	{"5K", 20480},
	{"0K", 0},
	{"5M", 20971520}
#endif
};

/*
 * Specific values for long operations
 */
struct test_vals test_l[] = {
/*
 * In case of 64b system as the results of capital multipliers are multiplied
 * by the sizeof(long) or sizeof(long long) respectively.
 * Check "/lib/bytes_by_prefix.c" file for more information.
 */
#if __SIZEOF_LONG__ == 8
	{"552558m", 579399057408},
	{"5g", 5368709120},
	{"5K", 40960},
	{"5M", 41943040},
	{"1G", 8589934592}
/*
 * In case of 32b system as the results of capital multipliers are multiplied
 * by the sizeof(long) or sizeof(long long) respectively.
 * Check "/lib/bytes_by_prefix.c" file for more information.
 */
#else
	{"552558m", -1},
	{"5g", -1},
	{"5K", 20480},
	{"5M", 20971520},
	{"1G", -1}
#endif
};

/*
 * Specific values for long long operations
 */
struct test_vals test_ll[] = {
	{"552558m", 579399057408LL},
	{"5g", 5368709120LL},
	{"5K", 40960},
	{"552558K", 4526555136LL},
	{"5M", 41943040},
	{"552558M", 4635192459264LL},
	{"5G", 42949672960LL},
	{"552558G", 4746437078286336LL}
};


static int test_values(void)
{
	/*
	 * 1st position of the array denotes the valid int operations
	 * 2nd position of the array denotes the valid long operations
	 * 3rd position of the array denotes the valid long long operations
	 */
	int valid_count[3];
	int tests_number[3]; /* int / long / long long */
	int i;
	int error_count = 0;
	int elements;	/* Number of elements inside the test array. */

	elements = sizeof(test)/sizeof(struct test_vals);
	/*
	 * Initializing counters.
	 */
	for (i = 0; i < 3; i++) {
		valid_count[i] = 0;
		tests_number[i] = elements;
	}

	/*
	 * The "generic" test loop. If the result of the function equals the
	 * expected predifined result, then increase the valid counter
	 */
	for (i = 0; i < elements; i++) {
		if (bytes_by_prefix(test[i].val) == test[i].res) {
			valid_count[0]++;
		} else {
			printf("Test value:%s failed on int.\n", test[i].val);
			error_count++;
		}

		if (lbytes_by_prefix(test[i].val) == test[i].res) {
			valid_count[1]++;
		} else {
			printf("Test value:%s failed on long.\n", test[i].val);
			error_count++;
		}

		if (llbytes_by_prefix(test[i].val) == test[i].res) {
			valid_count[2]++;
		} else {
			printf("Test value:%s failed on long long.\n",
				test[i].val);
			error_count++;
		}
	}

	elements = sizeof(test_i)/sizeof(struct test_vals);
	tests_number[0] += elements;

	/*
	 * Int specific test loop
	 */
	for (i = 0; i < elements; i++) {
		if (bytes_by_prefix(test_i[i].val) == test_i[i].res) {
			valid_count[0]++;
		} else {
			printf("Test value:%s failed on int.\n",
				test_i[i].val);
			error_count++;
		}
	}

	elements = sizeof(test_l)/sizeof(struct test_vals);
	tests_number[1] += elements;

	/*
	 * Long specific test loop
	 */
	for (i = 0; i < elements; i++) {
		if (lbytes_by_prefix(test_l[i].val) == test_l[i].res) {
			valid_count[1]++;
		} else {
			printf("Test value:%s failed on long.\n",
				test_l[i].val);
			error_count++;
		}
	}

	elements = sizeof(test_ll)/sizeof(struct test_vals);
	tests_number[2] += elements;

	/*
	 * Long long specific test loop
	 */
	for (i = 0; i < elements; i++) {
		if (llbytes_by_prefix(test_ll[i].val) == test_ll[i].res) {
			valid_count[2]++;
		} else {
			printf("Test value:%s failed on long long.\n",
				test_ll[i].val);
			error_count++;
		}
	}

	fprintf(stdout, "Succesfull int tests:%d/%d\n", valid_count[0],
		tests_number[0]);
	fprintf(stdout, "Succesfull long tests:%d/%d\n", valid_count[1],
		tests_number[1]);
	fprintf(stdout, "Succesfull long long tests:%d/%d\n", valid_count[2],
		tests_number[2]);

	return error_count;
}

int main(void)
{
	int errors = test_values();

	if (errors > 0) {
		fprintf(stderr, "\nSome test(s):(%d) failed please review!\n",
			errors);
		exit(1);
	} else {
		fprintf(stdout, "Tests succesfully completed!\n");
	}

	return 0;
}
