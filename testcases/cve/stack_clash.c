// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Pavel Boldin <pboldin@cloudlinux.com>
 * Copyright (c) 2023 Rick Edgecombe <rick.p.edgecombe@intel.com>
 */

/* This is a regression test of the Stack Clash [1] vulnerability. This tests
 * that there is at least 256 PAGE_SIZE of stack guard gap which is considered
 * hard to hop above. Code adapted from the Novell's bugzilla [2].
 *
 * The code `mmap(2)`s region close to the stack end. The code then allocates
 * memory on stack until it hits guard page and SIGSEGV or SIGBUS is generated
 * by the kernel. The signal handler checks that fault address is further than
 * THRESHOLD from the mmapped area.
 *
 * We read /proc/self/maps to examine exact top of the stack and `mmap(2)`
 * our region exactly GAP_PAGES * PAGE_SIZE away. We read /proc/cmdline to
 * see if a different stack_guard_gap size is configured. We set stack limit
 * to infinity and preallocate REQ_STACK_SIZE bytes of stack so that no calls
 * after `mmap` are moving stack further.
 *
 * If the architecture meets certain requirements (only x86_64 is verified)
 * then the test also tests that new mmap()s can't be placed in the stack's
 * guard gap. This part of the test works by forcing a bottom up search. The
 * assumptions are that the stack grows down (start gap) and either:
 *   1. The default search is top down, and will switch to bottom up if
 *      space is exhausted.
 *   2. The default search is bottom up and the stack is above mmap base
 *
 * [1] https://blog.qualys.com/securitylabs/2017/06/19/the-stack-clash
 * [2] https://bugzilla.novell.com/show_bug.cgi?id=CVE-2017-1000364
 */

#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <alloca.h>
#include <signal.h>
#include <stdlib.h>

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "lapi/mmap.h"

static unsigned long page_size;
static unsigned long page_mask;
static unsigned long GAP_PAGES = 256;
static unsigned long THRESHOLD;
static int STACK_GROWSDOWN;

#define SIGNAL_STACK_SIZE	(1UL<<20)
#define FRAME_SIZE		1024
#define REQ_STACK_SIZE		(1024 * 1024)

#define EXIT_TESTBROKE		TBROK

void exhaust_stack_into_sigsegv(void)
{
	volatile char * ptr = alloca(FRAME_SIZE - sizeof(long));
	*ptr = '\0';
	exhaust_stack_into_sigsegv();
}

#define MAPPED_LEN page_size
static unsigned long mapped_addr;

void segv_handler(int sig, siginfo_t *info, void *data LTP_ATTRIBUTE_UNUSED)
{
	unsigned long fault_addr = (unsigned long)info->si_addr;
	unsigned long mmap_end = mapped_addr + MAPPED_LEN;
	ssize_t diff;

	if (sig != SIGSEGV && sig != SIGBUS)
		return;

	if (STACK_GROWSDOWN)
		diff = fault_addr - mmap_end;
	else
		diff = mapped_addr - fault_addr;

	tst_res(TINFO,
		"mmap = [%lx, %lx), addr = %lx, diff = %zx, THRESHOLD = %lx",
		mapped_addr, mmap_end, fault_addr, diff, THRESHOLD);
	if (diff < 0 || (unsigned long)diff < THRESHOLD)
		_exit(EXIT_FAILURE);
	else
		_exit(EXIT_SUCCESS);
}

static void force_bottom_up(void)
{
	FILE *fh;
	char buf[1024];
	unsigned long start, end, size, lastend = 0;

	/* start filling from mmap_min_addr */
	SAFE_FILE_SCANF("/proc/sys/vm/mmap_min_addr", "%lu", &lastend);

	fh = SAFE_FOPEN("/proc/self/maps", "r");

	while (!feof(fh)) {
		if (fgets(buf, sizeof(buf), fh) == NULL)
			goto out;

		if (sscanf(buf, "%lx-%lx", &start, &end) != 2) {
			tst_brk(TBROK | TERRNO, "sscanf");
			goto out;
		}

		size = start - lastend;

		/* Skip the PROT_NONE that was just added (!size). */
		if (!size) {
			lastend = end;
			continue;
		}

		/* If the next area is the stack, quit. */
		if (!!strstr(buf, "[stack]"))
			break;

		/* This is not cleaned up. */
		SAFE_MMAP((void *)lastend, size, PROT_NONE,
			  MAP_ANON|MAP_PRIVATE|MAP_FIXED_NOREPLACE, -1, 0);

		lastend = end;
	}

out:
	SAFE_FCLOSE(fh);
}

unsigned long read_stack_addr_from_proc(unsigned long *stack_size)
{
	FILE *fh;
	char buf[1024];
	unsigned long stack_top = -1UL, start, end;

	fh = SAFE_FOPEN("/proc/self/maps", "r");

	while (!feof(fh)) {
		if (fgets(buf, sizeof(buf), fh) == NULL) {
			tst_brk(TBROK | TERRNO, "fgets");
			goto out;
		}

		if (!strstr(buf, "[stack"))
			continue;

		if (sscanf(buf, "%lx-%lx", &start, &end) != 2) {
			tst_brk(TBROK | TERRNO, "sscanf");
			goto out;
		}

		*stack_size = end - start;

		if (STACK_GROWSDOWN)
			stack_top = start;
		else
			stack_top = end;
		break;
	}

out:
	SAFE_FCLOSE(fh);
	return stack_top;
}

void dump_proc_self_maps(void)
{
	static char buf[64];
	static const char *cmd[] = {"cat", buf, NULL};
	sprintf(buf, "/proc/%d/maps", getpid());
	tst_cmd(cmd, NULL, NULL, 0);
}

void __attribute__((noinline)) preallocate_stack(unsigned long required)
{
	volatile char *garbage;

	garbage = alloca(required);
	garbage[0] = garbage[required - 1] = '\0';
}

static void do_mmap_placement_test(unsigned long stack_addr, unsigned long gap)
{
	void *map_test_gap;

	force_bottom_up();

	/*
	 * force_bottom_up() used up all the spaces below the stack. The search down
	 * path should fail, and search up might take a look at the guard gap
	 * region. If it avoids it, the allocation will be above the stack. If it
	 * uses it, the allocation will be in the gap and the test should fail.
	 */
	map_test_gap = SAFE_MMAP(0, MAPPED_LEN,
				 PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, 0, 0);

	if (stack_addr - gap <= (unsigned long)map_test_gap &&
		(unsigned long)map_test_gap <= stack_addr) {
		tst_res(TFAIL, "New mmap was placed in the guard gap.");
		SAFE_MUNMAP(map_test_gap, MAPPED_LEN);
	}
}

void do_child(void)
{
	unsigned long stack_addr, stack_size;
	stack_t signal_stack;
	struct sigaction segv_sig = {.sa_sigaction = segv_handler, .sa_flags = SA_ONSTACK|SA_SIGINFO};
	void *map;
	unsigned long gap = GAP_PAGES * page_size;
	struct rlimit rlimit;

	rlimit.rlim_cur = rlimit.rlim_max = RLIM_INFINITY;
	SAFE_SETRLIMIT(RLIMIT_STACK, &rlimit);

	preallocate_stack(REQ_STACK_SIZE);

	stack_addr = read_stack_addr_from_proc(&stack_size);
	if (stack_addr == -1UL) {
		tst_brk(TBROK, "can't read stack top from /proc/self/maps");
		return;
	}

	if (STACK_GROWSDOWN)
		mapped_addr = stack_addr - gap - MAPPED_LEN;
	else
		mapped_addr = stack_addr + gap;

	mapped_addr &= page_mask;
	map = SAFE_MMAP((void *)mapped_addr, MAPPED_LEN,
			PROT_READ|PROT_WRITE,
			MAP_ANON|MAP_PRIVATE|MAP_FIXED, -1, 0);
	tst_res(TINFO, "Stack:0x%lx+0x%lx mmap:%p+0x%lx",
		stack_addr, stack_size, map, MAPPED_LEN);

	signal_stack.ss_sp = SAFE_MALLOC(SIGNAL_STACK_SIZE);
	signal_stack.ss_size = SIGNAL_STACK_SIZE;
	signal_stack.ss_flags = 0;
	if (sigaltstack(&signal_stack, NULL) == -1) {
		tst_brk(TBROK | TERRNO, "sigaltstack");
		return;
	}
	if (sigaction(SIGSEGV, &segv_sig, NULL) == -1 ||
	    sigaction(SIGBUS,  &segv_sig, NULL) == -1) {
		tst_brk(TBROK | TERRNO, "sigaction");
		return;
	}

#ifdef DEBUG
	dump_proc_self_maps();
#endif

#ifdef __x86_64__
	do_mmap_placement_test(stack_addr, gap);
#endif

	/* Now see if it can grow too close to an adjacent region. */
	exhaust_stack_into_sigsegv();
}

void setup(void)
{
	char buf[4096], *p;

	page_size = sysconf(_SC_PAGESIZE);
	page_mask = ~(page_size - 1);

	buf[4095] = '\0';
	SAFE_FILE_SCANF("/proc/cmdline", "%4095[^\n]", buf);

	if ((p = strstr(buf, "stack_guard_gap=")) != NULL) {
		if (sscanf(p, "stack_guard_gap=%ld", &GAP_PAGES) != 1) {
			tst_brk(TBROK | TERRNO, "sscanf");
			return;
		}
		tst_res(TINFO, "stack_guard_gap = %ld", GAP_PAGES);
	}

	THRESHOLD = (GAP_PAGES - 1) * page_size;

	{
		volatile int *a = alloca(128);

		{
			volatile int *b = alloca(128);

			STACK_GROWSDOWN = a > b;
			tst_res(TINFO, "STACK_GROWSDOWN = %d == %p > %p", STACK_GROWSDOWN, a, b);
		}
	}
}

void stack_clash_test(void)
{
	int status;
	pid_t pid;

	pid = SAFE_FORK();
	if (!pid) {
		do_child();
		exit(EXIT_TESTBROKE);
		return;
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFEXITED(status)) {
		switch (WEXITSTATUS(status)) {
		case EXIT_FAILURE:
			tst_res(TFAIL, "stack is too close to the mmaped area");
			return;
		case EXIT_SUCCESS:
			tst_res(TPASS, "stack is far enough from mmaped area");
			return;
		default:
		case EXIT_TESTBROKE:
			break;
		}
	}

	tst_brk(TBROK, "Child %s", tst_strstatus(status));
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_root = 1,
	.setup = setup,
	.test_all = stack_clash_test,
	.tags = (const struct tst_tag[]) {
		{"CVE", "2017-1000364"},
		{"linux-git", "58c5d0d6d522"},
		{}
	}
};
