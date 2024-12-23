// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * [Description]
 *
 * Basic init_module() failure tests.
 *
 * [Algorithm]
 *
 * Tests various failure scenarios for init_module().
 */

#include <linux/capability.h>
#include <stdlib.h>
#include <errno.h>
#include "lapi/init_module.h"
#include "tst_kconfig.h"
#include "tst_module.h"
#include "tst_capability.h"

#define MODULE_NAME	"init_module.ko"

static unsigned long size, zero_size;
static int kernel_lockdown, secure_boot, sig_enforce;
static void *buf, *faulty_buf, *null_buf;

static struct tst_cap cap_req = TST_CAP(TST_CAP_REQ, CAP_SYS_MODULE);
static struct tst_cap cap_drop = TST_CAP(TST_CAP_DROP, CAP_SYS_MODULE);

static struct tcase {
	const char *name;
	void **buf;
	unsigned long *size;
	const char *param;
	int cap;
	int skip_in_lockdown;
	int exp_errno;
} tcases[] = {
	{"NULL-buffer", &null_buf, &size, "", 0, 0, EFAULT},
	{"faulty-buffer", &faulty_buf, &size, "", 0, 0, EFAULT},
	{"null-param", &buf, &size, NULL, 0, 1, EFAULT},
	{"zero-size", &buf, &zero_size, "", 0, 0, ENOEXEC},
	{"invalid_param", &buf, &size, "status=invalid", 0, 1, EINVAL},
	{"no-perm", &buf, &size, "", 1, 0, EPERM},
	{"module-exists", &buf, &size, "", 0, 1, EEXIST},
	{"module-unsigned", &buf, &size, "", 0, 1, EKEYREJECTED},
};

static void setup(void)
{
	struct stat sb;
	int fd;
	struct tst_kcmdline_var params = TST_KCMDLINE_INIT("module.sig_enforce");
	struct tst_kconfig_var kconfig = TST_KCONFIG_INIT("CONFIG_MODULE_SIG_FORCE");

	tst_kcmdline_parse(&params, 1);
	tst_kconfig_read(&kconfig, 1);
	if (params.found || kconfig.choice == 'y')
		sig_enforce = 1;

	tst_module_exists(MODULE_NAME, NULL);

	kernel_lockdown = tst_lockdown_enabled() > 0;
	secure_boot = tst_secureboot_enabled() > 0;
	fd = SAFE_OPEN(MODULE_NAME, O_RDONLY|O_CLOEXEC);
	SAFE_FSTAT(fd, &sb);
	size = sb.st_size;
	buf = SAFE_MMAP(0, size, PROT_READ|PROT_EXEC, MAP_PRIVATE, fd, 0);
	SAFE_CLOSE(fd);

	faulty_buf = tst_get_bad_addr(NULL);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (tc->skip_in_lockdown && (kernel_lockdown || secure_boot)) {
		tst_res(TCONF, "Cannot load unsigned modules, skipping %s", tc->name);
		return;
	}

	if ((sig_enforce == 1) && (tc->exp_errno != EKEYREJECTED)) {
		tst_res(TCONF, "module signature is enforced, skipping %s", tc->name);
		return;
	}

	if ((sig_enforce != 1) && (tc->exp_errno == EKEYREJECTED)) {
		tst_res(TCONF, "module signature is not enforced, skipping %s", tc->name);
		return;
	}

	if (tc->cap)
		tst_cap_action(&cap_drop);

	/* Insert module twice */
	if (tc->exp_errno == EEXIST)
		tst_module_load(MODULE_NAME, NULL);

	TST_EXP_FAIL(init_module(*tc->buf, *tc->size, tc->param),
		tc->exp_errno, "TestName: %s", tc->name);

	if (tc->exp_errno == EEXIST)
		tst_module_unload(MODULE_NAME);

	if (!TST_PASS && !TST_RET)
		tst_module_unload(MODULE_NAME);

	if (tc->cap)
		tst_cap_action(&cap_req);
}

static void cleanup(void)
{
	munmap(buf, size);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
};
