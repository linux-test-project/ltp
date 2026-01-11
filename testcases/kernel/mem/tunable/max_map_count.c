// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2012-2025
 * Copyright (C) 2012-2017  Red Hat, Inc.
 */

/*\
 * Test ``/proc/sys/vm/max_map_count`` tunable file.
 *
 * :kernel_doc:`admin-guide/sysctl/vm` claims:
 *
 *    /proc/sys/vm/max_map_count contains the maximum number of memory map
 *    areas a process may have. Memory map areas are used as a side-effect
 *    of calling malloc, directly by mmap and mprotect, and also when
 *    loading shared libraries.
 *
 * Each process has his own maps file: /proc/[pid]/maps, and each line
 * indicates a map entry, so it can caculate the amount of maps by reading
 * the file lines' number to check the tunable performance.
 *
 * The program tries to invoke :manpage:`mmap(2)` endlessly until it triggers
 * ``MAP_FAILED``, then reads the process's maps file /proc/[pid]/maps, save
 * the line number to map_count variable, and compare it with
 * ``/proc/sys/vm/max_map_count``, map_count should be greater than
 * max_map_count by 1.
 *
 * NOTE: On some architectures there is a special vma VSYSCALL, which
 * is allocated without incrementing mm->map_count variable. On these
 * architectures each /proc/<pid>/maps has at the end:
 *
 * ::
 *
 *    ...
 *    ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0   [vsyscall]
 *
 * Therefore this line is ignored during /proc/[pid]/maps reading.
 *
 * [References]
 *
 * - :kernel_doc:`admin-guide/sysctl/vm`
 */

#define _GNU_SOURCE
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include "tst_test.h"

#define MAP_COUNT_DEFAULT	1024
#define MAX_MAP_COUNT_MAX	65536L

#define OVERCOMMIT_MEMORY "/proc/sys/vm/overcommit_memory"
#define MAX_MAP_COUNT "/proc/sys/vm/max_map_count"

/* This is a filter to exclude map entries which aren't accounted
 * for in the vm_area_struct's map_count.
 */
static bool filter_map(const char *line)
{
	char buf[BUFSIZ];
	int ret;

	ret = sscanf(line, "%*p-%*p %*4s %*p %*2d:%*2d %*d %s", buf);
	if (ret != 1)
		return false;

	switch (tst_arch.type) {
	case TST_X86:
	case TST_X86_64:
		/* On x86, there's an old compat vsyscall page */
		if (!strcmp(buf, "[vsyscall]"))
			return true;
		break;
	case TST_IA64:
		/* On ia64, the vdso is not a proper mapping */
		if (!strcmp(buf, "[vdso]"))
			return true;
		break;
	case TST_ARM:
		/* Skip it when run it in aarch64 */
		if (tst_kernel_bits() == 64)
			return false;

		/* Older arm kernels didn't label their vdso maps */
		if (!strncmp(line, "ffff0000-ffff1000", 17))
			return true;
		break;
	default:
		break;
	};

	return false;
}

static long count_maps(pid_t pid)
{
	FILE *fp;
	size_t len;
	char *line = NULL;
	char buf[BUFSIZ];
	long map_count = 0;

	snprintf(buf, BUFSIZ, "/proc/%d/maps", pid);
	fp = fopen(buf, "r");
	if (fp == NULL)
		tst_brk(TBROK | TERRNO, "fopen %s", buf);
	while (getline(&line, &len, fp) != -1) {
		/* exclude vdso and vsyscall */
		if (filter_map(line))
			continue;
		map_count++;
	}
	fclose(fp);

	return map_count;
}

static void max_map_count_test(void)
{
	int status;
	pid_t pid;
	long max_maps;
	long map_count;
	long max_iters;
	long memfree;

	/*
	 * XXX Due to a possible kernel bug, oom-killer can be easily
	 * triggered when doing small piece mmaps in huge amount even if
	 * enough free memory available. Also it has been observed that
	 * oom-killer often kill wrong victims in this situation, we
	 * decided to do following steps to make sure no oom happen:
	 * 1) use a safe maximum max_map_count value as upper-bound,
	 *    we set it 65536 in this case, i.e., we don't test too big
	 *    value;
	 * 2) make sure total mapping isn't larger than
	 *        CommitLimit - Committed_AS
	 */
	memfree = SAFE_READ_MEMINFO("CommitLimit:") - SAFE_READ_MEMINFO("Committed_AS:");
	/* 64 used as a bias to make sure no overflow happen */
	max_iters = memfree / sysconf(_SC_PAGESIZE) * 1024 - 64;
	if (max_iters > MAX_MAP_COUNT_MAX)
		max_iters = MAX_MAP_COUNT_MAX;

	max_maps = MAP_COUNT_DEFAULT;
	if (max_iters < max_maps)
		tst_brk(TCONF, "test requires more free memory");

	while (max_maps <= max_iters) {
		TST_SYS_CONF_LONG_SET(MAX_MAP_COUNT, max_maps, 1);

		switch (pid = SAFE_FORK()) {
		case 0:
			while (mmap(NULL, 1, PROT_READ,
				    MAP_SHARED | MAP_ANONYMOUS, -1, 0)
			       != MAP_FAILED) ;
			if (raise(SIGSTOP) != 0)
				tst_brk(TBROK | TERRNO, "raise");
			exit(0);
		default:
			break;
		}
		/* wait child done mmap and stop */
		SAFE_WAITPID(pid, &status, WUNTRACED);
		if (!WIFSTOPPED(status))
			tst_brk(TBROK, "child did not stopped");

		map_count = count_maps(pid);
		/* Note max_maps will be exceeded by one for
		 * the sysctl setting of max_map_count. This
		 * is the mm failure point at the time of
		 * writing this COMMENT!
		*/
		if (map_count == (max_maps + 1))
			tst_res(TPASS, "%ld map entries in total "
				 "as expected.", max_maps);
		else
			tst_res(TFAIL, "%ld map entries in total, but "
				 "expected %ld entries", map_count, max_maps);

		/* make child continue to exit */
		SAFE_KILL(pid, SIGCONT);
		SAFE_WAITPID(pid, &status, 0);

		max_maps = max_maps << 1;
	}
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.test_all = max_map_count_test,
	.save_restore = (const struct tst_path_val[]) {
		{OVERCOMMIT_MEMORY, "0", TST_SR_TBROK},
		{MAX_MAP_COUNT, NULL, TST_SR_TBROK},
		{}
	},
};
