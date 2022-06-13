// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <chrubis@suse.cz>
 */

#define TST_NO_DEFAULT_MAIN
#include <stdlib.h>
#include "tst_test.h"

int main(int argc, char *argv[])
{
	tst_reinit();

	if (argc != 1) {
		tst_res(TFAIL, "argc is %d, expected 1", argc);
		return 0;
	}

	if (!argv[0]) {
		tst_res(TFAIL, "argv[0] == NULL");
		return 0;
	}

	tst_res(TPASS, "argv[0] was filled in by kernel");

	return 0;
}
