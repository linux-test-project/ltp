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
 * File: cap_bounds_r.c
 * Author: Serge Hallyn
 * Purpose: test reading of capability bounding set.
 * 	Test each valid cap value, as well as edge cases
 */

#include <errno.h>
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <sys/capability.h>
#endif
#include <sys/prctl.h>
#include "test.h"

char *TCID = "cap_bounds_r";
int TST_TOTAL=1;

int main(int argc, char *argv[])
{
#if HAVE_SYS_CAPABILITY_H
	int ret = 1;
	int i;

	for (i=0; i<=CAP_LAST_CAP; i++) {
#if HAVE_DECL_PR_CAPBSET_READ
		ret = prctl(PR_CAPBSET_READ, i);
#else
		errno = ENOSYS;
		ret = -1;
#endif
		if (ret != 1) {
			tst_resm(TFAIL, "prctl(PR_CAPBSET_READ, %d) returned %d\n", i, ret);
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
		tst_resm(TFAIL, "prctl(PR_CAPBSET_READ, -1) returned %d\n", ret);
		tst_exit();
	}

	/* Ideally I'd check CAP_LAST_CAP+1, but userspace
	 * tends to be far too unreliable to trust CAP_LAST_CAP>
	 * We could test using kernel API, but that's what we're
	 * testing...  So let's take an insanely high value */
#define INSANE 63
#define max(x,y) (x > y ? x : y)
#if HAVE_DECL_PR_CAPBSET_READ
	ret = prctl(PR_CAPBSET_READ, max(INSANE,CAP_LAST_CAP+1));
#else
	errno = ENOSYS;
	ret = -1;
#endif
	if (ret != -1) {
		tst_resm(TFAIL, "prctl(PR_CAPBSET_READ, %d) returned %d\n", CAP_LAST_CAP+1, ret);
		tst_resm(TINFO, " %d is CAP_LAST_CAP+1 and should not exist\n", CAP_LAST_CAP+1);
		tst_exit();
	}
	tst_resm(TPASS, "PR_CAPBSET_READ tests passed\n");
#else
	tst_resm(TCONF, "System doesn't have POSIX capabilities.");
#endif
	tst_exit();
}