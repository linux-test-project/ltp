// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Red Hat, Inc.
 */

#ifndef TST_HUGEPAGE__
#define TST_HUGEPAGE__

#define PATH_HUGEPAGES	"/sys/kernel/mm/hugepages/"
#define PATH_NR_HPAGES	"/proc/sys/vm/nr_hugepages"

extern char *nr_opt; /* -s num   Set the number of the been allocated hugepages */
extern char *Hopt;   /* -H /..   Location of hugetlbfs, i.e.  -H /var/hugetlbfs */

enum tst_hp_policy {
	TST_REQUEST,
	TST_NEEDS,
};

struct tst_hugepage {
	const unsigned long number;
	enum  tst_hp_policy policy;
};

/*
 * Get the default hugepage size. Returns 0 if hugepages are not supported.
 */
size_t tst_get_hugepage_size(void);

/*
 * Try the best to request a specified number of huge pages from system,
 * it will store the reserved hpage number in tst_hugepages.
 *
 * Note: this depend on the status of system memory fragmentation.
 */
unsigned long tst_reserve_hugepages(struct tst_hugepage *hp);

/*
 * This variable is used for recording the number of hugepages which system can
 * provides. It will be equal to 'hpages' if tst_reserve_hugepages on success,
 * otherwise set it to a number of hugepages that we were able to reserve.
 *
 * If system does not support hugetlb, then it will be set to 0.
 */
extern unsigned long tst_hugepages;

#endif /* TST_HUGEPAGE_H */
