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
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/* Author: Miao Xie <miaox@cn.fujitsu.com>                                    */
/*                                                                            */
/******************************************************************************/

#define _GNU_SOURCE

#include "config.h"
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
#include <sys/syscall.h>
#include <inttypes.h>

#include "test.h"

char *TCID = "cpuset_syscall_test";
int TST_TOTAL = 1;

#ifdef HAVE_NUMA_V2
#include <numaif.h>

#include "../cpuset_lib/cpuset.h"
#include "../cpuset_lib/bitmask.h"

static unsigned long mask;
static int test = -1;
static int flag_exit;
static int ret;

#define OPT_setaffinity		(SCHAR_MAX + 1)
#define OPT_getaffinity		(SCHAR_MAX + 2)
#define OPT_mbind		(SCHAR_MAX + 3)
#define OPT_set_mempolicy	(SCHAR_MAX + 4)
#define OPT_get_mempolicy	(SCHAR_MAX + 5)

const struct option long_opts[] = {
	{"setaffinity", 1, NULL, OPT_setaffinity},
	{"getaffinity", 0, NULL, OPT_getaffinity},
	{"mbind", 1, NULL, OPT_mbind},
	{"set_mempolicy", 1, NULL, OPT_set_mempolicy},
	{"get_mempolicy", 0, NULL, OPT_get_mempolicy},
	{NULL, 0, NULL, 0},
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

void sigint_handler(int __attribute__ ((unused)) signo)
{
	flag_exit = 1;
}

void test_setaffinity(void)
{
	cpu_set_t tmask;
	unsigned int i;
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
	unsigned int i;
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
	ret = mbind(addr, len, MPOL_BIND, &mask, 8 * sizeof(mask), 0);
}

void test_set_mempolicy(void)
{
	ret = set_mempolicy(MPOL_BIND, &mask, 8 * sizeof(mask));
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
	ret = get_mempolicy(NULL, bitmask_mask(nmask), bitmask_nbits(nmask), 0,
			    MPOL_F_MEMS_ALLOWED);

	bitmask_displaylist(str, 256, nmask);
	puts(str);
}

void sigusr_handler(int __attribute__ ((unused)) signo)
{
	switch (test) {
	case 0:
		test_setaffinity();
		break;
	case 1:
		test_getaffinity();
		break;
	case 2:
		test_mbind();
		break;
	case 3:
		test_set_mempolicy();
		break;
	case 4:
		test_get_mempolicy();
		break;
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
int main(void)
{
	tst_brkm(TCONF, NULL, NUMA_ERROR_MSG);
}
#endif
