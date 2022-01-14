// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tst_kconfig.h"

int main(int argc, char *argv[])
{
	char *str = argv[1];
	char *delim = argv[2];
	unsigned int i, cnt = 1, ret = 0;

	switch(argc) {
	case 2:
		delim = ",";
	break;
	case 3:
		if (strlen(delim) > 1) {
			fprintf(stderr, "The delim must be a single character\n");
			return 1;
		}
	break;
	default:
		fprintf(stderr, "Please provide kernel kconfig list and delim "
				"(optinal, default value is ',')\n");
		return 1;
	}

	for (i = 0; str[i]; i++) {
		if (str[i] == delim[0])
			cnt++;
	}

	char **kconfigs = malloc(++cnt * sizeof(char *));
	if (!kconfigs) {
		fprintf(stderr, "malloc failed\n");
		return 1;
	}

	for (i = 0; i < cnt; i++)
		kconfigs[i] = strtok_r(str, delim, &str);

	if (tst_kconfig_check((const char * const*)kconfigs))
		ret = 1;

	free(kconfigs);
	return ret;
}
