// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Red Hat, Inc.
 */

#ifndef TST_HUGEPAGE__
#define TST_HUGEPAGE__

#include "tst_path_defs.h"

extern char *nr_opt; /* -s num   Set the number of the been allocated hugepages */
extern char *Hopt;   /* -H /..   Location of hugetlbfs, i.e.  -H /var/hugetlbfs */

/**
 * enum tst_hp_policy - Hugepage reservation policy.
 * @TST_REQUEST: Try to reserve hugepages; tst_hugepages may be 0.
 * @TST_NEEDS: Fail with TCONF if the requested count cannot be reserved.
 */
enum tst_hp_policy {
	TST_REQUEST,
	TST_NEEDS,
};

#define TST_NO_HUGEPAGES ((unsigned long)-1)

/**
 * struct tst_hugepage - Hugepage reservation request.
 * @number: Number of hugepages to reserve.
 * @policy: Reservation policy (TST_REQUEST or TST_NEEDS).
 */
struct tst_hugepage {
	const unsigned long number;
	enum  tst_hp_policy policy;
};

/**
 * tst_get_hugepage_size() - Get the default hugepage size.
 *
 * Return: Hugepage size in bytes, or 0 if hugepages are not supported.
 */
size_t tst_get_hugepage_size(void);

/**
 * tst_reserve_hugepages() - Reserve hugepages from the system.
 * @hp: Hugepage request describing count and policy.
 *
 * Stores the number of actually reserved hugepages in tst_hugepages.
 * The result depends on system memory fragmentation.
 *
 * Return: Number of hugepages reserved.
 */
unsigned long tst_reserve_hugepages(struct tst_hugepage *hp);

/*
 * tst_hugepages - Number of hugepages actually reserved.
 *
 * Set by tst_reserve_hugepages(). Equals the requested count on success,
 * or fewer if the system could not provide enough. Zero when hugepages
 * are not supported.
 */
extern unsigned long tst_hugepages;

#endif /* TST_HUGEPAGE_H */
