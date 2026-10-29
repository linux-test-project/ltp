// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2002
 * Copyright (c) 2026 Jan Stancek <jstancek@redhat.com>
 */

/*\
 * Verify that :manpage:`nextafter(3)` returns the next representable double
 * in the direction of its second argument:
 *
 * - nextafter(1.0, 1.1) is 1.0 + 2^-52 (DBL_EPSILON)
 * - nextafter(1.0, 0.9) is 1.0 - 2^-53
 * - nextafter(1.0, 1.0) is 1.0
 */

#include <math.h>
#include "tst_test.h"

/* DBL_EPSILON spelled out, as sparse does not know __DBL_EPSILON__ */
#define EPSILON (1.0 / (1ULL << 52))

static struct tcase {
	double x;
	double y;
	double exp;
} tcases[] = {
	{1.0, 1.1, 1.0 + EPSILON},
	{1.0, 0.9, 1.0 - EPSILON / 2},
	{1.0, 1.0, 1.0},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	double ret = nextafter(tc->x, tc->y);

	if (ret == tc->exp) {
		tst_res(TPASS, "nextafter(%g, %g) == %.17g", tc->x, tc->y, ret);
	} else {
		tst_res(TFAIL, "nextafter(%g, %g) == %.17g, expected %.17g",
			tc->x, tc->y, ret, tc->exp);
	}
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
