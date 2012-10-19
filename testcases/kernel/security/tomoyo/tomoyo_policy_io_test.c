/******************************************************************************/
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
 * tomoyo_policy_io_test.c
 *
 * Testing program for security/tomoyo/
 *
 * Copyright (C) 2005-2010  NTT DATA CORPORATION
 */
#include "include.h"

static FILE *policy_fp = NULL;
static const char *policy_file = "";

static void try_io(const char *policy, const char should_success)
{
	FILE *fp = fopen(policy_file, "r");
	char buffer[8192];
	int policy_found = 0;
	memset(buffer, 0, sizeof(buffer));
	printf("%s: ", policy);
	fprintf(policy_fp, "%s\n", policy);
	if (!fp) {
		printf("BUG: policy read failed\n");
		return;
	}
	while (fgets(buffer, sizeof(buffer) - 1, fp)) {
		char *cp = strchr(buffer, '\n');
		if (cp)
			*cp = '\0';
		if (!strcmp(buffer, policy)) {
			policy_found = 1;
			break;
		}
	}
	fclose(fp);
	if (should_success) {
		if (policy_found)
			printf("OK\n");
		else
			printf("BUG: policy write failed\n");
	} else {
		if (!policy_found)
			printf("OK : write rejected.\n");
		else
			printf("BUG: policy write not rejected.\n");
	}
	fprintf(policy_fp, "delete %s\n", policy);
}

static void stage_policy_io_test(void)
{
	int i;
	policy_file = proc_policy_domain_policy;
	policy_fp = domain_fp;
	for (i = 0; i < 3; i++) {
		try_io("allow_chroot /", 1);
		try_io("allow_chroot ", 0);
		try_io("allow_chroot /mnt0/", 1);
		try_io("allow_chroot /var1/chroot2/", 1);
		try_io("allow_chroot /mnt0/", 1);
		try_io("allow_chroot /mnt0/", 1);
		try_io("allow_chroot /mnt0/", 1);
		try_io("allow_chroot /mnt\\?\\*/", 1);
		try_io("allow_chroot /mnt\\?\\*/", 1);
		try_io("allow_unmount /", 1);
		try_io("allow_unmount /sys1/", 1);
		try_io("allow_unmount /initrd2/", 1);
		try_io("allow_unmount /initrd/dev3/", 1);
		try_io("allow_unmount /initrd/\\*\\+/", 1);
		try_io("allow_unmount /initrd/\\@\\*/", 1);
		try_io("allow_unmount /initrd2/", 1);
		try_io("allow_pivot_root / /proc3/", 1);
		try_io("allow_pivot_root /sys5/ /proc3/", 1);
		try_io("allow_pivot_root /sys/", 0);
		try_io("allow_pivot_root *", 0);
		try_io("allow_pivot_root /sys5/ /proc3/", 1);
		try_io("allow_mount / / --bind 0xD", 1);
		try_io("allow_mount / / --move 0xF", 1);
		try_io("allow_mount / --remount", 0);
		try_io("allow_mount /", 0);
		try_io("allow_mount none /tmp/ tmpfs 0x1", 1);
		try_io("allow_mount none /tmp/ tmpfs", 0);
		try_io("allow_mount none /tmp/ nonexistent 0x0", 1);
		try_io("allow_mount none /proc/ proc 0x0", 1);
		try_io("allow_mount none /selinux/ selinuxfs 0x0", 1);
		try_io("allow_mount /proc/bus/usb /proc/bus/usb/ usbfs 0x0", 1);
		try_io("allow_mount none /dev/pts/ devpts 0x0", 1);
		try_io("allow_mount any / --remount 0xC00", 1);
		try_io("allow_mount /dev/sda1 /boot/ ext3 0xC00", 1);
		try_io("allow_mount none /dev/shm/ tmpfs 0x0", 1);
		try_io("allow_mount none /proc/sys/fs/binfmt_misc/ binfmt_misc "
		       "0x0", 1);
		try_io("allow_mount none /proc/sys/fs/binfmt_misc/ binfmt_misc "
		       "0x0 0x1", 0);
		try_io("allow_mount none /proc/sys/fs/binfmt_misc/ tmpfs "
		       "binfmt_misc 0x0", 0);
		try_io("allow_mount /proc/bus/usb /proc/bus/usb/ usbfs 0x0", 1);
	}
	policy_file = proc_policy_exception_policy;
	policy_fp = exception_fp;
	for (i = 0; i < 3; i++) {
		try_io("allow_read /tmp/abc", 1);
		try_io("allow_read /tmp/abc\\*", 1);
		try_io("allow_read abc", 1);
		try_io("allow_read /tmp/abc/", 1);
		try_io("allow_read", 0);
		try_io("allow_read *", 1);
		try_io("file_pattern /\\*\\*\\*", 1);
		try_io("file_pattern /abc", 1);
		try_io("file_pattern /abc /def", 0);
		try_io("file_pattern abcdef", 1);
		try_io("path_group TEST /", 1);
		try_io("path_group TEST /boo", 1);
		try_io("path_group TEST /bar", 1);
		try_io("path_group TEST /\\*", 1);
		try_io("path_group TEST / /", 0);
		try_io("path_group TEST /boo", 1);
		try_io("path_group TEST /bar", 1);
		try_io("path_group TEST boo", 1);
		try_io("path_group TEST boo/", 1);
		try_io("path_group TEST /bar", 1);
		try_io("path_group TEST3 /\\*", 1);
		try_io("path_group TEST3 / /", 0);
		try_io("path_group TEST3 /boo", 1);
		try_io("path_group TEST3 /bar", 1);
		try_io("path_group TEST3 boo", 1);
		try_io("path_group TEST3 boo/", 1);
		try_io("deny_rewrite /", 1);
		try_io("deny_rewrite /foo", 1);
		try_io("deny_rewrite /\\*", 1);
		try_io("deny_rewrite /\\:", 0);
		try_io("deny_rewrite / /", 0);
		try_io("deny_rewrite @/TEST", 1);
		try_io("aggregator /boo/\\* /BOO", 1);
		try_io("aggregator /boo/\\* /BOO\\*", 0);
		try_io("aggregator /boo/\\*/ /BOO", 1);
		try_io("aggregator /boo/\\* /BOO/", 1);
		try_io("keep_domain <kernel>", 1);
		try_io("keep_domain <kernel> /sbin/init", 1);
		try_io("keep_domain <kernel> foo", 0);
		try_io("keep_domain <kernel> \\*", 0);
		try_io("keep_domain /ssh", 1);
		try_io("keep_domain /ssh /foo", 0);
		try_io("keep_domain /foo from <kernel>", 1);
		try_io("keep_domain /foo from <kernel> /sbin/init", 1);
		try_io("keep_domain from <kernel> /sbin/init", 0);
		try_io("keep_domain \\* from <kernel> /sbin/init", 0);
		try_io("no_keep_domain <kernel>", 1);
		try_io("no_keep_domain <kernel> /sbin/init", 1);
		try_io("no_keep_domain <kernel> foo", 0);
		try_io("no_keep_domain <kernel> \\*", 0);
		try_io("no_keep_domain /ssh", 1);
		try_io("no_keep_domain /ssh /foo", 0);
		try_io("no_keep_domain /foo from <kernel>", 1);
		try_io("no_keep_domain /foo from <kernel> /sbin/init", 1);
		try_io("no_keep_domain from <kernel> /sbin/init", 0);
		try_io("no_keep_domain \\* from <kernel> /sbin/init", 0);
		try_io("initialize_domain /foo", 1);
		try_io("initialize_domain /\\*", 1);
		try_io("initialize_domain /foo /bar", 0);
		try_io("initialize_domain /foo from /bar", 1);
		try_io("initialize_domain /foo from <kernel> /bar", 1);
		try_io("initialize_domain /\\* from <kernel>", 1);
		try_io("initialize_domain /foo from <kernel> \\*", 0);
		try_io("no_initialize_domain /foo", 1);
		try_io("no_initialize_domain /\\*", 1);
		try_io("no_initialize_domain /foo /bar", 0);
		try_io("no_initialize_domain /foo from /bar", 1);
		try_io("no_initialize_domain /foo from <kernel> /bar", 1);
		try_io("no_initialize_domain /\\* from <kernel>", 1);
		try_io("no_initialize_domain /foo from <kernel> \\*", 0);
	}
}

int main(int argc, char *argv[])
{
	tomoyo_test_init();
	stage_policy_io_test();
	return 0;
}
