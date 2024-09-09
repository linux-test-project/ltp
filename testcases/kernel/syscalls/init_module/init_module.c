// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Dummy test module.
 *
 * The module accepts a single argument named "status" and it fails
 * initialization if the status is set to "invalid".
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>

#define DIRNAME "dummy_init"

static char status[20];
module_param_string(status, status, 20, 0444);

static int dummy_init(void)
{
	struct proc_dir_entry *proc_dummy;

	if (!strcmp(status, "invalid"))
		return -EINVAL;

	proc_dummy = proc_mkdir(DIRNAME, 0);
	return 0;
}
module_init(dummy_init);

static void dummy_exit(void)
{
	remove_proc_entry(DIRNAME, 0);
}
module_exit(dummy_exit);

MODULE_LICENSE("GPL");
