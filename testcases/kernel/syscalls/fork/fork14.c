/*********************************************************************
 * Copyright (C) 2014  Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * This test is a reporducer for this patch:
 *              https://lkml.org/lkml/2012/4/24/328
 * Since vma length in dup_mmap is calculated and stored in a unsigned
 * int, it will overflow when length of mmaped memory > 16 TB. When
 * overflow occur, fork will  incorrectly succeed. The patch above
 * fixed it.
 ********************************************************************/

#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/abisize.h"

char *TCID = "fork14";
int TST_TOTAL = 1;

#define GB		(1024 * 1024 * 1024L)

/* set mmap threshold to 16TB */
#define LARGE		(16 * 1024)
#define EXTENT		(16 * 1024 + 10)

static char **pointer_vec;

static void setup(void);
static void cleanup(void);
static int  fork_test(void);

int main(int ac, char **av)
{
	int lc, reproduced;

	tst_parse_opts(ac, av, NULL, NULL);
/*
 * Tested on ppc64/x86_64/i386/s390x. And only 64bit has this issue.
 * Since a 32bit program can't mmap so many memory.
 */
#ifdef TST_ABI32
	tst_brkm(TCONF, NULL, "This test is only for 64bit.");
#endif
	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		reproduced = fork_test();
		if (reproduced == 0)
			tst_resm(TPASS, "fork failed as expected.");
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	pointer_vec = SAFE_MALLOC(cleanup, EXTENT * sizeof(char *));
}

static void cleanup(void)
{
	free(pointer_vec);
}

static int fork_test(void)
{
	int i, j, prev_failed = 0, fails = 0;
	int reproduced = 0;
	void *addr;

	for (i = 0; i < EXTENT; i++) {
		addr = mmap(NULL, 1 * GB, PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
		if (addr == MAP_FAILED) {
			pointer_vec[i] = NULL;
			fails++;
			/*
			 * EXTENT is "16*1024+10", if fails count exceeds 10,
			 * we are almost impossible to get an vm_area_struct
			 * sized 16TB
			 */
			if (fails == 11) {
				tst_brkm(TCONF, cleanup, "mmap() fails too many"
					 "times, so we are almost impossible to"
					 " get an vm_area_struct sized 16TB.");
			}
		} else {
			pointer_vec[i] = addr;
		}

		switch (tst_fork()) {
		case -1:
			prev_failed = 1;
		break;
		case 0:
			exit(0);
		default:
			SAFE_WAITPID(cleanup, -1, NULL, 0);

			if (prev_failed > 0 && i >= LARGE) {
				tst_resm(TFAIL, "Fork succeeds incorrectly");
				reproduced = 1;
				goto clear_memory_map;
			}
		}
	}

clear_memory_map:
	for (j = 0; j <= i; j++) {
		if (pointer_vec[j])
			SAFE_MUNMAP(cleanup, pointer_vec[j], 1 * GB);
	}

	return reproduced;
}
