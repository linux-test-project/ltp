// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Red Hat, Inc.
 * Author: Boyang Xue <bxue@redhat.com>
 *
 * Functional test for splice(2): pipe to pipe
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/splice.h"
#include "splice.h"

#define PIPE_MAX (64*1024)

static char *str_len_data;
static int num_len_data = PIPE_MAX;
static char *arr_in, *arr_out;

static void setup(void)
{
	int i, pipe_limit;

	pipe_limit = get_max_limit(num_len_data);
	num_len_data = pipe_limit;

	if (tst_parse_int(str_len_data, &num_len_data, 1, pipe_limit)) {
		tst_brk(TBROK, "Invalid length of data: '%s', "
			"valid value: [1, %d]", str_len_data, pipe_limit);
	}
	tst_res(TINFO, "splice size = %d", num_len_data);
	arr_in = SAFE_MALLOC(num_len_data);
	arr_out = SAFE_MALLOC(num_len_data);
	for (i = 0; i < num_len_data; i++)
		arr_in[i] = i & 0xff;
}

static void cleanup(void)
{
	free(arr_in);
	free(arr_out);
}

static void pipe_pipe(void)
{
	int pp1[2], pp2[2], i, ret;

	SAFE_PIPE(pp1);
	SAFE_PIPE(pp2);
	SAFE_WRITE(SAFE_WRITE_ALL, pp1[1], arr_in, num_len_data);
	for (i = num_len_data; i > 0; i = i - ret) {
		ret = splice(pp1[0], NULL, pp2[1], NULL, i, 0);
		if (ret == -1) {
			tst_res(TFAIL | TERRNO, "splice error");
			goto exit;
		}
		SAFE_READ(1, pp2[0], arr_out + num_len_data - i, ret);
	}

	for (i = 0; i < num_len_data; i++) {
		if (arr_in[i] != arr_out[i]) {
			tst_res(TFAIL, "wrong data at %d: expected: %d, "
				"actual: %d", i, arr_in[i], arr_out[i]);
			goto exit;
		}
	}
	tst_res(TPASS, "splice(2) from pipe to pipe run pass.");

exit:
	SAFE_CLOSE(pp1[1]);
	SAFE_CLOSE(pp1[0]);
	SAFE_CLOSE(pp2[1]);
	SAFE_CLOSE(pp2[0]);
}

static struct tst_test test = {
	.test_all = pipe_pipe,
	.setup = setup,
	.cleanup = cleanup,
	.options = (struct tst_option[]) {
		{"l:", &str_len_data, "Length of test data (in bytes)"},
		{}
	},
	.min_kver = "2.6.31"
};
