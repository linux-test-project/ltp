/*
 * Copyright (C) 2015 Cedric Hnyda ced.hnyda@gmail.com
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
 */

/*
 * AUTHOR   : Cédric Hnyda
 * DATE STARTED : 06/13/2015
 *
 *  Calls getrandom(2), checks that the buffer is filled with random bytes
 *  and expects success.
 *
 */

#include "lapi/getrandom.h"
#include "lapi/syscalls.h"
#include "tst_test.h"

static int modes[] = { 0, GRND_RANDOM, GRND_NONBLOCK,
		       GRND_RANDOM | GRND_NONBLOCK };

static int check_content(unsigned char *buf, int nb)
{
	int table[256];
	int i, index, max;

	memset(table, 0, sizeof(table));

	max = 6 + nb * 0.2;

	for (i = 0; i < nb; i++) {
		index = buf[i];
		table[index]++;
	}

	for (i = 0; i < nb; i++) {
		if (max > 0 && table[i] > max)
			return 0;
	}
	return 1;
}

static void verify_getrandom(unsigned int n)
{
	unsigned char buf[256];

	memset(buf, 0, sizeof(buf));

	do {
		TEST(tst_syscall(__NR_getrandom, buf, sizeof(buf), modes[n]));
	} while ((modes[n] & GRND_NONBLOCK) && TEST_RETURN == -1
		  && TEST_ERRNO == EAGAIN);

	if (!check_content(buf, TEST_RETURN))
		tst_res(TFAIL | TTERRNO, "getrandom failed");
	else
		tst_res(TPASS, "getrandom returned %ld", TEST_RETURN);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(modes),
	.test = verify_getrandom,
};
