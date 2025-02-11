// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Red Hat, Inc.
 */

/*\
 * Utilize kernel's symbol table for unauthorized address access.
 *
 * Access the system symbols with root permission to test whether it's
 * possible to read and write the memory addresses of kernel-space
 * from user-space. This helps in identifying potential vulnerabilities
 * where user-space processes can inappropriately access kernel memory.
 *
 * Steps:
 *
 *  1. Start a process that reads all symbols and their addresses from
 *     /proc/kallsyms and stores them in a linked list.
 *
 *  2. Attempt to write to each kernel address found in the linked list.
 *     The expectation is that each attempt will fail with a SIGSEGV
 *     (segmentation fault), indicating that the user-space process
 *     cannot write to kernel memory.
 *
 *  3. Handle each SIGSEGV using a signal handler that sets a flag and
 *     long jumps out of the faulting context.
 *
 *  4. If any write operation does not result in a SIGSEGV, log this as
 *     a potential security vulnerability.
 *
 *  5. Observe and log the behavior and any system responses to these
 *     unauthorized access attempts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>

#include "tst_test.h"
#include "tst_safe_stdio.h"

struct kallsym {
	unsigned long addr;
	char type;
	char name[128];
};

struct range_struct {
	unsigned long start, end;

};

static struct kallsym *sym_table;
static unsigned int nr_symbols;
static sigjmp_buf jmpbuf;
volatile sig_atomic_t segv_caught;
static struct range_struct *ranges;
static int ranges_size, ranges_len;

static void segv_handler(int sig)
{
	if (sig == SIGSEGV)
		segv_caught++;
	else
		tst_res(TFAIL, "Unexpected signal %s", strsignal(sig));

	siglongjmp(jmpbuf, 1);
}

static unsigned int read_kallsyms(struct kallsym *table, unsigned int table_size)
{
	char *line = NULL;
	size_t len = 0;
	unsigned int nr_syms = 0;
	FILE *stream = SAFE_FOPEN("/proc/kallsyms", "r");

	while (getline(&line, &len, stream) != -1) {

		if (table && nr_syms < table_size) {
			sscanf(line, "%lx %c %s",
					&table[nr_syms].addr,
					&table[nr_syms].type,
					table[nr_syms].name);
		}

		nr_syms++;
	}

	SAFE_FCLOSE(stream);

	return nr_syms;
}

static void read_proc_self_maps(void)
{
	FILE *fp;

	ranges_len = 0;
	fp = fopen("/proc/self/maps", "r");
	if (fp == NULL)
		tst_brk(TBROK | TERRNO, "Failed to open /proc/self/maps.");

	while (!feof(fp)) {
		unsigned long start, end;
		int ret;

		ret = fscanf(fp, "%lx-%lx %*[^\n]\n", &start, &end);
		if (ret != 2) {
			fclose(fp);
			tst_brk(TBROK | TERRNO, "Couldn't parse /proc/self/maps line.");
		}

		if (ranges_size < ranges_len + 1) {
			ranges_size += 128;
			ranges = SAFE_REALLOC(ranges,
				ranges_size*sizeof(struct range_struct));
		}
		ranges[ranges_len].start = start;
		ranges[ranges_len].end = end;
		ranges_len++;
	}

	fclose(fp);
}

static int is_address_mapped(unsigned long addr)
{
	int i;

	for (i = 0; i < ranges_len; i++) {
		if (ranges[i].start <= addr && addr < ranges[i].end)
			return 1;
	}
	return 0;
}

static void setup(void)
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = segv_handler;
	sigaction(SIGSEGV, &sa, NULL);

	nr_symbols = read_kallsyms(NULL, 0);
	sym_table = SAFE_CALLOC(nr_symbols, sizeof(*sym_table));
	unsigned int read_symbols = read_kallsyms(sym_table, nr_symbols);

	if (nr_symbols != read_symbols)
		tst_res(TWARN, "/proc/kallsyms changed size!?");
}

static void access_ksymbols_address(struct kallsym *table)
{
	tst_res(TDEBUG, "Access kernel addr: 0x%lx (%c) (%s)",
				table->addr, table->type, table->name);

	if (sigsetjmp(jmpbuf, 1) == 0) {
		*(volatile unsigned long *)table->addr = 0;

		tst_res(TFAIL, "Successfully accessed kernel addr 0x%lx (%c) (%s)",
				table->addr, table->type, table->name);
	}
}


static void test_access_kernel_address(void)
{
	int skipped = 0;

	segv_caught = 0;
	read_proc_self_maps();

	for (unsigned int i = 0; i < nr_symbols; i++) {
		if (is_address_mapped(sym_table[i].addr)) {
			tst_res(TDEBUG, "Skipping userspace mapped address 0x%lx",
				sym_table[i].addr);
			skipped++;
			continue;
		}
		access_ksymbols_address(&sym_table[i]);
	}

	if (segv_caught == (sig_atomic_t)nr_symbols - skipped)
		tst_res(TPASS, "Caught %d SIGSEGV in access ksymbols addr, skipped %d",
			segv_caught, skipped);
	else
		tst_res(TFAIL, "Caught %d SIGSEGV but expected %d, skipped %d",
			segv_caught, nr_symbols-skipped, skipped);
}

static void cleanup(void)
{
	if (sym_table)
		free(sym_table);
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.timeout = 60,
	.needs_kconfigs = (const char *const[]){
		"CONFIG_KALLSYMS=y",
		NULL
	},
	.test_all = test_access_kernel_address,
};
