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
#include "safe_macros.h"

static int is_kvm(void)
{
	FILE *cpuinfo;
	char line[64];
	int found;

	/* this doesn't work with custom -cpu values, since there's
	 * no easy, reasonable or reliable way to work around those */
	cpuinfo = SAFE_FOPEN(NULL, "/proc/cpuinfo", "r");
	found = 0;
	while (fgets(line, sizeof(line), cpuinfo) != NULL) {
		if (strstr(line, "QEMU Virtual CPU")) {
			found = 1;
			break;
		}
	}

	SAFE_FCLOSE(NULL, cpuinfo);
	return found;
}

static int is_xen(void)
{
	char hypervisor_type[4];

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

static int is_ibmz(int virt_type)
{
	FILE *sysinfo;
	char line[64];
	int found_lpar, found_zvm;

	if (access("/proc/sysinfo", F_OK) != 0)
		return 0;

	sysinfo = SAFE_FOPEN(NULL, "/proc/sysinfo", "r");
	found_lpar = 0;
	found_zvm = 0;
	while (fgets(line, sizeof(line), sysinfo) != NULL) {
		if (strstr(line, "LPAR"))
			found_lpar = 1;
		else if (strstr(line, "z/VM"))
			found_zvm = 1;
	}

	SAFE_FCLOSE(NULL, sysinfo);

	switch (virt_type) {
	case VIRT_IBMZ:
		return found_lpar;
	case VIRT_IBMZ_LPAR:
		return found_lpar && !found_zvm;
	case VIRT_IBMZ_ZVM:
		return found_lpar && found_zvm;
	default:
		return 0;
	}
}

static int try_systemd_detect_virt(void)
{
	FILE *f;
	char virt_buf[64];
	int ret;
	char *virt_type = getenv("LTP_VIRT_OVERRIDE");

	if (virt_type) {
		if (!strcmp("", virt_type))
			return 0;

		goto cmp;
	}

	virt_type = virt_buf;

	/* See tst_cmd.c */
	void *old_handler = signal(SIGCHLD, SIG_DFL);

	f = popen("systemd-detect-virt", "r");
	if (!f) {
		signal(SIGCHLD, old_handler);
		return 0;
	}

	if (!fgets(virt_type, sizeof(virt_type), f))
		virt_type[0] = '\0';

	ret = pclose(f);

	signal(SIGCHLD, old_handler);

	/*
	 * systemd-detect-virt not found by shell or no virtualization detected
	 * (systemd-detect-virt returns non-zero)
         */
	if (ret < 0 || (WIFEXITED(ret) && WEXITSTATUS(ret) == 127))
		return -1;

	if (ret)
		return 0;

cmp:
	if (!strncmp("kvm", virt_type, 3))
		return VIRT_KVM;

	if (!strncmp("xen", virt_type, 3))
		return VIRT_XEN;

	if (!strncmp("zvm", virt_type, 3))
		return VIRT_IBMZ_ZVM;

	if (!strncmp("microsoft", virt_type, 9))
		return VIRT_HYPERV;

	return VIRT_OTHER;
}

int tst_is_virt(int virt_type)
{
	int ret = try_systemd_detect_virt();

	if (ret > 0) {
		if (virt_type == VIRT_ANY)
			return 1;
		else
			return ret == virt_type;
	}

	switch (virt_type) {
	case VIRT_ANY:
		return is_xen() || is_kvm() || is_ibmz(VIRT_IBMZ);
	case VIRT_XEN:
		return is_xen();
	case VIRT_KVM:
		return is_kvm();
	case VIRT_IBMZ:
	case VIRT_IBMZ_LPAR:
	case VIRT_IBMZ_ZVM:
		return is_ibmz(virt_type);
	case VIRT_HYPERV:
	case VIRT_OTHER:
		return 0;
	}

	tst_brkm(TBROK, NULL, "invalid virt_type flag: %d", virt_type);
	return -1;
}
