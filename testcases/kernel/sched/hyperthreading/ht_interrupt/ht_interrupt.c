/***************************************************************************
                          HTinterrupt.c  -  description
                             -------------------
    email                : sonic,zhang@intel.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ht_utils.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

char *TCID = "ht_interrupt";
int TST_TOTAL = 1;

#define INTERRUPT_NAME	"/proc/interrupts"

int HT_InterruptDistribution()
{
	FILE *pFile;
	int ci[32], cj[32];
	int cpucount, i;
	int cmax, cmin, d;

	tst_resm(TINFO, "Get interrupts distribution with HT.");

	if ((cpucount = get_cpu_count()) <= 0) {
		return 0;
	}

	if ((pFile = fopen(INTERRUPT_NAME, "r")) == NULL) {
		return 0;
	}

	fgets(buf, 255, pFile);
	fscanf(pFile, "%s %d %d %d %d %d %d %d %d \
%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d \
%d %d %d %d %d %d %d %d", buf, ci, ci + 1, ci + 2, ci + 3, ci + 4, ci + 5, ci + 6, ci + 7, ci + 8, ci + 9, ci + 10, ci + 11, ci + 12, ci + 13, ci + 14, ci + 15, ci + 16, ci + 17, ci + 18, ci + 19, ci + 20, ci + 21, ci + 22, ci + 23, ci + 24, ci + 25, ci + 26, ci + 27, ci + 28, ci + 29, ci + 30, ci + 31);

	fclose(pFile);

	for (i = 0; i < 10; i++) {
		sleep(1);
		printf(".");
	}

	if ((pFile = fopen(INTERRUPT_NAME, "r")) == NULL) {
		return 0;
	}

	fgets(buf, 255, pFile);
	fscanf(pFile, "%s %d %d %d %d %d %d %d %d \
%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d \
%d %d %d %d %d %d %d %d", buf, cj, cj + 1, cj + 2, cj + 3, cj + 4, cj + 5, cj + 6, cj + 7, cj + 8, cj + 9, cj + 10, cj + 11, cj + 12, cj + 13, cj + 14, cj + 15, cj + 16, cj + 17, cj + 18, cj + 19, cj + 20, cj + 21, cj + 22, cj + 23, cj + 24, cj + 25, cj + 26, cj + 27, cj + 28, cj + 29, cj + 30, cj + 31);

	fclose(pFile);

	printf("\n\n");
	printf("Timer interrupt counts per CPU:\n");
	d = cj[0] - ci[0];
	printf("%d ", d);
	cmax = cmin = d;
	for (i = 1; i < cpucount; i++) {
		d = cj[i] - ci[i];
		printf("%d ", d);
		if (cmax < d)
			cmax = d;
		if (cmin > d)
			cmin = d;
	}

	printf("\n\n");
	printf("max value: %d\n", cmax);
	printf("min value: %d\n", cmin);
	printf("\n");

	if (cmin == 0 || cmax / cmin > 10) {
		return 0;
	} else {
		return 1;
	}
}

// return 0 means Pass, return 1 means Fail.
int main(int argc, char *argv[])
{
	tst_resm(TINFO, "Begin: HyperThreading Interrupt");

#if (!defined __i386__ && !defined __x86_64__)
	tst_brkm(TCONF, NULL,
		 "This test suite can only execute on x86 architecture.");
#else
	if (!check_ht_capability()) {
		if (HT_InterruptDistribution())
			tst_resm(TPASS,
				 "Interrupt distribution is balanceable.");
		else
			tst_resm(TFAIL,
				 "Interrupt distribution is not balanceable.");
	} else {
		tst_brkm(TCONF, NULL, "HT is not enabled or not supported.");
	}
#endif

	tst_resm(TINFO, "End: HyperThreading Interrupt");

	tst_exit();
}
