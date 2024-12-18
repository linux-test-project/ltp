/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2017  Red Hat, Inc.
 */

/*
 * Parse the ksm0* test options in funcion parse_ksm_options().
 */

#ifndef KSM_COMMON_H__
#define KSM_COMMON_H__

#include "tst_test.h"
#include "ksm_helper.h"
#include "numa_helper.h"
#include "ksm_test.h"

#define DEFAULT_MEMSIZE 128

static int size = DEFAULT_MEMSIZE, num = 3, unit = 1;
static char *opt_sizestr, *opt_numstr, *opt_unitstr;

static inline void parse_ksm_options(char *str_size, int *size,
		char *str_num, int *num, char *str_unit, int *unit)
{
	if (tst_parse_int(str_size, size, 1, INT_MAX))
		tst_brk(TBROK, "Invalid size '%s'", str_size);

	if (tst_parse_int(str_num, num, 3, INT_MAX))
		tst_brk(TBROK, "Invalid num '%s'", str_num);

	if (tst_parse_int(str_unit, unit, 1, *size))
		tst_brk(TBROK, "Invalid unit '%s'", str_unit);

	if (*size % *unit != 0)
		tst_brk(TBROK, "the remainder of division of size by unit is not zero.");
}

/* Warning: *DO NOT* use this function in child */
static inline unsigned int get_a_numa_node(void)
{
	unsigned int nd1, nd2;
	int ret;

	ret = get_allowed_nodes(0, 2, &nd1, &nd2);
	switch (ret) {
	case 0:
		break;
	case -3:
		tst_brk(TCONF, "requires a NUMA system.");
	default:
		tst_brk(TBROK | TERRNO, "1st get_allowed_nodes");
	}

	ret = get_allowed_nodes(NH_MEMS | NH_CPUS, 1, &nd1);
	switch (ret) {
	case 0:
		tst_res(TINFO, "get node%u.", nd1);
		return nd1;
	case -3:
		tst_brk(TCONF, "requires a NUMA system that has "
			 "at least one node with both memory and CPU "
			 "available.");
	default:
		tst_brk(TBROK | TERRNO, "2nd get_allowed_nodes");
	}

	/* not reached */
	abort();
}

#endif /* KSM_COMMON_H__ */
