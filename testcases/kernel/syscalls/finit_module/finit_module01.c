// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * [Description]
 *
 * Basic finit_module() tests.
 *
 * [Algorithm]
 *
 * Inserts a simple module after opening and mmaping the module file.
 */

#include <stdlib.h>
#include <errno.h>
#include "lapi/init_module.h"
#include "tst_module.h"
#include "tst_kconfig.h"

#define MODULE_NAME	"finit_module.ko"

static int fd, sig_enforce;

static char *mod_path;

static void setup(void)
{
	struct tst_kcmdline_var params = TST_KCMDLINE_INIT("module.sig_enforce");
	struct tst_kconfig_var kconfig = TST_KCONFIG_INIT("CONFIG_MODULE_SIG_FORCE");

	tst_kcmdline_parse(&params, 1);
	tst_kconfig_read(&kconfig, 1);
	if (params.found || kconfig.choice == 'y')
		sig_enforce = 1;

	tst_module_exists(MODULE_NAME, &mod_path);

	fd = SAFE_OPEN(mod_path, O_RDONLY|O_CLOEXEC);
}

static void run(void)
{
	if (sig_enforce == 1) {
		tst_res(TINFO, "module signature is enforced");
		TST_EXP_FAIL(finit_module(fd, "status=valid", 0), EKEYREJECTED);
		return;
	}

	TST_EXP_PASS(finit_module(fd, "status=valid", 0));
	if (!TST_PASS)
		return;

	tst_module_unload(MODULE_NAME);
}

static void cleanup(void)
{
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	/* lockdown and SecureBoot requires signed modules */
	.skip_in_lockdown = 1,
	.skip_in_secureboot = 1,
};
