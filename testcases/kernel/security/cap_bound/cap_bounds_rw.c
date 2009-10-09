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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
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
#include <sys/capability.h>
#endif
#include <sys/prctl.h>
#include <test.h>

#ifndef CAP_LAST_CAP
#warning out-of-date capability.h does not define CAP_LAST_CAP
#define CAP_LAST_CAP 28  /* be ultra-conservative */
#endif

#ifndef CAP_BSET_READ
#warning CAP_BSET_READ not defined
#define CAP_BSET_READ 23
#endif

#ifndef CAP_BSET_DROP
#warning CAP_BSET_DROP not defined
#define CAP_BSET_DROP 24
#endif

char *TCID = "cap_bounds_rw";
int TST_TOTAL=1;

int errno;

int check_remaining_caps(int lastdropped)
{
	int i;
	int ret;

	for (i=0; i <= lastdropped; i++) {
		ret = prctl(CAP_BSET_READ, i);
		if (ret == -1) {
			tst_resm(TBROK, "Failed to read bounding set during sanity check\n");
			tst_exit();
		}
		if (ret == 1) {
			tst_resm(TFAIL, "Bit %d should have been dropped but wasn't\n", i);
			return i;
		}
	}
	for (; i<=CAP_LAST_CAP; i++) {
		ret = prctl(CAP_BSET_READ, i);
		if (ret == -1) {
			tst_resm(TBROK, "Failed to read bounding set during sanity check\n");
			tst_exit();
		}
		if (ret == 0) {
			tst_resm(TFAIL, "Bit %d wasn't yet dropped, but isn't in bounding set\n", i);
			return -i;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 1;
	int i;

	ret = prctl(CAP_BSET_DROP, -1);
	if (ret != -1) {
		tst_resm(TFAIL, "prctl(CAP_BSET_DROP, -1) returned %d\n", ret);
		tst_exit();
	}
	/* Ideally I'd check CAP_LAST_CAP+1, but userspace
	 * tends to be far too unreliable to trust CAP_LAST_CAP>
	 * We could test using kernel API, but that's what we're
	 * testing...  So let's take an insanely high value */
#define INSANE 63
#define max(x,y) (x > y ? x : y)
	ret = prctl(CAP_BSET_DROP, max(INSANE,CAP_LAST_CAP+1));
	if (ret != -1) {
		tst_resm(TFAIL, "prctl(CAP_BSET_DROP, %d) returned %d\n", max(INSANE, CAP_LAST_CAP+1), ret);
		tst_resm(TINFO, " %d is should not exist\n", max(INSANE, CAP_LAST_CAP+1));
		tst_exit();
	}
	for (i=0; i<=CAP_LAST_CAP; i++) {
		ret = prctl(CAP_BSET_DROP, i);
		if (ret != 0) {
			tst_resm(TFAIL, "prctl(CAP_BSET_DROP, %d) returned %d\n", i, ret);
			if (ret == -1)
				tst_resm(TINFO, "errno was %d\n", errno);
			tst_exit();
		}
		ret = check_remaining_caps(i);
		if (ret > 0) {
			tst_resm(TFAIL, "after dropping bits 0..%d, %d was still in bounding set\n",
				i, ret);
			tst_exit();
		} else if (ret < 0) {
			tst_resm(TFAIL, "after dropping bits 0..%d, %d was not in bounding set\n",
				i, -ret);
			tst_exit();
		}
	}
	tst_resm(TPASS, "CAP_BSET_DROP tests passed\n");
	tst_exit();
}
