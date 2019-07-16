// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 *
 * Test the PR_CAP_AMBIENT of prctl(2).
 * Reads or changes the ambient capability set of the calling thread,
 * according to the value of arg2, which must be one of the following:
 * 1)PR_CAP_AMBIENT_RAISE:
 * The capability specified in arg3 is added to the ambient set.
 * The specified capability must already be present in both pE and pI.
 * If we set SECBIT_NO_CAP_AMBIENT_RAISE bit, raise option will be rejected
 * and retrun EPERM. We also raise a CAP twice.
 * 2)PR_CAP_AMBIENT_LOWER:
 * The capability specified in arg3 is removed from the ambient set.
 * Even though this cap is not in set, it also should return 0.
 * 3)PR_CAP_AMBIENT_IS_SET:
 * Returns 1 if the capability in arg3 is in the ambient set and 0 if it
 * is not.
 * 4)PR_CAP_AMBIENT_CLEAR_ALL:
 * All capabilities will be removed from the ambient set. This operation
 * requires setting arg3 to zero.
 */

#include <sys/prctl.h>
#include <stdlib.h>
#include "config.h"
#ifdef HAVE_SYS_CAPABILITY_H
# include <sys/capability.h>
#endif
#include "lapi/syscalls.h"
#include "lapi/prctl.h"
#include "lapi/securebits.h"
#include "tst_test.h"

#define PROC_STATUS "/proc/self/status"

#ifdef HAVE_SYS_CAPABILITY_H
static void check_proc_capamb(char *message, int flag)
{
	int cap_num;
	char CapAmb[20];

	SAFE_FILE_LINES_SCANF(PROC_STATUS, "CapAmb:%s", CapAmb);
	cap_num = strtol(CapAmb, NULL, 16);
	if (flag == 2) {
		if (cap_num == 0)
			tst_res(TPASS,
				"%s, %s CapAmb has been clear as %d",
				message, PROC_STATUS, cap_num);
		else
			tst_res(TFAIL,
				"%s, %s CapAmb has been clear expect 0, got %d",
				message, PROC_STATUS, cap_num);
		return;
	}
	if (cap_num == (1 << CAP_NET_BIND_SERVICE))
		tst_res(flag ? TPASS : TFAIL,
			"%s, CapAmb in %s has CAP_NET_BIND_SERVICE",
			message, PROC_STATUS);
	else
		tst_res(flag ? TFAIL : TPASS,
			"%s, CapAmb in %s doesn't have CAP_NET_BIND_SERVICE",
			message, PROC_STATUS);
}
#endif

static inline void check_cap_raise(unsigned int cap, char *message, int fail_flag)
{
	TEST(prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0, 0));
	switch (fail_flag) {
	case 0:
	if (TST_RET == 0)
		tst_res(TPASS, "PR_CAP_AMBIENT_RAISE %s succeeded", message);
	else
		tst_res(TFAIL, "PR_CAP_AMBIENT_RAISE %s failed unexpectedly",
			message);
	break;
	case 1:
	if (TST_RET == 0)
		tst_res(TFAIL,
			"PR_CAP_AMBIENT_RAISE succeeded unexpectedly %s",
			message);
	else if (TST_ERR == EPERM)
		tst_res(TPASS,
			"PR_CAP_AMBIENT_RAISE failed with EPERM %s", message);
	else
		tst_res(TFAIL | TTERRNO,
			"PR_CAP_AMBIENT_RAISE failed %s", message);
	break;
	}
}

static inline void check_cap_is_set(unsigned int cap, char *message, int val)
{
	TEST(prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_IS_SET, cap, 0, 0, 0));
	if (TST_RET == 1)
		tst_res(val ? TPASS : TFAIL,
			"PR_CAP_AMBIENT_IS_SET %s in AmbientCap", message);
	else if (TST_RET == 0)
		tst_res(val ? TFAIL : TPASS,
			"PR_CAP_AMBIENT_IS_SET %s not in AmbientCap", message);
	else
		tst_res(TFAIL | TTERRNO, "PR_CAP_AMBIENT_IS_SET failed");
}

static inline void check_cap_lower(unsigned int cap, char *message)
{
	TEST(prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER, cap, 0, 0, 0));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO,
			"PR_CAP_AMBIENT_LOWER %s failed", message);
	else
		tst_res(TPASS, "PR_CAP_AMBIENT_LOWER %s succeeded", message);
}

static void verify_prctl(void)
{
#ifdef HAVE_LIBCAP
	cap_t caps = cap_init();

	cap_value_t caplist[3] = {CAP_NET_RAW, CAP_NET_BIND_SERVICE, CAP_SETPCAP};
	unsigned int numcaps = 3;

	cap_set_flag(caps, CAP_EFFECTIVE, numcaps, caplist, CAP_SET);
	cap_set_flag(caps, CAP_INHERITABLE, numcaps, caplist, CAP_SET);
	cap_set_flag(caps, CAP_PERMITTED, numcaps, caplist, CAP_SET);
	cap_set_proc(caps);

	check_proc_capamb("At the beginning", 0);

	cap_clear_flag(caps, CAP_INHERITABLE);
	cap_set_proc(caps);
	check_cap_raise(CAP_NET_BIND_SERVICE, "on non-inheritable cap", 1);

	cap_set_flag(caps, CAP_INHERITABLE, numcaps, caplist, CAP_SET);
	cap_clear_flag(caps, CAP_PERMITTED);
	cap_set_proc(caps);
	check_cap_raise(CAP_NET_RAW, "on non-permitted cap", 1);

	cap_set_flag(caps, CAP_PERMITTED, numcaps, caplist, CAP_SET);
	cap_set_proc(caps);
	prctl(PR_SET_SECUREBITS, SECBIT_NO_CAP_AMBIENT_RAISE);
	check_cap_raise(CAP_NET_BIND_SERVICE, "because of NO_RAISE_SECBIT set", 1);
	prctl(PR_SET_SECUREBITS, 0);

	check_cap_raise(CAP_NET_BIND_SERVICE, "CAP_NET_BIND_SERVICE", 0);
	/*Even this cap has been in ambient set, raise succeeds and return 0*/
	check_cap_raise(CAP_NET_BIND_SERVICE, "CAP_NET_BIND_SERIVCE twice", 0);

	check_proc_capamb("After PR_CAP_AMBIENT_RAISE", 1);

	check_cap_is_set(CAP_NET_BIND_SERVICE, "CAP_NET_BIND_SERVICE was", 1);
	check_cap_is_set(CAP_NET_RAW, "CAP_NET_RAW was", 0);
	/*move a cap what was not in ambient set, it also return 0*/
	check_cap_lower(CAP_NET_RAW, "CAP_NET_RAW(it wasn't in ambient set)");
	check_cap_lower(CAP_NET_BIND_SERVICE, "CAP_NET_BIND_SERVICE(it was in ambient set)");
	check_proc_capamb("After PR_CAP_AMBIENT_LORWER", 0);

	prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, CAP_NET_BIND_SERVICE, 0, 0, 0);
	tst_res(TINFO, "raise cap for clear");
	TEST(prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0, 0));
	if (TST_RET == 0)
		tst_res(TPASS, "PR_CAP_AMBIENT_CLEAR ALL succeeded");
	else
		tst_res(TFAIL | TERRNO, "PR_AMBIENT_CLEAR_ALL failed");

	check_proc_capamb("After PR_CAP_AMBIENT_CLEAN_ALL", 2);

	cap_free(caps);
#else
	tst_res(TCONF, "libcap devel files missing during compilation");
#endif
}

static void setup(void)
{
	TEST(prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0, 0));
	if (TST_RET == 0) {
		tst_res(TINFO, "kernel supports PR_CAP_AMBIENT");
		return;
	}

	if (TST_ERR == EINVAL)
		tst_brk(TCONF, "kernel doesn't support PR_CAP_AMBIENT");

	tst_brk(TBROK | TERRNO,
		"current environment doesn't permit PR_CAP_AMBIENT");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_prctl,
	.needs_root = 1,
};
