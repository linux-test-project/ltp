/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef PROCESS_MADVISE_H__
#define PROCESS_MADVISE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tst_safe_stdio.h"

struct addr_mapping {
	int size;
	int rss;
	int pss;
	int shared_clean;
	int shared_dirty;
	int private_clean;
	int private_dirty;
	int referenced;
	int anonymous;
	int anon_huge_pages;
	int shmem_huge_pages;
	int shmem_pmd_mapped;
	int swap;
	int kernel_page_size;
	int mmu_page_size;
	int locked;
	int protection_key;
};

static inline void read_address_mapping(unsigned long address, struct addr_mapping *mapping)
{
	FILE *f;
	int found = 0;
	char label[BUFSIZ];
	char line[BUFSIZ];
	char smaps[BUFSIZ];
	char ptr_str[BUFSIZ];
	int value;

	snprintf(smaps, BUFSIZ, "/proc/%i/smaps", getpid());
	snprintf(ptr_str, BUFSIZ, "%lx", address);

	f = SAFE_FOPEN(smaps, "r");

	while (fgets(line, BUFSIZ, f) != NULL) {
		if (strncmp(ptr_str, line, strlen(ptr_str)) == 0)
			found = 1;

		if (!found)
			continue;

		if (found && strcmp(line, "VmFlags") >= 0)
			break;

		if (sscanf(line, "%31[^:]: %d", label, &value) > 0) {
			if (strcmp(label, "Size") == 0)
				mapping->size = value;
			else if (strcmp(label, "Rss") == 0)
				mapping->rss = value;
			else if (strcmp(label, "Pss") == 0)
				mapping->pss = value;
			else if (strcmp(label, "Shared_Clean") == 0)
				mapping->shared_clean = value;
			else if (strcmp(label, "Shared_Dirty") == 0)
				mapping->shared_dirty = value;
			else if (strcmp(label, "Private_Clean") == 0)
				mapping->private_clean = value;
			else if (strcmp(label, "Private_Dirty") == 0)
				mapping->private_dirty = value;
			else if (strcmp(label, "Referenced") == 0)
				mapping->referenced = value;
			else if (strcmp(label, "Anonymous") == 0)
				mapping->anonymous = value;
			else if (strcmp(label, "AnonHugePages") == 0)
				mapping->anon_huge_pages = value;
			else if (strcmp(label, "ShmemHugePages") == 0)
				mapping->shmem_huge_pages = value;
			else if (strcmp(label, "ShmemPmdMapped") == 0)
				mapping->shmem_pmd_mapped = value;
			else if (strcmp(label, "Swap") == 0)
				mapping->swap = value;
			else if (strcmp(label, "KernelPageSize") == 0)
				mapping->kernel_page_size = value;
			else if (strcmp(label, "MMUPageSize") == 0)
				mapping->mmu_page_size = value;
			else if (strcmp(label, "Locked") == 0)
				mapping->locked = value;
			else if (strcmp(label, "ProtectionKey") == 0)
				mapping->protection_key = value;
		}
	}

	SAFE_FCLOSE(f);
}

#endif
