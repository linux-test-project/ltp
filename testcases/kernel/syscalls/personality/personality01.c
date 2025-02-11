// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project
 * Copyright (c) International Business Machines  Corp., 2001
 *  03/2001 - Written by Wayne Boyer
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Tries to set different personalities.
 *
 * We set the personality in a child process since it's not guaranteed that we
 * can set it back in some cases. I.e. PER_LINUX32 cannot be unset on some 64
 * bit archs.
 */

#include "tst_test.h"
#include "lapi/personality.h"

#define PAIR(id) {id, #id}

struct personalities {
	unsigned long pers;
	const char *name;
};

static struct personalities pers[] = {
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

static void run(unsigned int i)
{
	pid_t pid;

	pid = SAFE_FORK();
	if (!pid) {
		SAFE_PERSONALITY(pers[i].pers);

		TST_EXP_EXPR((unsigned long)SAFE_PERSONALITY(0xffffffff) == pers[i].pers,
			"%s personality is set",
			 pers[i].name);

		return;
	}
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(pers),
	.forks_child = 1,
};
