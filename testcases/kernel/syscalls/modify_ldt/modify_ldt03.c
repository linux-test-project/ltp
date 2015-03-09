/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.
 */
/*
 * DESCRIPTION
 *	Basic test for modify_ldt(2) using func=0 argument.
 */

#include "config.h"
#include "test.h"

char *TCID = "modify_ldt03";
int TST_TOTAL = 1;

#if defined(__i386__) && defined(HAVE_MODIFY_LDT)

#ifdef HAVE_ASM_LDT_H
# include <asm/ldt.h>
#endif
extern int modify_ldt(int, void *, unsigned long);

#include <asm/unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include "safe_macros.h"

#ifdef HAVE_STRUCT_USER_DESC
# define SIZE sizeof(struct user_desc)
#else
# define SIZE 16
#endif

static char buf[SIZE];
static void cleanup(void);
static void setup(void);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(modify_ldt(0, buf, SIZE));

		if (TEST_RETURN < 0) {
			tst_resm(TFAIL | TTERRNO,
				 "modify_ldt() failed with errno: %s",
				 strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS, "modify_ldt() tested success");
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}

#elif HAVE_MODIFY_LDT

int main(void)
{
	tst_brkm(TCONF,
		 NULL, "modify_ldt is available but not tested on the platform than "
		 "__i386__");
}

#else /* if defined(__i386__) */

int main(void)
{
	tst_resm(TINFO, "modify_ldt03 test only for ix86");
	tst_exit();
}

#endif /* if defined(__i386__) */
