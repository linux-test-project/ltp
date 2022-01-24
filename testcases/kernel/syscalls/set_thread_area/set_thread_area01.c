/*************************************************************************
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) Manas Kumar Nayak <maknayak@in.ibm.com>
 * Copyright (c) Cyril Hrubis <chrubis@suse.cz> 2011
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 ************************************************************************/

#include "set_thread_area.h"

char *TCID = "set_thread_area_01";
int TST_TOTAL = 6;

#if defined(HAVE_ASM_LDT_H) && defined(HAVE_STRUCT_USER_DESC)

static void cleanup(void)
{
}

static void setup(void)
{
	TEST_PAUSE;
}

struct test {
	int syscall;
	const char *const syscall_name;
	thread_area_s *u_info;
	int exp_ret;
	int exp_errno;
};

/*
 * The set_thread_area uses a free entry_number if entry number is set to -1
 * and upon the syscall exit the entry number is set to entry which was used.
 * So when we call get_thread_area on u_info1, the entry number is initalized
 * corectly by the previous set_thread_area.
 */
static struct user_desc u_info1 = {.entry_number = -1 };
static struct user_desc u_info2 = {.entry_number = -2 };

#define VALUE_AND_STRING(val) val, #val

static struct test tests[] = {
	{VALUE_AND_STRING(__NR_set_thread_area), &u_info1, 0, 0},
	{VALUE_AND_STRING(__NR_get_thread_area), &u_info1, 0, 0},
	{VALUE_AND_STRING(__NR_set_thread_area), &u_info2, -1, EINVAL},
	{VALUE_AND_STRING(__NR_get_thread_area), &u_info2, -1, EINVAL},
	{VALUE_AND_STRING(__NR_set_thread_area), (void *)-9, -1, EFAULT},
	{VALUE_AND_STRING(__NR_get_thread_area), (void *)-9, -1, EFAULT},
};

int main(int argc, char *argv[])
{
	int lc;
	unsigned i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		for (i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
			TEST(tst_syscall(tests[i].syscall, tests[i].u_info));

			if (TEST_RETURN != tests[i].exp_ret) {
				tst_resm(TFAIL, "%s returned %li expected %i",
					 tests[i].syscall_name,
					 TEST_RETURN, tests[i].exp_ret);
				continue;
			}

			if (TEST_ERRNO != tests[i].exp_errno) {
				tst_resm(TFAIL,
					 "%s failed with %i (%s) expected %i (%s)",
					 tests[i].syscall_name, TEST_ERRNO,
					 strerror(TEST_ERRNO),
					 tests[i].exp_errno,
					 strerror(tests[i].exp_errno));
				continue;
			}

			tst_resm(TPASS, "%s returned %li errno %i (%s)",
				 tests[i].syscall_name, TEST_RETURN,
				 TEST_ERRNO, strerror(TEST_ERRNO));
		}
	}

	cleanup();
	tst_exit();
}
#else
int main(void)
{
	tst_brkm(TCONF, NULL,
		 "set_thread_area isn't available for this architecture");
}
#endif
