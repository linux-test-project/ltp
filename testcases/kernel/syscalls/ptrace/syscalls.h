const struct sysnums {
	long nr;
	const char *snr;
} sysnums[] = {
#define P(NR) { .nr = SYS_##NR, .snr = #NR, },
#include "_syscalls.h"
#undef P
};

const char *get_sysnum(long nr)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(sysnums); ++i)
		if (sysnums[i].nr == nr)
			break;
	return i == ARRAY_SIZE(sysnums) ? "???" : sysnums[i].snr;
}
