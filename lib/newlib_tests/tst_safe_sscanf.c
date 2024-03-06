// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Linux Test Project
 */

/*
 * Basic unit test for the tst_safe_sscanf() function.
 */

#include "tst_test.h"

static void check_safe_sscanf(void)
{
	int day, month, year;
	int nvalues = SAFE_SSCANF("14-07-2023", "%d-%d-%d", &day, &month, &year);

	if (nvalues == 3 && day == 14 && month == 7 && year == 2023)
		tst_res(TPASS, "%d values parsed : %d,%d,%d", nvalues, day, month, year);
	else
		tst_res(TFAIL, "expected 3 values 14,7,2023 got: %d values %d,%d,%d", nvalues, day, month, year);
}

static struct tst_test test = {
	.test_all = check_safe_sscanf,
};
