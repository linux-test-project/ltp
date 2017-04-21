/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2009-2016 Cyril Hrubis chrubis@suse.cz
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
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
