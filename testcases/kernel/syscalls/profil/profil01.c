/*
 *   Copyright (c) International Business Machines  Corp., 2002
 *   Copyright (C) 2014 Linux Test Project, Inc.
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 */
/*
 * ALGORITHM
 *	Set up a profiling buffer, turn profiling on, set a timer for
 *	cpu time, spin the pc and wait for timer to go off.
 *	The profiling buffer should contain some info, highly concentrated.
 *	We just do a "looks reasonable" check.
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/abisize.h"
#include "config.h"

char *TCID = "profil01";

#if HAVE_PROFIL

#define PROFIL_TIME 5

/* Should be large enough to hold data for test_profil() .text,
 * on x86_64 this is ~600 bytes, so 16k should enough for all arches.
 * We will monitor 16k on each side around current pc value,
 * just in case compiler put call to get_pc() below "data shuffling" code */
#define PROFIL_BUFLEN (32*1024)

int TST_TOTAL = 1;

static volatile sig_atomic_t profil_done;

static void alrm_handler(int sig)
{
	(void) sig;
	profil_done = 1;
}

static void __attribute__ ((noinline)) *get_pc(void)
{
#if defined(__s390__) && defined(TST_ABI32)
	/* taken from glibc,
	 *   sysdeps/unix/sysv/linux/s390/s390-32/profil-counter.h
	 * 31-bit s390 pointers don't use the 32th bit, however integers do,
	 * so wrap the value around at 31 bits */
	return (void *)
		((unsigned long) __builtin_return_address(0) & 0x7fffffffUL);
#else
	return __builtin_return_address(0);
#endif
}

static void test_profil(void)
{
	unsigned short buf[PROFIL_BUFLEN] = { 0 };
	volatile int data[8] = { 0 };
	size_t offset = (size_t) get_pc() - PROFIL_BUFLEN/2, count = 0;
	int ret, i;

	/* reset for test looping */
	profil_done = 0;

	/* profil_count in glibc calculates offset as
	 *   i = (pc - pc_offset - (void *) 0) / 2
	 *   i = i * pc_scale / 65536
	 * set scale to 2*65536 to have 1:1 mapping for $pc */
	ret = profil(buf, sizeof(buf), offset, 2*65536);
	if (ret)
		tst_brkm(TBROK, NULL, "profil returned: %d\n", ret);

	signal(SIGALRM, alrm_handler);
	alarm(PROFIL_TIME);

	while (!profil_done) {
		if (data[0])
			data[0] = -data[7];
		else
			data[1] = data[0] / 2;
		if (data[2])
			data[2] = data[1] * 2;
		else
			data[3] = data[2] + data[0];
		if (data[4])
			data[4] = data[3] - data[1];
		else
			data[5] = data[4] * data[2];
		if (data[6])
			data[6] = data[5] + data[3];
		else
			data[7] = data[6] - data[4];
	}

	for (i = 0; i < PROFIL_BUFLEN; i++)
		if (buf[i]) {
			tst_resm(TINFO, "buf[0x%04x]=%d", i, buf[i]);
			count += buf[i];
		}

	if (count > 0)
		tst_resm(TPASS, "profil recorded some data");
	else
		tst_resm(TFAIL, "profil failed to record anything");
}

int main(int ac, char *av[])
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++)
		test_profil();

	tst_exit();
}
#else /* systems without profil() */
int main(void)
{
        tst_brkm(TCONF, NULL, "system doesn't have profil() support");
}
#endif
