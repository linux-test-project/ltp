// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) Manas Kumar Nayak <maknayak@in.ibm.com>
 * Copyright (c) Cyril Hrubis <chrubis@suse.cz> 2011
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Basic test of i386 thread-local storage for set_thread_area and
 * get_thread_area syscalls. It verifies a simple write and read of an entry
 * works.
 *
 * [Algorithm]
 *
 * - Call set_thread_area to a struct user_desc pointer with entry_number = -1,
 *   which will be set to a free entry_number upon exiting.
 * - Call get_thread_area to read the new entry.
 * - Use the new entry_number in another pointer and call get_thread_area.
 * - Make sure they have the same data.
 */

#include "tst_test.h"

#include "lapi/ldt.h"

static struct user_desc *u_info1;
static struct user_desc *u_info2;

static void run(void)
{
	TST_EXP_PASS_SILENT(set_thread_area(u_info1));
	TST_EXP_PASS_SILENT(get_thread_area(u_info1));

	u_info2->entry_number = u_info1->entry_number;
	TST_EXP_PASS_SILENT(get_thread_area(u_info2));

	TST_EXP_PASS(memcmp(u_info1, u_info2, sizeof(struct user_desc)));
}

static void setup(void)
{
	/* When set_thread_area() is passed an entry_number of -1, it  searches
	 * for a free TLS entry. If set_thread_area() finds a free TLS entry,
	 * the value of u_info->entry_number is set upon return to show which
	 * entry was changed.
	 */
	u_info1->entry_number = -1;
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.supported_archs = (const char *const[]){ "x86", NULL },
	.bufs = (struct tst_buffers[]) {
			{ &u_info1, .size = sizeof(struct user_desc) },
			{ &u_info2, .size = sizeof(struct user_desc) },
			{},
		},
};
