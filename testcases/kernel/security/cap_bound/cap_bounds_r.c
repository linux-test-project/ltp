/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2007, 2008           */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/
/*
 * File: cap_bounds_r.c
 * Author: Serge Hallyn
 * Purpose: test reading of capability bounding set.
 * 	Test each valid cap value, as well as edge cases
 */

#include <errno.h>
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <linux/types.h>
#include <sys/capability.h>
#endif
#include <sys/prctl.h>
#include <unistd.h>
#include "test.h"

#define PROC_CAP_LAST "/proc/sys/kernel/cap_last_cap"

char *TCID = "cap_bounds_r";
int TST_TOTAL = 1;

int main(void)
{
#ifdef HAVE_LIBCAP
	int ret = 1;
	int i;
	int cap_last_cap = CAP_LAST_CAP;

	if (access(PROC_CAP_LAST, R_OK) == 0) {
		SAFE_FILE_SCANF(NULL, PROC_CAP_LAST, "%d", &cap_last_cap);
		if (cap_last_cap > CAP_LAST_CAP)
		       cap_last_cap = CAP_LAST_CAP;
	}

	for (i = 0; i <= cap_last_cap; i++) {
#if HAVE_DECL_PR_CAPBSET_READ
		ret = prctl(PR_CAPBSET_READ, i);
#else
		errno = ENOSYS;
		ret = -1;
#endif
		if (ret != 1) {
			tst_resm(TFAIL,
				 "prctl(PR_CAPBSET_READ, %d) returned %d\n", i,
				 ret);
			if (ret == -1)
				tst_resm(TINFO, "errno was %d\n", errno);
			tst_exit();
		}
	}
#if HAVE_DECL_PR_CAPBSET_READ
	ret = prctl(PR_CAPBSET_READ, -1);
#else
	errno = ENOSYS;
	ret = -1;
#endif
	if (ret != -1) {
		tst_brkm(TFAIL, NULL,
			 "prctl(PR_CAPBSET_READ, -1) returned %d\n",
			 ret);
	}

	/* Ideally I'd check CAP_LAST_CAP+1, but userspace
	 * tends to be far too unreliable to trust CAP_LAST_CAP>
	 * We could test using kernel API, but that's what we're
	 * testing...  So let's take an insanely high value */
#define INSANE 63
#define max(x,y) (x > y ? x : y)
#if HAVE_DECL_PR_CAPBSET_READ
	ret = prctl(PR_CAPBSET_READ, max(INSANE, CAP_LAST_CAP + 1));
#else
	errno = ENOSYS;
	ret = -1;
#endif
	if (ret != -1) {
		tst_resm(TFAIL, "prctl(PR_CAPBSET_READ, %d) returned %d\n",
			 CAP_LAST_CAP + 1, ret);
		tst_resm(TINFO, " %d is CAP_LAST_CAP+1 and should not exist\n",
			 CAP_LAST_CAP + 1);
		tst_exit();
	}
	tst_resm(TPASS, "PR_CAPBSET_READ tests passed\n");
#else
	tst_resm(TCONF, "System doesn't have POSIX capabilities.");
#endif
	tst_exit();
}
