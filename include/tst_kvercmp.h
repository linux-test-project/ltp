/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2009-2016 Cyril Hrubis chrubis@suse.cz
 * Copyright (c) Linux Test Project, 2020-2025
 */

#ifndef TST_KVERCMP_H__
#define TST_KVERCMP_H__

/**
 * tst_kvcmp() - Compare given kernel version with kernel in string.
 *
 * @cur_kver: Kernel version string (struct utsname.release).
 * @r1: Major kernel version.
 * @r2: Minor kernel version.
 * @r3: Kernel patch level.
 *
 * Everything after first three version numbers till the end of the string is
 * ignored.
 *
 * The same as tst_kvercmp() but running kernel version is passed as parameter
 * instead of utilizing uname().
 *
 * Return: Negative if older, 0 if the same and positive if newer.
 */
int tst_kvcmp(const char *cur_kver, int r1, int r2, int r3);

/**
 * tst_parse_kver() - Parses a version string into three integers.
 *
 * @str_kver: Kernel version string (struct utsname.release).
 * @v1: Major kernel version.
 * @v2: Minor kernel version.
 * @v3: Kernel patch level.
 *
 * Everything after first three version numbers till the end of the string is
 * ignored.
 *
 * Return: 0 on success, 1 on error.
 */
int tst_parse_kver(const char *str_kver, int *v1, int *v2, int *v3);

/**
 * tst_kvcmp_distname() - Get the distribution name from kernel version string.
 *
 * @cur_kver: Kernel version string (struct utsname.release).
 *
 * Return: The distribution name parsed from kernel version string or NULL.
 */
const char *tst_kvcmp_distname(const char *cur_kver);

/**
 * tst_kvexcmp() - Compares versions up to five version numbers long.
 * @tst_exv: The tested kernel version string (struct utsname.release).
 * @cur_kver: The current version in string (struct utsname.release).
 *
 * The return value is similar to the :manpage:`strcmp(3)` function, i.e. zero means
 * equal, negative value means that the kernel is older than the expected value
 * and positive means that it's newer.
 *
 * Return: negative if older, 0 if the same and positive if newer.
 */
int tst_kvexcmp(const char *tst_exv, const char *cur_kver);

/**
 * tst_kvercmp() - Compare a kernel version against currently running kernel.
 *
 * @r1: Major kernel version.
 * @r2: Minor kernel version.
 * @r3: Kernel patch level.
 *
 * Parse the output from :manpage:`uname(2)` and compare it to the passed values.
 * This is shortcut for calling tst_kvcmp() with ``uname -r`` as str_kver.
 *
 * Return: Negative if older, 0 if the same and positive if newer.
 */
int tst_kvercmp(int r1, int r2, int r3);

/**
 * struct tst_kern_exv - describe vendor kernel.
 *
 * @dist_name: A distribution name, e.g. "SLES", "RHEL9", "UBUNTU".
 * @extra_ver: A vendor kernel version to check, e.g. "5.14.0-441".
 */
struct tst_kern_exv {
	char *dist_name;
	char *extra_ver;
};

/**
 * tst_kvercmp2() - Compare given *distro* kernel version with the currently running kernel.
 *
 * @r1: Major kernel version.
 * @r2: Minor kernel version.
 * @r3: Kernel patch level.
 * @vers: A {} terminated array of :ref:`struct tst_kern_exv`.
 *
 * Attempts to look up a distro specific kernel version from the struct
 * tst_kern_exv table first and if no match is found falls back to the version
 * passed in r1, r2, r3 (see tst_kvercmp()).
 *
 * The distribution name is detected either from the kernel release string e.g.
 * el9 is mapped to RHEL9 or as a capitalized value of the ``ID=`` variable from
 * ``/etc/os-release``.
 *
 * Return: Negative if older, 0 if the same and positive if newer.
 */
int tst_kvercmp2(int r1, int r2, int r3, struct tst_kern_exv *vers);

#endif	/* TST_KVERCMP_H__ */
