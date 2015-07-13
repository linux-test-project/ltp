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
#include "linux_syscall_numbers.h"
#include "test.h"

char *TCID = "getrandom02";
static int modes[] = { 0, GRND_RANDOM, GRND_NONBLOCK,
		       GRND_RANDOM | GRND_NONBLOCK };

int TST_TOTAL = ARRAY_SIZE(modes);

static unsigned char buf[256];
static size_t size = 256;

static void fill(void);
static int check_content(int nb);

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			fill();

			do {
				TEST(ltp_syscall(__NR_getrandom, buf, size,
					modes[i]));
			} while ((modes[i] & GRND_NONBLOCK) && TEST_RETURN == -1
				&& TEST_ERRNO == EAGAIN);

			if (!check_content(TEST_RETURN))
				tst_resm(TFAIL | TTERRNO, "getrandom failed");
			else
				tst_resm(TPASS, "getrandom returned %ld",
						TEST_RETURN);
		}
	}
	tst_exit();
}

static void fill(void)
{
	memset(buf, '\0', sizeof(buf));
}

static int check_content(int nb)
{
	int table[256];
	int i, index, max;

	memset(table, 0, sizeof(table));

	max = nb * 0.10;

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
