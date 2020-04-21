#define TST_NO_DEFAULT_MAIN

#include "tst_test.h"
#include "tst_taint.h"
#include "tst_safe_stdio.h"

#define TAINT_FILE "/proc/sys/kernel/tainted"

static unsigned int taint_mask = -1;

static unsigned int tst_taint_read(void)
{
	unsigned int val;

	SAFE_FILE_SCANF(TAINT_FILE, "%u", &val);

	return val;
}

static int tst_taint_check_kver(unsigned int mask)
{
	int r1;
	int r2;
	int r3 = 0;

	if (mask & TST_TAINT_X) {
		r1 = 4;
		r2 = 15;
	} else if (mask & TST_TAINT_K) {
		r1 = 4;
		r2 = 0;
	} else if (mask & TST_TAINT_L) {
		r1 = 3;
		r2 = 17;
	} else if (mask & TST_TAINT_E) {
		r1 = 3;
		r2 = 15;
	} else if (mask & TST_TAINT_O) {
		r1 = 3;
		r2 = 2;
	} else if (mask & TST_TAINT_I) {
		r1 = 2;
		r2 = 6;
		r3 = 35;
	} else if (mask & TST_TAINT_C) {
		r1 = 2;
		r2 = 6;
		r3 = 28;
	} else if (mask & TST_TAINT_W) {
		r1 = 2;
		r2 = 6;
		r3 = 26;
	} else if (mask & TST_TAINT_A) {
		r1 = 2;
		r2 = 6;
		r3 = 25;
	} else if (mask & TST_TAINT_D) {
		r1 = 2;
		r2 = 6;
		r3 = 23;
	} else if (mask & TST_TAINT_U) {
		r1 = 2;
		r2 = 6;
		r3 = 21;
	} else {
		r1 = 2;
		r2 = 6;
		r3 = 16;
	}

	return tst_kvercmp(r1, r2, r3);
}

void tst_taint_init(unsigned int mask)
{
	unsigned int taint = -1;

	if (mask == 0)
		tst_brk(TBROK, "mask is not allowed to be 0");

	if (tst_taint_check_kver(mask) < 0)
		tst_res(TCONF, "Kernel is too old for requested mask");

	taint_mask = mask;
	taint = tst_taint_read();

	if (taint & TST_TAINT_W) {
		tst_res(TCONF, "Ignoring already set kernel warning taint");
		taint_mask &= ~TST_TAINT_W;
	}

	if ((taint & taint_mask) != 0)
		tst_brk(TBROK, "Kernel is already tainted: %u", taint);
}


unsigned int tst_taint_check(void)
{
	unsigned int taint = -1;

	if (taint_mask == (unsigned int) -1)
		tst_brk(TBROK, "need to call tst_taint_init() first");

	taint = tst_taint_read();

	return (taint & taint_mask);
}
