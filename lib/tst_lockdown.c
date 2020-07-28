// SPDX-License-Identifier: GPL-2.0-or-later

#define TST_NO_DEFAULT_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>

#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_safe_stdio.h"
#include "tst_lockdown.h"

int tst_lockdown_enabled(void)
{
	char line[BUFSIZ];
	FILE *file;

	if (access(PATH_LOCKDOWN, F_OK) != 0) {
		tst_res(TINFO, "Unable to determine system lockdown state");
		return 0;
	}

	file = SAFE_FOPEN(PATH_LOCKDOWN, "r");
	if (!fgets(line, sizeof(line), file))
		tst_brk(TBROK | TERRNO, "fgets %s", PATH_LOCKDOWN);
	SAFE_FCLOSE(file);

	return (strstr(line, "[none]") == NULL);
}
