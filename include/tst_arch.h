/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2021 Li Wang <liwang@redhat.com>
 */

#ifndef TST_ARCH_H__
#define TST_ARCH_H__

enum tst_arch_type {
	TST_UNKNOWN,
	TST_X86,
	TST_X86_64,
	TST_IA64,
	TST_PPC,
	TST_PPC64,
	TST_S390,
	TST_S390X,
	TST_ARM,
	TST_AARCH64,
	TST_SPARC,
};

/*
 * This tst_arch is to save the system architecture for
 * using in the whole testcase.
 */
extern const struct tst_arch {
        char name[16];
        enum tst_arch_type type;
} tst_arch;

/*
 * Check if test platform is in the given arch list. If yes return 1,
 * otherwise return 0.
 *
 * @archlist A NULL terminated array of architectures to support.
 */
int tst_is_on_arch(const char *const *archlist);

#endif /* TST_ARCH_H__ */
