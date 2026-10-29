/*
 * vma01 - test not merging a VMA which cloned from parent process
 *
 * This program is used for testing the following upstream commit:
 * 965f55dea0e331152fa53941a51e4e16f9f06fae
 *
 * The cloned VMA shares the anon_vma lock with the parent process's
 * VMA. If we do the merge, more vmas (even the new range is only
 * for current process) use the perent process's anon_vma lock. This
 * introduces scalability issues. find_mergeable_anon_vma() already
 * considers this case.
 *
 * This test program clones VMA and checks /proc/self/maps file, on
 * an unpatched kernel, there is a single 6*ps VMA for the child
 * like this:
 *
 * 7fee32989000-7fee3298f000 -w-p 00000000 00:00 0
 *
 * On a patched kernel, there are two 3*ps VMAs like this:
 *
 * 7f55bbd47000-7f55bbd4a000 -w-p 00000000 00:00 0
 * 7f55bbd4a000-7f55bbd4d000 -w-p 00000000 00:00 0
 *
 * Copyright (C) 2011  Red Hat, Inc.
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
 */
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "tso_safe_macros.h"

#define MAPS_FILE "/proc/self/maps"

char *TCID = "vma01";
int TST_TOTAL = 1;

static void check_vma(void);
static void create_bighole(void);
static void *get_end_addr(void *addr_s);
static void check_status(int status);
static void setup(void);
static void cleanup(void);

static unsigned long ps;
static void *p;

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	ps = sysconf(_SC_PAGE_SIZE);
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		check_vma();
	}
	cleanup();
	tst_exit();
}

static void check_vma(void)
{
	int status;
	int topdown;
	void *t, *u, *x;

	create_bighole();
	t = mmap(p, 3 * ps, PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (t == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, cleanup, "mmap");
	memset(t, 1, ps);

	switch (fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");
	case 0:
		memset(t, 2, ps);
		u = mmap(t + 3 * ps, 3 * ps, PROT_WRITE,
			 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if (u == MAP_FAILED) {
			perror("mmap");
			exit(255);
		}
		topdown = (u > t) ? 0 : 1;
		printf("parent: t = %p\n", t);
		printf("child : u = %p\n", u);
		memset(u, 2, ps);

		if (topdown) {
			x = get_end_addr(u);
			printf("child : x = %p\n", x);
			if (x == t + 3 * ps)
				exit(1);
			else if (x == t && get_end_addr(x) == t + 3 * ps)
				exit(0);
		} else {
			x = get_end_addr(t);
			printf("child : x = %p\n", x);
			if (x == t + 6 * ps)
				exit(1);
			else if (x == u && get_end_addr(x) == t + 6 * ps)
				exit(0);
		}
		exit(255);
	default:
		SAFE_WAITPID(cleanup, -1, &status, 0);
		if (!WIFEXITED(status))
			tst_brkm(TBROK, cleanup, "child exited abnormally.");
		check_status(WEXITSTATUS(status));
		break;
	}
}

static void create_bighole(void)
{
	void *t;

	/*
	 * |-3*ps-|
	 * |------|------|------| hole
	 * t      p
	 * |======|------|        top-down alloc
	 *        |------|======| bottom-up alloc
	 */
	t = mmap(NULL, 9 * ps, PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (t == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, cleanup, "mmap");
	memset(t, 'a', ps);
	p = t + 3 * ps;
	SAFE_MUNMAP(cleanup, t, 9 * ps);
}

static void *get_end_addr(void *addr_s)
{
	FILE *fp;
	void *s, *t;
	char buf[BUFSIZ];

	fp = fopen(MAPS_FILE, "r");
	if (fp == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "fopen");
	while (fgets(buf, BUFSIZ, fp) != NULL) {
		if (sscanf(buf, "%p-%p ", &s, &t) != 2)
			continue;
		if (addr_s == s) {
			tst_resm(TINFO, "s = %p, t = %p", s, t);
			fclose(fp);
			return t;
		}
	}
	fclose(fp);
	tst_brkm(TBROK, cleanup, "no matched s = %p found.", addr_s);
}

static void check_status(int status)
{
	switch (status) {
	case 0:
		tst_resm(TPASS, "two 3*ps VMAs found.");
		break;
	case 1:
		tst_resm(TFAIL, "A single 6*ps VMA found.");
		break;
	default:
		tst_brkm(TBROK, cleanup, "unexpected VMA found.");
	}
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}
