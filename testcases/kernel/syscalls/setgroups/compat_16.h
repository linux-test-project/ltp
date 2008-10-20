/*
 *
 *   Copyright (c) Red Hat Inc., 2008
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
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Author: Masatake YAMATO <yamato@redhat.com> */

#ifndef __SETGROUPS_COMPAT_16_H__
#define __SETGROUPS_COMPAT_16_H__

#include "compat_gid.h"
#include "linux_syscall_numbers.h"


/* For avoiding circular dependency. */
extern void cleanup(void);

#ifdef TST_USE_COMPAT16_SYSCALL

long
SETGROUPS(size_t gidsetsize, GID_T *list)
{
	return syscall(__NR_setgroups, gidsetsize, list);
}

int
GETGROUPS(int size16, GID_T *list16)
{
	int r;
	int i;

	gid_t *list32;

	list32 = malloc(size16 * sizeof(gid_t));
	if (list32 == NULL)
	  tst_brkm(TBROK, NULL, "malloc failed to alloc %d errno "
		   " %d ", size16 * sizeof(gid_t), errno);

	r = getgroups(size16, list32);
	if (r < 0)
	  goto out;
  
	for (i = 0; i < r; i++) {
		if (!GID_SIZE_CHECK(list32[i]))
		  tst_brkm(TBROK,
			   cleanup,
			   "gid returned from getgroups is too large for testing setgroups32");
		list16[i] = (GID_T)list32[i];
	}

 out:
	free(list32);
	return r;
}

#else
int
SETGROUPS(size_t size, const GID_T *list)
{
	return setgroups(size, list);
}

int
GETGROUPS(int size, GID_T *list)
{
	return getgroups(size, list);
}

#endif	/* TST_USE_COMPAT16_SYSCALL */

#endif /* __SETGROUPS_COMPAT_16_H__ */
