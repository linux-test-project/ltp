#include <errno.h>
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <linux/types.h>
#include <sys/capability.h>
#endif
#include <sys/prctl.h>
#include "test.h"

#ifndef SECBIT_KEEP_CAPS
#define SECBIT_KEEP_CAPS (1<<4)
#endif

/* Tests:
	1. drop capabilities at setuid if KEEPCAPS is not set and
	   new user is nonroot
	2. keep capabilities if set and new user is nonroot
	   a. do with prctl(PR_SET_KEEPCAPS)
	   (call this test 2)
	   b. do with prctl(PR_SET_SECUREBITS, SECURE_KEEP_CAPS)
	   (call this test 3)
   TODO: test that exec clears KEEPCAPS
   	(just create a simple executable that checks PR_GET_KEEPCAPS
	 results, and execute that as test 4 after doing PR_SET_KEEPCAPS).
   TODO: all of the other securebits tests.
 */

char *TCID = "keepcaps";
int TST_TOTAL = 1;

#if (HAVE_LINUX_SECUREBITS_H && HAVE_LIBCAP)
#include <linux/securebits.h>

static int eff_caps_empty(cap_t c)
{
	int i, ret, empty = 1;
	cap_flag_value_t v;

	for (i = 0; i < CAP_LAST_CAP; i++) {
		ret = cap_get_flag(c, i, CAP_PERMITTED, &v);
		/*
		 * If the value of CAP_LAST_CAP in linux/capability.h is greater
		 * than the value in the capability.h which is used to create
		 * libcap.so. Then cap_get_flag returns -1, and errno is set to
		 * EINVAL.
		 */
		if (ret == -1) {
			tst_brkm(TBROK | TERRNO, NULL,
				"Not expected. Please check arguments.");
		}
		if (ret || v)
			empty = 0;
	}

	return empty;
}

static int am_privileged(void)
{
	int am_privileged = 1;

	cap_t cap = cap_get_proc();
	if (eff_caps_empty(cap))
		am_privileged = 0;
	cap_free(cap);

	return am_privileged;
}

#define EXPECT_NOPRIVS 0
#define EXPECT_PRIVS 1
static void do_setuid(int expect_privs)
{
	int ret;
	int have_privs;

	ret = setuid(1000);
	if (ret)
		tst_brkm(TERRNO | TFAIL, NULL, "setuid failed");

	have_privs = am_privileged();
	if (have_privs && expect_privs == EXPECT_PRIVS) {
		tst_resm(TPASS, "kept privs as expected");
		tst_exit();
	}
	if (!have_privs && expect_privs == EXPECT_PRIVS) {
		tst_brkm(TFAIL, NULL, "expected to keep privs but did not");
	}
	if (!have_privs && expect_privs == EXPECT_NOPRIVS) {
		tst_resm(TPASS, "dropped privs as expected");
		tst_exit();
	}

	/* have_privs && EXPECT_NOPRIVS */
	tst_brkm(TFAIL, NULL, "expected to drop privs but did not");
}

int main(int argc, char *argv[])
{
	int ret, whichtest;

	tst_require_root();

	ret = prctl(PR_GET_KEEPCAPS);
	if (ret)
		tst_brkm(TBROK, NULL, "keepcaps was already set?");

	if (argc < 2)
		tst_brkm(TBROK, NULL, "Usage: %s <tescase_num>", argv[0]);

	whichtest = atoi(argv[1]);
	if (whichtest < 1 || whichtest > 3)
		tst_brkm(TFAIL, NULL, "Valid tests are 1-3");

	switch (whichtest) {
	case 1:
		do_setuid(EXPECT_NOPRIVS);	/* does not return */
	case 2:
		ret = prctl(PR_SET_KEEPCAPS, 1);
		if (ret == -1) {
			tst_brkm(TFAIL | TERRNO, NULL,
				 "PR_SET_KEEPCAPS failed");
		}
		ret = prctl(PR_GET_KEEPCAPS);
		if (!ret) {
			tst_brkm(TFAIL | TERRNO, NULL,
				 "PR_SET_KEEPCAPS did not set keepcaps");
		}
		do_setuid(EXPECT_PRIVS);	/* does not return */
	case 3:
		ret = prctl(PR_GET_SECUREBITS);
		ret = prctl(PR_SET_SECUREBITS, ret | SECBIT_KEEP_CAPS);
		if (ret == -1) {
			tst_brkm(TFAIL | TERRNO, NULL,
				 "PR_SET_SECUREBITS failed");
		}
		ret = prctl(PR_GET_KEEPCAPS);
		if (!ret) {
			tst_brkm(TFAIL | TERRNO, NULL,
				 "PR_SET_SECUREBITS did not set keepcaps");
		}
		do_setuid(EXPECT_PRIVS);	/* does not return */
	default:
		tst_brkm(TFAIL, NULL, "Valid tests are 1-3");
	}
}

#else

int main(void)
{
	tst_brkm(TCONF, NULL, "linux/securebits.h or libcap does not exist.");
}

#endif /* HAVE_LIBCAP */
