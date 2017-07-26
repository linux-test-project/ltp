/*
 * Copyright (c) 2017 Pavel Boldin <pboldin@cloudlinux.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Adapted from code by Michal Hocko.
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
 * [1] https://blog.qualys.com/securitylabs/2017/06/19/the-stack-clash
 * [2] https://bugzilla.novell.com/show_bug.cgi?id=CVE-2017-1000364
 */

#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <alloca.h>
#include <signal.h>
#include <stdlib.h>

#include "tst_test.h"
#include "tst_safe_stdio.h"

static unsigned long PAGE_SIZE;
static unsigned long PAGE_MASK;
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

#define MAPPED_LEN PAGE_SIZE
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
		"mmap = [%lx, %lx), addr = %lx, diff = %lx, THRESHOLD = %lx",
		mapped_addr, mmap_end, fault_addr, diff, THRESHOLD);
	if (diff < 0 || (unsigned long)diff < THRESHOLD)
		_exit(EXIT_FAILURE);
	else
		_exit(EXIT_SUCCESS);
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
	tst_run_cmd(cmd, NULL, NULL, 0);
}

void preallocate_stack(unsigned long required)
{
	volatile char *garbage;

	garbage = alloca(required);
	garbage[0] = garbage[required - 1] = '\0';
}

void do_child(void)
{
	unsigned long stack_addr, stack_size;
	stack_t signal_stack;
	struct sigaction segv_sig = {.sa_sigaction = segv_handler, .sa_flags = SA_ONSTACK|SA_SIGINFO};
	void *map;
	unsigned long gap = GAP_PAGES * PAGE_SIZE;
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

	mapped_addr &= PAGE_MASK;
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

	exhaust_stack_into_sigsegv();
}

void setup(void)
{
	char buf[4096], *p;

	PAGE_SIZE = sysconf(_SC_PAGESIZE);
	PAGE_MASK = ~(PAGE_SIZE - 1);

	buf[4095] = '\0';
	SAFE_FILE_SCANF("/proc/cmdline", "%4095[^\n]", buf);

	if ((p = strstr(buf, "stack_guard_gap=")) != NULL) {
		if (sscanf(p, "stack_guard_gap=%ld", &GAP_PAGES) != 1) {
			tst_brk(TBROK | TERRNO, "sscanf");
			return;
		}
		tst_res(TINFO, "stack_guard_gap = %ld", GAP_PAGES);
	}

	THRESHOLD = (GAP_PAGES - 1) * PAGE_SIZE;

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

	tst_brk(TBROK, "child did not exit gracefully");
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_root = 1,
	.setup = setup,
	.test_all = stack_clash_test,
};
