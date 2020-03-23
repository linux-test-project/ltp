/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2009-2016 Cyril Hrubis chrubis@suse.cz
 */

#ifndef TST_KVERCMP_H__
#define TST_KVERCMP_H__

/*
 * The same as tst_kvercmp() but running kernel version is passed as parameter
 * instead of utilizing uname().
 */
int tst_kvcmp(const char *cur_kver, int r1, int r2, int r3);

/*
 * Parsers string into three integer version.
 */
int tst_parse_kver(const char *str_kver, int *v1, int *v2, int *v3);

/*
 * Returns distribution name parsed from kernel version string or NULL.
 */
const char *tst_kvcmp_distname(const char *cur_kver);

/*
 * Compares versions up to five version numbers long.
 */
int tst_kvexcmp(const char *tst_exv, const char *cur_kver);

/*
 * Compare given kernel version with currently running kernel.
 *
 * Returns negative if older, 0 if the smame and possitive if newer.
 */
int tst_kvercmp(int r1, int r2, int r3);

struct tst_kern_exv {
	char *dist_name;
	char *extra_ver;
};

int tst_kvercmp2(int r1, int r2, int r3, struct tst_kern_exv *vers);

#endif	/* TST_KVERCMP_H__ */
