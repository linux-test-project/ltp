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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/
/*
 * include.h
 *
 * Common functions for testing TOMOYO Linux's kernel.
 *
 * Copyright (C) 2005-2009  NTT DATA CORPORATION
 *
 * Version: 2.2.0   2009/06/23
 *
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
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

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

#define proc_policy_dir              "/sys/kernel/security/tomoyo/"
#define proc_policy_domain_policy    proc_policy_dir "domain_policy"
#define proc_policy_exception_policy proc_policy_dir "exception_policy"
#define proc_policy_profile          proc_policy_dir "profile"
#define proc_policy_self_domain      proc_policy_dir "self_domain"

static void fprintf_encoded(FILE *fp, const char *pathname)
{
	while (1) {
		unsigned char c = *(const unsigned char *) pathname++;
		if (!c)
			break;
		if (c == '\\') {
			fputc('\\', fp);
			fputc('\\', fp);
		} else if (c > ' ' && c < 127) {
			fputc(c, fp);
		} else {
			fprintf(fp, "\\%c%c%c", (c >> 6) + '0',
				((c >> 3) & 7) + '0', (c & 7) + '0');
		}
	}
}

static char self_domain[4096] = "";
static FILE *fp_domain = NULL;
static FILE *fp_exception = NULL;
static FILE *fp_profile = NULL;

static void write_profile(const char *cp)
{
	fprintf(fp_profile, "%s", cp);
	fflush(fp_profile);
}

static void clear_status(void)
{
	FILE *fp = fopen(proc_policy_profile, "r");
	static char buffer[4096];
	if (!fp) {
		fprintf(stderr, "Can't open %s\n", proc_policy_profile);
		exit(1);
	}
	while (memset(buffer, 0, sizeof(buffer)),
	       fgets(buffer, sizeof(buffer) - 10, fp)) {
		const char *mode;
		int v;
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
		if (!strcmp(cp, "COMMENT"))
			mode = "=Profile for kernel test";
		else if (sscanf(mode, "%u", &v) == 1)
			mode = "=0";
		else
			mode = "=disabled";
		fprintf(fp_profile, "255-%s%s\n", cp, mode);
	}
	fclose(fp);
	fflush(fp_profile);
}

static void ccs_test_init(void)
{
	int fd = open(proc_policy_self_domain, O_RDONLY);
	memset(self_domain, 0, sizeof(self_domain));
	read(fd, self_domain, sizeof(self_domain) - 1);
	close(fd);
	errno = 0;
	fp_profile = fopen(proc_policy_profile, "w");
	fp_domain = fopen(proc_policy_domain_policy, "w");
	fp_exception = fopen(proc_policy_exception_policy, "w");
	if (!fp_domain || !fp_exception || !fp_profile) {
		if (errno != ENOENT)
			fprintf(stderr, "Please run \n"
				"# echo 255-MAC_FOR_FILE=disabled | "
				"/usr/sbin/ccs-loadpolicy -p\n");
		else
			fprintf(stderr, "You can't use this program "
				"for this kernel.\n");
		exit(1);
	}
	if (fwrite("\n", 1, 1, fp_profile) != 1 || fflush(fp_profile)) {
		memset(self_domain, 0, sizeof(self_domain));
		readlink("/proc/self/exe", self_domain,
			 sizeof(self_domain) - 1);
		if (self_domain[0] != '/')
			snprintf(self_domain, sizeof(self_domain) - 1,
				 "path_to_this_program");
		fprintf(stderr, "Please do either\n"
			"(a) run\n"
			"    # echo ");
		fprintf_encoded(stderr, self_domain);
		fprintf(stderr, " >> /etc/tomoyo/manager.conf\n"
			"    and reboot\n"
			"or\n"
			"(b) run\n"
			"    # echo ");
		fprintf_encoded(stderr, self_domain);
		fprintf(stderr, " | /usr/sbin/ccs-loadpolicy -m\n"
			"before running this program.\n");
		exit(1);
	}
	clear_status();
	fprintf(fp_domain, "%s\nuse_profile 255\n", self_domain);
	fflush(fp_domain);
	write_profile("255-TOMOYO_VERBOSE=enabled\n");
}
