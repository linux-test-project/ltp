/******************************************************************************/
/*                                                                            */
/* Copyright (c) Tetsuo Handa <penguin-kernel@I-love.SAKURA.ne.jp>, 2009      */
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
 * include.h
 *
 * Common functions for testing TOMOYO Linux's kernel.
 *
 * Copyright (C) 2005-2010  NTT DATA CORPORATION
 */
#include <errno.h>
#include <fcntl.h>
#include <linux/kdev_t.h>
#include <linux/unistd.h>
#include <pty.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#include <sched.h>
#include <stdarg.h>
#include <sys/mount.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/ip.h>
#include <err.h>
#include "test.h"

/*
 * Some architectures like mips n32 don't have __NR_uselib defined in the
 * system headers.
 */
#ifdef __NR_uselib
static inline int uselib(const char *library)
{
	return syscall(__NR_uselib, library);
}
#else
static inline int uselib(const char *library)
{
	errno = ENOSYS;
	return -1;
}
#endif

/* Is there an architecture without __NR_pivot_root defined? */
#ifdef __NR_pivot_root
static inline int pivot_root(const char *new_root, const char *put_old)
{
	return syscall(__NR_pivot_root, new_root, put_old);
}
#else
static inline int pivot_root(const char *new_root, const char *put_old)
{
	errno = ENOSYS;
	return -1;
}
#endif

/* The sysctl() wrapper is dead and newer arches omit it now. */
static inline int write_sysctl(const char *path, const char *value)
{
	FILE *fp = fopen(path, "w");
	if (!fp)
		return 1;
	fputs(value, fp);
	fclose(fp);
	return 0;
}

static inline int read_sysctl(const char *path, char *value, int len)
{
	char scratch[100];
	FILE *fp = fopen(path, "r");
	if (!fp)
		return 1;
	if (!value) {
		value = scratch;
		len = sizeof(scratch);
	}
	if (fgets(value, len, fp))
		/* ignore */;
	fclose(fp);
	return 0;
}

/* Should be a fairly benign path to bang on. */
#define TEST_SYSCTL_PATH "/proc/sys/net/ipv4/ip_local_port_range"

#define proc_policy_dir              "/sys/kernel/security/tomoyo/"
#define proc_policy_domain_policy    "/sys/kernel/security/tomoyo/domain_policy"
#define proc_policy_exception_policy "/sys/kernel/security/tomoyo/exception_policy"
#define proc_policy_profile          "/sys/kernel/security/tomoyo/profile"
#define proc_policy_manager          "/sys/kernel/security/tomoyo/manager"
#define proc_policy_query            "/sys/kernel/security/tomoyo/query"
#define proc_policy_grant_log        "/sys/kernel/security/tomoyo/grant_log"
#define proc_policy_reject_log       "/sys/kernel/security/tomoyo/reject_log"
#define proc_policy_domain_status    "/sys/kernel/security/tomoyo/.domain_status"
#define proc_policy_process_status   "/sys/kernel/security/tomoyo/.process_status"
#define proc_policy_self_domain      "/sys/kernel/security/tomoyo/self_domain"

static FILE *profile_fp = NULL;
static FILE *domain_fp = NULL;
static FILE *exception_fp = NULL;
static char self_domain[4096] = "";
static pid_t pid = 0;

static void clear_status(void)
{
	static const char *keywords[] = {
		"file::execute",
		"file::open",
		"file::create",
		"file::unlink",
		"file::mkdir",
		"file::rmdir",
		"file::mkfifo",
		"file::mksock",
		"file::truncate",
		"file::symlink",
		"file::rewrite",
		"file::mkblock",
		"file::mkchar",
		"file::link",
		"file::rename",
		"file::chmod",
		"file::chown",
		"file::chgrp",
		"file::ioctl",
		"file::chroot",
		"file::mount",
		"file::umount",
		"file::pivot_root",
		NULL
	};
	int i;
	FILE *fp = fopen(proc_policy_profile, "r");
	static char buffer[4096];
	if (!fp) {
		fprintf(stderr, "Can't open %s\n", proc_policy_profile);
		exit(1);
	}
	for (i = 0; keywords[i]; i++)
		fprintf(profile_fp,
			"255-CONFIG::%s={ mode=disabled }\n",
			keywords[i]);
	while (memset(buffer, 0, sizeof(buffer)),
	       fgets(buffer, sizeof(buffer) - 10, fp)) {
		const char *mode;
		char *cp = strchr(buffer, '=');
		if (!cp)
			continue;
		*cp = '\0';
		mode = cp + 1;
		cp = strchr(buffer, '-');
		if (!cp)
			continue;
		*cp++ = '\0';
		if (strcmp(buffer, "0"))
			continue;
		fprintf(profile_fp, "255-%s", cp);
		if (!strcmp(cp, "COMMENT"))
			mode = "Profile for kernel test\n";
		else
			mode = "{ mode=disabled verbose=no }\n";
		fprintf(profile_fp, "255-%s=%s", cp, mode);
	}
	fprintf(profile_fp, "255-PREFERENCE::learning= verbose=no\n");
	fprintf(profile_fp, "255-PREFERENCE::enforcing= verbose=no\n");
	fprintf(profile_fp, "255-PREFERENCE::permissive= verbose=no\n");
	fprintf(profile_fp, "255-PREFERENCE::disabled= verbose=no\n");
	fprintf(profile_fp, "255-PREFERENCE::learning= max_entry=2048\n");
	fflush(profile_fp);
	fclose(fp);
}

static void tomoyo_test_init(void)
{
	pid = getpid();
	if (access(proc_policy_dir, F_OK)) {
		fprintf(stderr, "You can't use this program for this kernel."
			"\n");
		exit(1);
	}
	profile_fp = fopen(proc_policy_profile, "w");
	if (!profile_fp) {
		fprintf(stderr, "Can't open %s .\n", proc_policy_profile);
		exit(1);
	}
	setlinebuf(profile_fp);
	domain_fp = fopen(proc_policy_domain_policy, "w");
	if (!domain_fp) {
		fprintf(stderr, "Can't open %s .\n",
			proc_policy_domain_policy);
		exit(1);
	}
	setlinebuf(domain_fp);
	exception_fp = fopen(proc_policy_exception_policy, "w");
	if (!exception_fp) {
		fprintf(stderr, "Can't open %s .\n",
			proc_policy_exception_policy);
		exit(1);
	}
	setlinebuf(exception_fp);
	if (fputc('\n', profile_fp) != '\n' || fflush(profile_fp)) {
		fprintf(stderr, "You need to register this program to %s to "
			"run this program.\n", proc_policy_manager);
		exit(1);
	}
	clear_status();
	{
		FILE *fp = fopen(proc_policy_self_domain, "r");
		memset(self_domain, 0, sizeof(self_domain));
		if (!fp || !fgets(self_domain, sizeof(self_domain) - 1, fp) ||
		    fclose(fp)) {
			fprintf(stderr, "Can't open %s .\n",
				proc_policy_self_domain);
			exit(1);
		}
	}
	fprintf(domain_fp, "select pid=%u\n", pid);
	fprintf(domain_fp, "use_profile 255\n");
	fprintf(domain_fp, "allow_read/write /sys/kernel/security/tomoyo/domain_policy\n");
	fprintf(domain_fp, "allow_truncate /sys/kernel/security/tomoyo/domain_policy\n");
	fprintf(domain_fp, "allow_read/write /sys/kernel/security/tomoyo/exception_policy\n");
	fprintf(domain_fp, "allow_truncate /sys/kernel/security/tomoyo/exception_policy\n");
	fprintf(domain_fp, "allow_read/write /sys/kernel/security/tomoyo/profile\n");
	fprintf(domain_fp, "allow_truncate /sys/kernel/security/tomoyo/profile\n");
}

static void BUG(const char *fmt, ...)
	__attribute__ ((format(printf, 1, 2)));

static void BUG(const char *fmt, ...)
{
	va_list args;
	printf("BUG: ");
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	putchar('\n');
	fflush(stdout);
	while (1)
		sleep(100);
}

int write_domain_policy(const char *policy, int is_delete)
{
	FILE *fp = fopen(proc_policy_domain_policy, "r");
	char buffer[8192];
	int domain_found = 0;
	int policy_found = 0;
	memset(buffer, 0, sizeof(buffer));
	if (!fp) {
		BUG("Can't read %s", proc_policy_domain_policy);
		return 0;
	}
	if (is_delete)
		fprintf(domain_fp, "delete ");
	fprintf(domain_fp, "%s\n", policy);
	while (fgets(buffer, sizeof(buffer) - 1, fp)) {
		char *cp = strchr(buffer, '\n');
		if (cp)
			*cp = '\0';
		if (!strncmp(buffer, "<kernel>", 8))
			domain_found = !strcmp(self_domain, buffer);
		if (!domain_found)
			continue;
		/* printf("<%s>\n", buffer); */
		if (strcmp(buffer, policy))
			continue;
		policy_found = 1;
		break;
	}
	fclose(fp);
	if (policy_found == is_delete) {
		BUG("Can't %s %s", is_delete ? "delete" : "append",
		    policy);
		return 0;
	}
	errno = 0;
	return 1;

}

int write_exception_policy(const char *policy, int is_delete)
{
	FILE *fp = fopen(proc_policy_exception_policy, "r");
	char buffer[8192];
	int policy_found = 0;
	memset(buffer, 0, sizeof(buffer));
	if (!fp) {
		BUG("Can't read %s", proc_policy_exception_policy);
		return 0;
	}
	if (is_delete)
		fprintf(exception_fp, "delete ");
	fprintf(exception_fp, "%s\n", policy);
	while (fgets(buffer, sizeof(buffer) - 1, fp)) {
		char *cp = strchr(buffer, '\n');
		if (cp)
			*cp = '\0';
		if (strcmp(buffer, policy))
			continue;
		policy_found = 1;
		break;
	}
	fclose(fp);
	if (policy_found == is_delete) {
		BUG("Can't %s %s", is_delete ? "delete" : "append",
		    policy);
		return 0;
	}
	errno = 0;
	return 1;

}

int set_profile(const int mode, const char *name)
{
	static const char *modes[4] = { "disabled", "learning", "permissive",
					"enforcing" };
	FILE *fp = fopen(proc_policy_profile, "r");
	char buffer[8192];
	int policy_found = 0;
	const int len = strlen(name);
	if (!fp) {
		BUG("Can't read %s", proc_policy_profile);
		return 0;
	}
	fprintf(profile_fp, "255-CONFIG::%s=%s\n", name, modes[mode]);
	while (memset(buffer, 0, sizeof(buffer)),
	       fgets(buffer, sizeof(buffer) - 1, fp)) {
		char *cp = strchr(buffer, '\n');
		if (cp)
			*cp = '\0';
		if (strncmp(buffer, "255-CONFIG::", 12) ||
		    strncmp(buffer + 12, name, len) ||
		    buffer[12 + len] != '=')
			continue;
		if (strstr(buffer + 13 + len, modes[mode]))
			policy_found = 1;
		break;
	}
	fclose(fp);
	if (!policy_found) {
		BUG("Can't change profile to 255-CONFIG::%s=%s",
		    name, modes[mode]);
		return 0;
	}
	errno = 0;
	return 1;
}
