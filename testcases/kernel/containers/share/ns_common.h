/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2015 Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2015-2023
 */

#ifndef __NS_COMMON_H__
#define __NS_COMMON_H__
#include <sched.h>
#include "lapi/sched.h"

#define PROC_PATH "/proc"

struct param {
	const char *name;
	int flag;
};

static struct param params[] = {
	{"ipc",  CLONE_NEWIPC},
	{"mnt",  CLONE_NEWNS},
	{"net",  CLONE_NEWNET},
	{"pid",  CLONE_NEWPID},
	{"user", CLONE_NEWUSER},
	{"uts",  CLONE_NEWUTS},
	{NULL,   0}
};

#define NS_TOTAL (ARRAY_SIZE(params) - 1)

static struct param *get_param(const char *name)
{
	int i;

	for (i = 0; params[i].name; i++) {
		if (!strcasecmp(params[i].name, name))
			return params + i;
	}

	return NULL;
}

#endif
