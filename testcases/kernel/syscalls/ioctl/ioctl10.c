// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Wei Gao <wegao@suse.com>
 */

/*\
 * Test PROCMAP_QUERY ioctl() for /proc/$PID/maps.
 *
 * Test based on :kselftest:`proc/proc-pid-vm.c`.
 *
 * - ioctl with exact match query_addr
 * - ioctl without match query_addr
 * - check COVERING_OR_NEXT_VMA query_flags
 * - check PROCMAP_QUERY_VMA_WRITABLE query_flags
 * - check vma_name_addr content
 */

#include "config.h"
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fnmatch.h>
#include "tst_test.h"
#include "tst_safe_stdio.h"
#include <sys/sysmacros.h>
#include <linux/fs.h>
#include "lapi/ioctl.h"

#define PROC_MAP_PATH "/proc/self/maps"

struct map_entry {
	unsigned long vm_start;
	unsigned long vm_end;
	char vm_flags_str[5];
	unsigned long vm_pgoff;
	unsigned int vm_major;
	unsigned int vm_minor;
	unsigned long vm_inode;
	char vm_name[256];
	unsigned int vm_flags;
};

struct procmap_query *q;
static int fd = -1;

static unsigned int parse_vm_flags(const char *vm_flags_str)
{
	unsigned int flags = 0;

	if (strchr(vm_flags_str, 'r'))
		flags |= PROCMAP_QUERY_VMA_READABLE;
	if (strchr(vm_flags_str, 'w'))
		flags |= PROCMAP_QUERY_VMA_WRITABLE;
	if (strchr(vm_flags_str, 'x'))
		flags |= PROCMAP_QUERY_VMA_EXECUTABLE;
	if (strchr(vm_flags_str, 's'))
		flags |= PROCMAP_QUERY_VMA_SHARED;

	return flags;

}

static void parse_maps_file(const char *filename, const char *keyword, struct map_entry *entry)
{
	FILE *fp = SAFE_FOPEN(filename, "r");

	char line[1024];

	while (fgets(line, sizeof(line), fp) != NULL) {
		if (fnmatch(keyword, line, 0) == 0) {
			if (sscanf(line, "%lx-%lx %s %lx %x:%x %lu %s",
						&entry->vm_start, &entry->vm_end, entry->vm_flags_str,
						&entry->vm_pgoff, &entry->vm_major, &entry->vm_minor,
						&entry->vm_inode, entry->vm_name) < 7)
				tst_brk(TFAIL, "parse maps file /proc/self/maps failed");

			entry->vm_flags = parse_vm_flags(entry->vm_flags_str);

			SAFE_FCLOSE(fp);
			return;
		}
	}

	SAFE_FCLOSE(fp);
	tst_brk(TFAIL, "parse maps file /proc/self/maps failed");
}

static void verify_ioctl(void)
{
	struct map_entry entry;

	memset(&entry, 0, sizeof(entry));

	parse_maps_file(PROC_MAP_PATH, "*", &entry);

	/* CASE 1: exact MATCH at query_addr */
	memset(q, 0, sizeof(*q));
	q->size = sizeof(*q);
	q->query_addr = (uint64_t)entry.vm_start;
	q->query_flags = 0;

	TST_EXP_PASS(ioctl(fd, PROCMAP_QUERY, q));

	TST_EXP_EQ_LU(q->query_addr, entry.vm_start);
	TST_EXP_EQ_LU(q->query_flags, 0);
	TST_EXP_EQ_LU(q->vma_flags, entry.vm_flags);
	TST_EXP_EQ_LU(q->vma_start, entry.vm_start);
	TST_EXP_EQ_LU(q->vma_end, entry.vm_end);
	TST_EXP_EQ_LU(q->vma_page_size, getpagesize());
	TST_EXP_EQ_LU(q->vma_offset, entry.vm_pgoff);
	TST_EXP_EQ_LU(q->inode, entry.vm_inode);
	TST_EXP_EQ_LU(q->dev_major, entry.vm_major);
	TST_EXP_EQ_LU(q->dev_minor, entry.vm_minor);

	/* CASE 2: NO MATCH at query_addr */
	memset(q, 0, sizeof(*q));
	q->size = sizeof(*q);
	q->query_addr = entry.vm_start - 1;
	q->query_flags = 0;

	TST_EXP_FAIL(ioctl(fd, PROCMAP_QUERY, q), ENOENT);

	/* CASE 3: MATCH COVERING_OR_NEXT_VMA */
	memset(q, 0, sizeof(*q));
	q->size = sizeof(*q);
	q->query_addr = entry.vm_start - 1;
	q->query_flags = PROCMAP_QUERY_COVERING_OR_NEXT_VMA;

	TST_EXP_PASS(ioctl(fd, PROCMAP_QUERY, q));

	/* CASE 4: NO MATCH WRITABLE at query_addr */
	memset(&entry, 0, sizeof(entry));
	parse_maps_file(PROC_MAP_PATH, "*r-?p *", &entry);

	memset(q, 0, sizeof(*q));
	q->size = sizeof(*q);
	q->query_addr = entry.vm_start;
	q->query_flags = PROCMAP_QUERY_VMA_WRITABLE;
	TST_EXP_FAIL(ioctl(fd, PROCMAP_QUERY, q), ENOENT);

	/* CASE 5: check vma_name_addr content */
	char process_name[256];
	char pattern[258];
	char buf[256];

	SAFE_READLINK("/proc/self/exe", process_name, sizeof(process_name));
	snprintf(pattern, sizeof(pattern), "*%s*", process_name);
	memset(&entry, 0, sizeof(entry));
	parse_maps_file(PROC_MAP_PATH, pattern, &entry);

	memset(q, 0, sizeof(*q));
	q->size = sizeof(*q);
	q->query_addr = entry.vm_start;
	q->query_flags = 0;
	q->vma_name_addr = (uint64_t)(unsigned long)buf;
	q->vma_name_size = sizeof(buf);

	TST_EXP_PASS(ioctl(fd, PROCMAP_QUERY, q));
	TST_EXP_EQ_LU(q->vma_name_size, strlen(process_name) + 1);
	TST_EXP_EQ_STR((char *)q->vma_name_addr, process_name);

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	struct procmap_query q = {};

	fd = SAFE_OPEN(PROC_MAP_PATH, O_RDONLY);

	if (tst_kvercmp(6, 11, 0) < 0) {
		TEST(ioctl(fd, PROCMAP_QUERY, q));

		if ((TST_RET == -1) && (TST_ERR == ENOTTY))
			tst_brk(TCONF,
				"This system does not provide support for ioctl(PROCMAP_QUERY)");
	}
}

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_ioctl,
	.bufs = (struct tst_buffers []) {
		{&q, .size = sizeof(*q)},
		{}
	}
};
