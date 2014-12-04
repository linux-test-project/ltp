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
 * File: cap_bset_inh_bounds.c
 * Author: Serge Hallyn
 * Purpose: test bounding set constraint on pI
 * 	(X = the capability bounding set)
 * 	1. if N \in pI, then dropping N from X does not drop it from pI
 * 	2. if N \notin X and N \notin pI, then adding N to pI fails
 */

#include <errno.h>
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <linux/types.h>
#include <sys/capability.h>
#endif
#include <sys/prctl.h>
#include "test.h"

char *TCID = "cap_bounds_r";
int TST_TOTAL = 2;

int main(int argc, char *argv[])
{
#if HAVE_SYS_CAPABILITY_H
#if HAVE_DECL_PR_CAPBSET_READ && HAVE_DECL_PR_CAPBSET_DROP
#ifdef HAVE_LIBCAP
	int ret = 1;
	cap_value_t v[1];
	cap_flag_value_t f;
	cap_t cur, tmpcap;

	/* We pick a random capability... let's use CAP_SYS_ADMIN */
	/* make sure we have the capability now */
	ret = prctl(PR_CAPBSET_READ, CAP_SYS_ADMIN);
	if (ret != 1) {
		tst_brkm(TBROK, NULL, "Not starting with CAP_SYS_ADMIN\n");
	}

	/* Make sure it's in pI */
	cur = cap_from_text("all=eip");
	if (!cur) {
		tst_brkm(TBROK,
			 NULL,
			 "Failed to create cap_sys_admin+i cap_t (errno %d)\n",
			 errno);
	}
	ret = cap_set_proc(cur);
	if (ret) {
		tst_brkm(TBROK,
			 NULL,
			 "Failed to cap_set_proc with cap_sys_admin+i (ret %d errno %d)\n",
			 ret, errno);
	}
	cap_free(cur);
	cur = cap_get_proc();
	ret = cap_get_flag(cur, CAP_SYS_ADMIN, CAP_INHERITABLE, &f);
	if (ret || f != CAP_SET) {
		tst_brkm(TBROK, NULL, "Failed to add CAP_SYS_ADMIN to pI\n");
	}
	cap_free(cur);

	/* drop the capability from bounding set */
	ret = prctl(PR_CAPBSET_DROP, CAP_SYS_ADMIN);
	if (ret) {
		tst_resm(TFAIL,
			 "Failed to drop CAP_SYS_ADMIN from bounding set.\n");
		tst_resm(TINFO, "(ret=%d, errno %d)\n", ret, errno);
		tst_exit();
	}

	/* test 1: is CAP_SYS_ADMIN still in pI? */
	cur = cap_get_proc();
	ret = cap_get_flag(cur, CAP_SYS_ADMIN, CAP_INHERITABLE, &f);
	if (ret || f != CAP_SET) {
		tst_brkm(TFAIL,
			 NULL,
			 "CAP_SYS_ADMIN not in pI after dropping from bounding set\n");
	}
	tst_resm(TPASS,
		 "CAP_SYS_ADMIN remains in pI after removing from bounding set\n");

	tmpcap = cap_dup(cur);
	v[0] = CAP_SYS_ADMIN;
	ret = cap_set_flag(tmpcap, CAP_INHERITABLE, 1, v, CAP_CLEAR);
	if (ret) {
		tst_brkm(TFAIL, NULL,
			 "Failed to drop CAP_SYS_ADMIN from cap_t\n");
	}
	ret = cap_set_proc(tmpcap);
	if (ret) {
		tst_brkm(TFAIL, NULL,
			 "Failed to drop CAP_SYS_ADMIN from pI\n");
	}
	cap_free(tmpcap);
	/* test 2: can we put it back in pI? */
	ret = cap_set_proc(cur);
	if (ret == 0) {		/* success means pI was not bounded by X */
		tst_brkm(TFAIL,
			 NULL,
			 "Managed to put CAP_SYS_ADMIN back into pI though not in X\n");
	}
	cap_free(cur);

	tst_resm(TPASS,
		 "Couldn't put CAP_SYS_ADMIN back into pI when not in bounding set\n");
#else /* HAVE_LIBCAP */
	tst_resm(TCONF, "System doesn't have POSIX capabilities.");
#endif
#else /* HAVE_DECL_PR_CAPBSET_READ && HAVE_DECL_PR_CAPBSET_DROP */
	tst_resm(TCONF, "System doesn't have CAPBSET prctls.");
#endif
#else /* HAVE_SYS_CAPABILITY_H */
	tst_resm(TCONF, "System doesn't have sys/capability.h.");
#endif
	tst_exit();
}
