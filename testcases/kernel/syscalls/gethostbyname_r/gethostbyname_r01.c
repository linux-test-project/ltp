/*
 *   Copyright (c) 2015 Fujitsu Ltd.
 *   Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 */

/*
 * This is a test for glibc bug:
 * https://www.qualys.com/research/security-advisories/GHOST-CVE-2015-0235.txt
 */

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "test.h"

#define CANARY "in_the_coal_mine"

static void setup(void);
static void check_vulnerable(void);

static struct {
	char buffer[1024];
	char canary[sizeof(CANARY)];
} temp = {
	"buffer",
	CANARY,
};

char *TCID = "gethostbyname_r01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		check_vulnerable();
	}

	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, NULL);
	TEST_PAUSE;
}

static void check_vulnerable(void)
{
	struct hostent resbuf;
	struct hostent *result;
	int herrno;
	int retval;
	char name[sizeof(temp.buffer)];
	size_t len;

	/*
	 * <glibc>/nss/digits_dots.c:
	 * strlen(name) = size_needed - sizeof(*host_addr) -
	 *                sizeof(*h_addr_ptrs) - 1;
	 */
	len = sizeof(temp.buffer) - 16 - 2 * sizeof(char *) - 1;
	memset(name, '0', len);
	name[len] = '\0';

	retval = gethostbyname_r(name, &resbuf, temp.buffer,
				 sizeof(temp.buffer), &result, &herrno);

	if (strcmp(temp.canary, CANARY) != 0) {
		tst_resm(TFAIL, "vulnerable");
		return;
	}

	if (retval == ERANGE) {
		tst_resm(TPASS, "not vulnerable");
		return;
	}

	tst_resm(TFAIL, "gethostbyname_r() returned %s, expected ERANGE",
		 tst_strerrno(retval));
}
