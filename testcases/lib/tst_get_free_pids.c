// SPDX-License-Identifier: GPL-2.0-or-later

#define TST_NO_DEFAULT_MAIN
#include <stdio.h>
#include <tst_test.h>

extern struct tst_test *tst_test;

static struct tst_test test = {
};

int main(void)
{
	/* force messages to be printed from new library */
	tst_test = &test;

	printf("%i\n", tst_get_free_pids());

	return 0;
}
