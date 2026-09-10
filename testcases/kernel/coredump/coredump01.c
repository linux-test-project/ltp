// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Linux Test Project
 */

/*\
 * Verify that the kernel expands the :manpage:`core(5)` specifiers %e
 * (executable name), %p (PID) and %s (signal number) when it creates a
 * core dump.
 *
 * Both flavors of ``/proc/sys/kernel/core_pattern`` are tested:
 *
 * - a plain file name, where the specifiers become part of the core file
 *   name
 * - a pipe to a user space helper, where the specifiers are expanded in
 *   the helper arguments and the core dump is written to the helper
 *   standard input
 *
 * The test needs root because it rewrites the system wide core_pattern.
 * The original value is saved and restored by the test library on all
 * exit paths.
 *
 * [Algorithm]
 *
 * - Point core_pattern into the test temporary directory
 * - Fork a child which aborts itself to produce a core dump
 * - For the file pattern check that ``core.<exe>.<pid>.<signal>`` was created
 * - For the pipe pattern check the arguments the helper was called with
 *   and that it received an ELF core dump on its standard input
 */

#include "coredump_common.h"
#include "tst_kconfig.h"

#define HELPER "coredump01_helper"
#define HELPER_TIMEOUT 10

static char helper_path[PATH_MAX];
static int static_usermodehelper;

/*
 * e_ident[] and e_type live at the same file offset in ELF32 and ELF64,
 * so we can validate both without picking a class.
 */
static void verify_elf_core(const char *path)
{
	unsigned char hdr[EI_NIDENT + sizeof(Elf32_Half)];
	Elf32_Half e_type;
	int fd;

	fd = SAFE_OPEN(path, O_RDONLY);
	SAFE_READ(1, fd, hdr, sizeof(hdr));
	SAFE_CLOSE(fd);

	memcpy(&e_type, hdr + EI_NIDENT, sizeof(e_type));

	TST_EXP_EXPR(!memcmp(hdr, ELFMAG, SELFMAG), "%s starts with ELF magic", path);
	TST_EXP_EQ_LI(e_type, ET_CORE);
}

static void verify_file_pattern(void)
{
	char dump[PATH_MAX + 32];
	pid_t pid;

	set_pattern("%s/core.%%e.%%p.%%s", cwd);

	pid = crash_child();

	snprintf(dump, sizeof(dump), "%s/core.coredump01.%d.%d", cwd, pid, SIGABRT);

	TST_EXP_PASS(access(dump, F_OK), "core.%%e.%%p.%%s expanded to core.coredump01.%d.%d",
		     pid, SIGABRT);
	if (!TST_PASS)
		return;

	verify_elf_core(dump);
}

static void verify_pipe_pattern(void)
{
	char res[PATH_MAX + 32], exe[PATH_MAX];
	int pid_seen, sig_seen, elf, et_core;
	long long bytes;
	pid_t pid;

	if (static_usermodehelper) {
		tst_res(TCONF, "CONFIG_STATIC_USERMODEHELPER is enabled, skipping pipe core_pattern");
		return;
	}

	set_pattern("|%s %%e %%p %%s %s/res.%%p", helper_path, cwd);

	pid = crash_child();

	snprintf(res, sizeof(res), "%s/res.%d", cwd, pid);

	/* the kernel spawns the helper asynchronously */
	if (TST_RETRY_FN_EXP_BACKOFF(access(res, F_OK), TST_RETVAL_EQ0, HELPER_TIMEOUT)) {
		tst_res(TFAIL, "%s did not report any core dump", HELPER);
		return;
	}

	SAFE_FILE_SCANF(res, "exe=%15s pid=%d sig=%d bytes=%lld elf=%d et_core=%d",
			exe, &pid_seen, &sig_seen, &bytes, &elf, &et_core);

	TST_EXP_EQ_STR(exe, "coredump01");
	TST_EXP_EQ_LI(pid_seen, pid);
	TST_EXP_EQ_LI(sig_seen, SIGABRT);
	TST_EXP_EXPR(elf && bytes > 0, "%s read %lli bytes of ELF core dump", HELPER, bytes);
	TST_EXP_EQ_LI(et_core, 1);
}

static struct tcase {
	void (*verify_fn)(void);
	const char *desc;
} tcases[] = {
	{verify_file_pattern, "file core_pattern"},
	{verify_pipe_pattern, "pipe core_pattern"},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "Testing %s", tc->desc);
	tc->verify_fn();
}

static void setup(void)
{
	char path[PATH_MAX];
	struct tst_kconfig_var kconfig = TST_KCONFIG_INIT("CONFIG_STATIC_USERMODEHELPER");

	coredump_setup();

	tst_kconfig_read(&kconfig, 1);
	static_usermodehelper = (kconfig.choice == 'y');

	if (tst_get_path(HELPER, path, sizeof(path)))
		tst_brk(TCONF, "'%s' not found in $PATH", HELPER);

	if (!realpath(path, helper_path))
		tst_brk(TBROK | TERRNO, "realpath(%s) failed", path);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char* []) {
		"CONFIG_COREDUMP=y",
		NULL,
	},
	.save_restore = (const struct tst_path_val[]) {
		{PATH_KERN_CORE_PATTERN, NULL, TST_SR_TCONF},
		{}
	},
};
