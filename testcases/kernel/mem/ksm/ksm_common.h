// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017  Red Hat, Inc.
 */

 /*
  * Parse the ksm0* test options in funcion parse_ksm_options().
  */

#include "tst_test.h"

#define DEFAULT_MEMSIZE 128

int size = DEFAULT_MEMSIZE, num = 3, unit = 1;
char *opt_sizestr, *opt_numstr, *opt_unitstr;

static inline void parse_ksm_options(char *str_size, int *size,
		char *str_num, int *num, char *str_unit, int *unit)
{
	if(tst_parse_int(str_size, size, 1, INT_MAX))
		tst_brk(TBROK, "Invalid size '%s'", str_size);

	if(tst_parse_int(str_num, num, 3, INT_MAX))
		tst_brk(TBROK, "Invalid num '%s'", str_num);

	if(tst_parse_int(str_unit, unit, 1, *size))
		tst_brk(TBROK, "Invalid unit '%s'", str_unit);
	if (*size % *unit != 0)
		tst_brk(TBROK,
				"the remainder of division of size by unit is "
				"not zero.");
}
