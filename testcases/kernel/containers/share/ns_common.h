/* Copyright (c) 2015 Red Hat, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 2 the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#ifndef __NS_COMMON_H__
#define __NS_COMMON_H__
#include <sched.h>
#include "lapi/sched.h"

#define PROC_PATH "/proc"
#define NS_TOTAL 6


struct param {
	const char *name;
	int flag;
};

struct param params[] = {
	{"ipc",  CLONE_NEWIPC},
	{"mnt",  CLONE_NEWNS},
	{"net",  CLONE_NEWNET},
	{"pid",  CLONE_NEWPID},
	{"user", CLONE_NEWUSER},
	{"uts",  CLONE_NEWUTS},
	{NULL,   0}
};


struct param *get_param(const char *name)
{
	int i;

	for (i = 0; params[i].name; i++) {
		if (!strcasecmp(params[i].name, name))
			return params + i;
	}

	return NULL;
}


#endif
