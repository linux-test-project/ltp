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
 * File: cap_bounds_rw
 * Author: Serge Hallyn
 * Purpose: test dropping capabilities from bounding set
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

char *TCID = "cap_bounds_rw";
int TST_TOTAL = 1;
static int cap_last_cap;

int check_remaining_caps(int lastdropped)
{
	int i;
	int ret;

	for (i = 0; i <= lastdropped; i++) {
#if HAVE_DECL_PR_CAPBSET_READ
		ret = prctl(PR_CAPBSET_READ, i);
#else
		errno = ENOSYS;
		ret = -1;
#endif
		if (ret == -1) {
			tst_brkm(TBROK,
				 NULL,
				 "Failed to read bounding set during sanity check\n");
		}
		if (ret == 1) {
			tst_resm(TFAIL,
				 "Bit %d should have been dropped but wasn't\n",
				 i);
			return i;
		}
	}
#ifdef HAVE_LIBCAP
	for (; i <= cap_last_cap; i++) {
#if HAVE_DECL_PR_CAPBSET_READ
		ret = prctl(PR_CAPBSET_READ, i);
#else
		errno = ENOSYS;
		ret = -1;
#endif
		if (ret == -1) {
			tst_brkm(TBROK,
				 NULL,
				 "Failed to read bounding set during sanity check\n");
		}
		if (ret == 0) {
			tst_resm(TFAIL,
				 "Bit %d wasn't yet dropped, but isn't in bounding set\n",
				 i);
			return -i;
		}
	}
#endif
	return 0;
}

int main(void)
{
	int ret = 1;
	int i;

#ifdef HAVE_LIBCAP
	cap_last_cap = CAP_LAST_CAP;
	if (access(PROC_CAP_LAST, R_OK) == 0) {
		SAFE_FILE_SCANF(NULL, PROC_CAP_LAST, "%d", &cap_last_cap);
		if (cap_last_cap > CAP_LAST_CAP)
		       cap_last_cap = CAP_LAST_CAP;
	}
#if HAVE_DECL_PR_CAPBSET_DROP
	ret = prctl(PR_CAPBSET_READ, -1);
#else
	errno = ENOSYS;
	ret = -1;
#endif
	if (ret != -1) {
		tst_brkm(TFAIL, NULL,
			 "prctl(PR_CAPBSET_DROP, -1) returned %d\n",
			 ret);
	}
	/* Ideally I'd check CAP_LAST_CAP+1, but userspace
	 * tends to be far too unreliable to trust CAP_LAST_CAP>
	 * We could test using kernel API, but that's what we're
	 * testing...  So let's take an insanely high value */
#define INSANE 63
#define max(x,y) (x > y ? x : y)
#if HAVE_DECL_PR_CAPBSET_DROP
	ret = prctl(PR_CAPBSET_DROP, max(INSANE, CAP_LAST_CAP + 1));
#else
	errno = ENOSYS;
	ret = -1;
#endif
	if (ret != -1) {
		tst_resm(TFAIL, "prctl(PR_CAPBSET_DROP, %d) returned %d\n",
			 max(INSANE, CAP_LAST_CAP + 1), ret);
		tst_resm(TINFO, " %d is should not exist\n",
			 max(INSANE, CAP_LAST_CAP + 1));
		tst_exit();
	}
	for (i = 0; i <= cap_last_cap; i++) {
#if HAVE_DECL_PR_CAPBSET_DROP
		ret = prctl(PR_CAPBSET_DROP, i);
#else
		errno = ENOSYS;
		ret = -1;
#endif
		if (ret != 0) {
			tst_resm(TFAIL,
				 "prctl(PR_CAPBSET_DROP, %d) returned %d\n", i,
				 ret);
			if (ret == -1)
				tst_resm(TINFO, "errno was %d\n", errno);
			tst_exit();
		}
		ret = check_remaining_caps(i);
		if (ret > 0) {
			tst_brkm(TFAIL,
				 NULL,
				 "after dropping bits 0..%d, %d was still in bounding set\n",
				 i, ret);
		} else if (ret < 0) {
			tst_brkm(TFAIL,
				 NULL,
				 "after dropping bits 0..%d, %d was not in bounding set\n",
				 i, -ret);
		}
	}
	tst_resm(TPASS, "PR_CAPBSET_DROP tests passed\n");
#else
	tst_resm(TCONF, "System doesn't have POSIX capabilities.");
#endif
	tst_exit();
}
