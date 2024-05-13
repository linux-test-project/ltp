// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright Red Hat
 * Author: Filippo Storniolo <fstornio@redhat.com>
 */

/*\
 * [Description]
 *
 * Verify mlock() causes pre-faulting of PTEs and prevent memory to be swapped out.
 *
 * Find the new mapping in /proc/$pid/smaps and check Rss and Locked fields after
 * mlock syscall:
 * Rss and Locked size should be equal to the size of the memory allocation
 */

#include "tst_test.h"
#include "tst_safe_stdio.h"

#define MMAPLEN			(1UL<<20)
#define LINELEN			256

static void get_proc_smaps_info(unsigned long desired_mapping_address, unsigned long *Rss, unsigned long *Locked)
{
	bool mapping_found = false;
	bool Locked_found = false;
	bool Rss_found = false;
	char buffer[LINELEN];
	FILE *fp;
	int ret;

	fp = SAFE_FOPEN("/proc/self/smaps", "r");

	while (fgets(buffer, LINELEN, fp) != NULL) {
		unsigned long mapping_address;

		ret = sscanf(buffer, "%lx[^-]", &mapping_address);
		if ((ret == 1) && (mapping_address == desired_mapping_address)) {
			mapping_found = true;
			break;
		}
	}

	if (!mapping_found) {
		SAFE_FCLOSE(fp);
		tst_brk(TBROK, "Mapping %lx not found in /proc/self/smaps", desired_mapping_address);
		return;
	}

	while (fgets(buffer, LINELEN, fp) != NULL) {
		unsigned long possible_starting_mapping;
		unsigned long possible_ending_mapping;

		ret = sscanf(buffer, "%lx-%lx", &possible_starting_mapping, &possible_ending_mapping);
		if (ret == 2)
			break;

		if (strncmp(buffer, "Rss", strlen("Rss")) == 0) {
			ret = sscanf(buffer, "%*[^:]:%lu kB", Rss);
			if (ret != 1) {
				SAFE_FCLOSE(fp);
				tst_brk(TBROK, "failure occurred while reading field Rss");
				return;
			}

			Rss_found = true;
		}

		if (strncmp(buffer, "Locked", strlen("Locked")) == 0) {
			ret = sscanf(buffer, "%*[^:]:%lu kB", Locked);
			if (ret != 1) {
				SAFE_FCLOSE(fp);
				tst_brk(TBROK, "failure occurred while reading field Locked");
				return;
			}

			Locked_found =  true;
		}

		if (Rss_found && Locked_found) {
			SAFE_FCLOSE(fp);
			return;
		}
	}

	SAFE_FCLOSE(fp);
	tst_brk(TBROK, "cannot find both Rss and Locked in mapping %lx", desired_mapping_address);
}

static void verify_mlock(void)
{
	unsigned long Locked;
	unsigned long Rss;
	char *buf;

	buf = SAFE_MMAP(NULL, MMAPLEN, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	SAFE_MLOCK(buf, MMAPLEN);

	get_proc_smaps_info((unsigned long)buf, &Rss, &Locked);

	// Convertion from KiB to B
	Rss *= 1024;
	Locked *= 1024;

	TST_EXP_EQ_LU(Rss, MMAPLEN);
	TST_EXP_EQ_LU(Locked, MMAPLEN);

	SAFE_MUNLOCK(buf, MMAPLEN);
	SAFE_MUNMAP(buf, MMAPLEN);
}

static struct tst_test test = {
	.test_all = verify_mlock,
};
