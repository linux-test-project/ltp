// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Shell test C child example.
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

int main(void)
{
	tst_reinit();

	tst_res(TPASS, "C child works fine!");

	return 0;
}
