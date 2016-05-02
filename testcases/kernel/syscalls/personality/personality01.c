/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  03/2001 - Written by Wayne Boyer
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Tries to set different personalities.
 *
 * We set the personality in a child process since it's not guaranteed that we
 * can set it back in some cases. I.e. PER_LINUX32 cannot be unset on some 64
 * bit archs.
 */

#include "test.h"
#include <sys/personality.h>

char *TCID = "personality01";

#define PAIR(id) {id, #id}

struct personalities {
	unsigned long int pers;
	const char *name;
};

struct personalities pers[] = {
	PAIR(PER_LINUX),
	PAIR(PER_LINUX_32BIT),
	PAIR(PER_SVR4),
	PAIR(PER_SVR3),
	PAIR(PER_SCOSVR3),
	PAIR(PER_OSR5),
	PAIR(PER_WYSEV386),
	PAIR(PER_ISCR4),
	PAIR(PER_BSD),
	PAIR(PER_XENIX),
#if defined(__x86_64__)
	PAIR(PER_LINUX32),
#endif
	PAIR(PER_IRIX32),
	PAIR(PER_IRIXN32),
	PAIR(PER_IRIX64),
	PAIR(PER_RISCOS),
	PAIR(PER_SOLARIS),
	PAIR(PER_UW7),
	PAIR(PER_OSF4),
	PAIR(PER_HPUX),
};

int TST_TOTAL = ARRAY_SIZE(pers);

static void do_child(unsigned int i)
{
	int ret;

	ret = personality(pers[i].pers);
	if (ret < 0) {
		tst_resm(TFAIL | TERRNO, "personality(%s) failed", pers[i].name);
		return;
	}

	ret = personality(0xffffffff);

	if ((unsigned long)ret != pers[i].pers) {
		tst_resm(TFAIL,
			 "%s: wrong personality read back %d expected %lu",
			 pers[i].name, ret, pers[i].pers);
		return;
	}

	tst_resm(TPASS, "personality(%s)", pers[i].name);
}

static void verify_personality(unsigned int i)
{
	pid_t pid;

	pid = tst_fork();
	switch (pid) {
	case 0:
		do_child(i);
		tst_exit();
	break;
	case -1:
		tst_brkm(TBROK | TERRNO, NULL, "fork() failed");
	break;
	default:
		tst_record_childstatus(NULL, pid);
	break;
	}
}

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		for (i = 0; i < TST_TOTAL; i++) {
			verify_personality(i);
		}
	}

	tst_exit();
}
