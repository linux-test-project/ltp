// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * [Description]
 *
 * Test that address space layout randomization (ASLR) is sufficiently random.
 * A bug in dynamic library mmapping may reduce ASLR randomness if the library
 * file is larger than hugepage size. In 32bit compat mode, this may
 * completely disable ASLR and force large dynamic libraries to be loaded
 * at fixed addresses.
 *
 * The issue may not be reproducible if hugepage support is missing or no
 * sufficiently large library is loaded into the test program. If libc is not
 * large enough, you may use `export LD_PRELOAD=...` to load another
 * sufficiently large library. The export keyword is required because
 * the checks are done on a subprocess.
 *
 * In normal mode, the test checks that library base address has a minimum
 * number of random bits (configurable using the -b option). In strict mode,
 * the test checks that library base address is aligned to regular pagesize
 * (not hugepage) and the number of random bits is at least
 * CONFIG_ARCH_MMAP_RND_BITS_MIN or the compat equivalent. The -b option is
 * ignored.
 */

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "tst_kernel.h"
#include "tst_kconfig.h"
#include "tst_safe_stdio.h"

/* Indices for aslr_kconfigs[] below */
#define ASLR_HAVE_COMPAT 0
#define ASLR_MINBITS 1
#define ASLR_COMPAT_MINBITS 2

static int pagebits, minbits = 8;
static char *minbits_str, *strict_check;
static char lib_path[PATH_MAX];
static FILE *ldd;

static struct tst_kconfig_var aslr_kconfigs[] = {
	TST_KCONFIG_INIT("CONFIG_HAVE_ARCH_MMAP_RND_COMPAT_BITS"),
	TST_KCONFIG_INIT("CONFIG_ARCH_MMAP_RND_BITS_MIN"),
	TST_KCONFIG_INIT("CONFIG_ARCH_MMAP_RND_COMPAT_BITS_MIN"),
};

static int count_align_bits(size_t val)
{
	int ret = 0;

	for (; val && !(val & 0x1); val >>= 1, ret++)
		;

	return ret;
}

static int count_bits(size_t val)
{
	int ret = 0;

	for (; val; val >>= 1) {
		if (val & 1)
			ret++;
	}

	return ret;
}

/* Extract library path and base address from a line of ldd output. */
static int parse_mapping(const char *line, char *path, uint64_t *addr)
{
	int ret;

	line = strchr(line, '/');

	if (!line)
		return 0;

	ret = sscanf(line, "%s (0x%" PRIx64 ")", path, addr);
	return ret >= 2;
}

/*
 * Run ldd on the test executable and pass each library/address to callback.
 * If the callback returns non-zero, the reader loop will immediately exit.
 */
static void read_shared_libraries(int (*callback)(void*, const char*, uint64_t),
	void *arg)
{
	char line[PATH_MAX], path[PATH_MAX];
	uint64_t addr;
	int ret;

	sprintf(path, "ldd /proc/%d/exe", getpid());
	ldd = SAFE_POPEN(path, "r");

	while (fgets(line, PATH_MAX, ldd)) {
		if (*line && !feof(ldd) && line[strlen(line) - 1] != '\n')
			tst_brk(TBROK, "Dynamic library entry too long");

		if (!parse_mapping(line, path, &addr))
			continue;

		if (callback(arg, path, addr))
			break;
	}

	while (fgets(line, PATH_MAX, ldd))
		;

	ret = pclose(ldd);
	ldd = NULL;

	if (!WIFEXITED(ret) || WEXITSTATUS(ret))
		tst_brk(TBROK, "Reading dynamic libraries failed");
}

static int find_large_lib_callback(void *arg, const char *path,
	uint64_t addr LTP_ATTRIBUTE_UNUSED)
{
	size_t *libsize = arg;
	struct stat statbuf;

	SAFE_STAT(path, &statbuf);

	if (*libsize < (size_t)statbuf.st_size) {
		strcpy(lib_path, path);
		*libsize = statbuf.st_size;
	}

	return 0;
}

static void find_large_lib(void)
{
	size_t hpsize, libsize = 0;

	read_shared_libraries(find_large_lib_callback, &libsize);

	if (!libsize) {
		tst_brk(TCONF,
			"No dynamic libraries loaded, please use LD_PRELOAD");
	}

	hpsize = tst_get_hugepage_size();
	tst_res(TINFO, "Largest loaded library: %s (%zu bytes)", lib_path,
		libsize);

	if (!hpsize) {
		tst_res(TCONF, "Hugepage support appears to be missing");
	} else if (libsize < hpsize) {
		tst_res(TCONF, "The largest dynamic library is smaller than hugepage size, "
				"please use LD_PRELOAD to add larger library");
	}
}

static int get_lib_address_callback(void *arg, const char *path, uint64_t addr)
{
	uint64_t *out_addr = arg;

	if (!strcmp(path, lib_path)) {
		*out_addr = addr;
		return 1;
	}

	return 0;
}

static void setup(void)
{
	int compat = tst_is_compat_mode();
	const char *kconf_minbits, *minbits_path;

	pagebits = count_align_bits(getpagesize());
	tst_kconfig_read(aslr_kconfigs, ARRAY_SIZE(aslr_kconfigs));

	if (compat && aslr_kconfigs[ASLR_HAVE_COMPAT].choice != 'y')
		tst_brk(TCONF, "ASLR not supported in compat mode");

	if (!strict_check && tst_parse_int(minbits_str, &minbits, 1, 64))
		tst_brk(TBROK, "Invalid bit count argument '%s'", minbits_str);

	if (strict_check) {
		if (compat) {
			kconf_minbits = aslr_kconfigs[ASLR_COMPAT_MINBITS].val;
			minbits_path = "/proc/sys/vm/mmap_rnd_compat_bits";
		} else {
			kconf_minbits = aslr_kconfigs[ASLR_MINBITS].val;
			minbits_path = "/proc/sys/vm/mmap_rnd_bits";
		}

		/*
		 * Reading mmap_rnd_bits usually requires root privileges.
		 * Fall back to kernel config values if unprivileged.
		 */
		if (!access(minbits_path, R_OK))
			SAFE_FILE_SCANF(minbits_path, "%d", &minbits);
		else if (!kconf_minbits)
			tst_brk(TBROK, "Cannot determine kernel ASLR min bits");
		else if (tst_parse_int(kconf_minbits, &minbits, 1, 64))
			tst_brk(TBROK, "Invalid kernel ASLR min bits value");
	}

	find_large_lib();
}

static void run(void)
{
	uint64_t rndbits = 0, fixbits, addr;
	int rndcount, aligncount, i;

	fixbits = ~rndbits;

	for (i = 0; i < 512; i++) {
		addr = 0;
		read_shared_libraries(get_lib_address_callback, &addr);

		if (!addr) {
			tst_res(TWARN, "Library %s not found?!", lib_path);
			continue;
		}

		rndbits |= addr;
		fixbits &= addr;
	}

	rndcount = count_bits(rndbits & ~fixbits);
	aligncount = count_align_bits(rndbits);

	if (rndcount < minbits) {
		tst_res(TFAIL,
			"Large lib base address has less than %d random bits",
			minbits);
		return;
	}

	tst_res(TPASS, "Library address has %d random bits", rndcount);
	tst_res(TINFO, "Library base address alignment: %d bits", aligncount);

	if (aligncount > pagebits) {
		tst_res(strict_check ? TFAIL : TINFO,
			"Base address alignment is higher than expected (%d)",
			pagebits);
	}
}

static void cleanup(void)
{
	if (ldd) {
		char buf[PATH_MAX];

		while (fgets(buf, PATH_MAX, ldd))
			;

		pclose(ldd);
	}
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.options = (struct tst_option []) {
		{"b:", &minbits_str, "Minimum ASLR random bits (default: 8)"},
		{"s", &strict_check, "Run in strict mode"},
		{}
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_HAVE_ARCH_MMAP_RND_BITS=y",
		NULL
	},
	.needs_cmds = (const char *[]) {
		"ldd",
		NULL
	},
};
