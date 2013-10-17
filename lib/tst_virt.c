/*
 * Copyright (C) 2013 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <unistd.h>
#include "test.h"

static int is_xen(void)
{
	char hypervisor_type[3];

	if (access("/proc/xen", F_OK) == 0)
		return 1;

	if (access("/sys/hypervisor/type", F_OK) == 0) {
		SAFE_FILE_SCANF(NULL, "/sys/hypervisor/type", "%3s",
			hypervisor_type);
		return strncmp("xen", hypervisor_type,
			sizeof(hypervisor_type)) == 0;
	}

	return 0;
}

int tst_is_virt(int virt_type)
{
	switch (virt_type) {
	case VIRT_XEN:
		return is_xen();
	}
	tst_brkm(TBROK, NULL, "invalid virt_type flag: %d", virt_type);
	return 0;
}
