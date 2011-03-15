/******************************************************************************/
/*                                                                            */
/* Copyright (c) 2009 FUJITSU LIMITED                                         */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/* Author: Miao Xie <miaox@cn.fujitsu.com>                                    */
/*                                                                            */
/******************************************************************************/

#define _GNU_SOURCE

#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <err.h>
#include <limits.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <syscall.h>
#include <inttypes.h>
#include "config.h"
#include "linux_syscall_numbers.h"
#include "test.h"
#include "usctest.h"

char *TCID = "cpuset_syscall_test";

#if HAVE_LINUX_MEMPOLICY_H
#include <linux/mempolicy.h>

#include "../cpuset_lib/cpuset.h"
#include "../cpuset_lib/bitmask.h"

int TST_TOTAL = 1;

unsigned long mask;
int test = -1;
int flag_exit;
int ret;

#if HAVE_DECL_MPOL_F_MEMS_ALLOWED
static int get_mempolicy(int *policy, unsigned long *nmask,
			unsigned long maxnode, void *addr, int flags)
{
	return syscall(__NR_get_mempolicy, policy, nmask, maxnode, addr, flags);
}
#endif

#if HAVE_DECL_MPOL_BIND
static int mbind(void *start, unsigned long len, int policy, unsigned long *nodemask,
		unsigned long maxnode, unsigned flags)
{
	return syscall(__NR_mbind, start, len, policy, nodemask, maxnode, flags);
}

static int set_mempolicy(int policy, unsigned long *nodemask, unsigned long maxnode)
{
	return syscall(__NR_set_mempolicy, policy, nodemask, maxnode);
}
#endif

#define OPT_setaffinity		(SCHAR_MAX + 1)
#define OPT_getaffinity		(SCHAR_MAX + 2)
#define OPT_mbind		(SCHAR_MAX + 3)
#define OPT_set_mempolicy	(SCHAR_MAX + 4)
#define OPT_get_mempolicy	(SCHAR_MAX + 5)

const struct option long_opts[] = {
	{ "setaffinity",	1, NULL, OPT_setaffinity	},
	{ "getaffinity",	0, NULL, OPT_getaffinity	},
	{ "mbind",		1, NULL, OPT_mbind		},
	{ "set_mempolicy",	1, NULL, OPT_set_mempolicy	},
	{ "get_mempolicy",	0, NULL, OPT_get_mempolicy	},
	{ NULL,			0, NULL, 0			},
};

void process_options(int argc, char *argv[])
{
	int c;
	char *end;

	while (1) {
		c = getopt_long(argc, argv, "", long_opts, NULL);
		if (c == -1)
			break;

		switch (c) {
		case OPT_setaffinity:
			test = 0;
			mask = strtoul(optarg, &end, 10);
			if (*end != '\0')
				errx(1, "wrong -s argument!");
			break;
		case OPT_getaffinity:
			test = 1;
			break;
		case OPT_mbind:
			test = 2;
			mask = strtoul(optarg, &end, 10);
			if (*end != '\0')
				errx(1, "wrong -s argument!");
			break;
		case OPT_set_mempolicy:
			test = 3;
			mask = strtoul(optarg, &end, 10);
			if (*end != '\0')
				errx(1, "wrong -s argument!");
			break;
		case OPT_get_mempolicy:
			test = 4;
			break;
		default:
			errx(1, "unknown option!\n");
			break;
		}
	}
}

void sigint_handler(int __attribute__((unused)) signo)
{
	flag_exit = 1;
}

void test_setaffinity(void)
{
	cpu_set_t tmask;
	int i;
	CPU_ZERO(&tmask);
	for (i = 0; i < 8 * sizeof(mask); i++) {
		if ((1 << i) & mask)
			CPU_SET(i, &tmask);
	}
	ret = sched_setaffinity(0, sizeof(tmask), &tmask);
}

void test_getaffinity(void)
{
	cpu_set_t tmask;
	int i;
	CPU_ZERO(&tmask);
	ret = sched_getaffinity(0, sizeof(tmask), &tmask);
	for (i = 0; i < 8 * sizeof(mask); i++) {
		if (CPU_ISSET(i, &tmask))
			printf("%d,", i);
	}
}

void test_mbind(void)
{
	void *addr;
	int len = 10 * 1024 * 1024;
	addr = mmap(NULL, len, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (addr == MAP_FAILED) {
		ret = 1;
		return;
	}
	printf("%p\n", addr);
#if HAVE_DECL_MPOL_BIND
	ret = mbind(addr, len, MPOL_BIND, &mask, 8 * sizeof(mask), 0);
#else
	ret = 1;
#endif
}

void test_set_mempolicy(void)
{
#if HAVE_DECL_MPOL_BIND
	ret = set_mempolicy(MPOL_BIND, &mask, 8 * sizeof(mask));
#else
	ret = -1;
#endif
}

void test_get_mempolicy(void)
{
	int nbits;
	struct bitmask *nmask;
	char str[256];

	nbits = cpuset_mems_nbits();
	if (nbits <= 0) {
		warn("get the nbits of nodes failed");
		ret = 1;
		return;
	}

	nmask = bitmask_alloc(nbits);
	if (nmask == NULL) {
		warn("alloc bitmask failed");
		ret = 1;
		return;
	}
#if HAVE_DECL_MPOL_F_MEMS_ALLOWED
	ret = get_mempolicy(NULL, bitmask_mask(nmask), bitmask_nbits(nmask), 0,
				MPOL_F_MEMS_ALLOWED);
#else
	ret = -1;
#endif

	bitmask_displaylist(str, 256, nmask);
	puts(str);
}

void sigusr_handler(int __attribute__((unused)) signo)
{
	switch (test) {
	case 0: test_setaffinity(); break;
	case 1: test_getaffinity(); break;
	case 2: test_mbind(); break;
	case 3: test_set_mempolicy(); break;
	case 4: test_get_mempolicy(); break;
	default:;
	}
	test = -1;
}

int main(int argc, char *argv[])
{
	struct sigaction sigint_action;
	struct sigaction sigusr_action;

	memset(&sigint_action, 0, sizeof(sigint_action));
	sigint_action.sa_handler = &sigint_handler;
	sigaction(SIGINT, &sigint_action, NULL);

	memset(&sigusr_action, 0, sizeof(sigusr_action));
	sigusr_action.sa_handler = &sigusr_handler;
	sigaction(SIGUSR1, &sigusr_action, NULL);

	process_options(argc, argv);

	while (!flag_exit)
		sleep(1);

	return ret;
}
#else
int main (void) {
	printf("System doesn't have required mempolicy support\n");
	tst_exit();
}
#endif
