/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2021 Li Wang <liwang@redhat.com>
 */

#include <string.h>
#include <stdlib.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_arch.h"
#include "tst_test.h"

const struct tst_arch tst_arch = {
#if defined(__x86_64__)
        .name = "x86_64",
        .type = TST_X86_64,
#elif defined(__i386__) || defined(__i586__) || defined(__i686__)
        .name = "x86",
        .type = TST_X86,
#elif defined(__ia64__)
        .name = "ia64",
        .type = TST_IA64,
#elif defined(__powerpc64__) || defined(__ppc64__)
        .name = "ppc64",
        .type = TST_PPC64,
#elif defined(__powerpc__) || defined(__ppc__)
        .name = "ppc",
        .type = TST_PPC,
#elif defined(__s390x__)
        .name = "s390x",
        .type = TST_S390X,
#elif defined(__s390__)
        .name = "s390",
        .type = TST_S390,
#elif defined(__aarch64__)
        .name = "aarch64",
        .type = TST_AARCH64,
#elif defined(__arm__)
        .name = "arm",
        .type = TST_ARM,
#elif defined(__sparc__)
        .name = "sparc",
        .type = TST_SPARC,
#else
        .name = "unknown",
        .type = TST_UNKNOWN,
#endif
};

static const char *const arch_type_list[] = {
	"x86",
	"x86_64",
	"ia64",
	"ppc",
	"ppc64",
	"s390",
	"s390x",
	"arm",
	"aarch64",
	"sparc",
	NULL
};

static int is_valid_arch_name(const char *name)
{
	unsigned int i;

	for (i = 0; arch_type_list[i]; i++) {
		if (!strcmp(arch_type_list[i], name))
			return 1;
	}

	return 0;
}

int tst_is_on_arch(const char *const *archlist)
{
	unsigned int i;

	if (!archlist)
		return 1;

	for (i = 0; archlist[i]; i++) {
		if (!is_valid_arch_name(archlist[i]))
			tst_brk(TBROK, "%s is invalid arch, please reset!",
					archlist[i]);
	}

	for (i = 0; archlist[i]; i++) {
		if (!strcmp(tst_arch.name, archlist[i]))
			return 1;
	}

	return 0;
}
