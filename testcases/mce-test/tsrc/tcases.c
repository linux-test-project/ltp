/* 
 * Verify MCA grading engine against some examples.
 */
#include <sys/types.h>
#include <stdio.h>
#define __KERNEL__ 1
#include <asm/types.h>
#include <asm/mce.h>
#include <errno.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))

typedef unsigned long long u64;

#define MCI_STATUS_S	 (1ULL<<56)  /* Signaled machine check */
#define MCI_STATUS_AR	 (1ULL<<55)  /* Action required */

int mce_ser = 1;
int tolerant = 1;
int panic_on_oops = 0;

#include "mce-severity.c"

char *resname[] = {
#define R(x) [MCE_ ## x ## _SEVERITY] = #x
	R(NO),
	R(KEEP),
	R(SOME),
	R(AO),
	R(AR),
	R(PANIC),
};
#define VAL MCI_STATUS_VAL
#define EN MCI_STATUS_EN
#define PCC MCI_STATUS_PCC
#define S MCI_STATUS_S
#define AR MCI_STATUS_AR
#define UC MCI_STATUS_UC

int ring = 3;
int fail;

void test2(u64 flag, char *flagname, u64 mcg, char *mcgname, int result)
{
	struct mce m = {
		.ip = 1,
		.cs = ring,
		.status = flag,
		.mcgstatus = mcg,
	};
	int r;
	char *msg;

	if ((r = mce_severity(&m, tolerant, &msg)) != result) { 
		printf("%s %s expected %s got %s msg %s\n",
		       flagname, mcgname, resname[result], resname[r], msg);	
		fail++;
	}
}


#define TEST(flag, result) \
	test2(flag, #flag, MCG_STATUS_MCIP|MCG_STATUS_RIPV, "mcip,ripv", \
		MCE_ ## result ## _SEVERITY)

void test(void)
{
	// corrected
	TEST(VAL|EN, KEEP);

	// uncorrected fatal
	TEST(VAL|UC|PCC|EN|S|AR, PANIC);
	TEST(VAL|UC|PCC|EN|S, PANIC);
	TEST(VAL|UC|PCC|EN, PANIC);

	// SW recoverable action required
	// unknown mcacod -> panic
	TEST(VAL|UC|EN|S|AR, PANIC);

	// SW recoverable action optional
	TEST(VAL|UC|EN|S|0xc0, AO);
	// unknown mcacod
	TEST(VAL|UC|EN|S|1, SOME);

	// UCNA
	TEST(VAL|UC|EN, KEEP);
	TEST(VAL|UC, NO);	// linux clears. correct?	
}

int main(void)
{
	ring = 3;
	test();
	ring = 0;
	test();
	if (fail == 0)
		printf("SUCCESS\n");
	else
		printf("%d FAILURES\n", fail);
	return fail;
}
