/*
 * Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

#ifdef __linux__
#include <mntent.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/param.h>
#include <sys/mount.h>
#include <errno.h>
#include <string.h>
#endif
#include <stdio.h>
#include "posixtest.h"

#ifdef __linux__

/*
 * Returns if prefix is prefix of a string and the length of prefix.
 */
int strpref(const char *str, const char *pref)
{
	int i;

	for (i = 0; pref[i] != '\0'; i++) {
		/* string ended too soon */
		if (str[i] == 0)
			return -1;

		/* string is diferent */
		if (str[i] != pref[i])
			return -1;
	}

	/* returns length of prefix */
	return i;
}

/*
 * Scans through mounted filesystems and check for longest prefix
 * contained in path.
 */
int mounted_noatime(const char *path)
{
	struct mntent *mnt;
	int prefix_max = 0, prefix;
	int has_noatime = 0;
	FILE *f;

	f = setmntent("/proc/mounts", "r");

	if (f == NULL) {
		printf("Couldn't mount /proc/mounts\n");
		return -1;
	}

	while ((mnt = getmntent(f))) {
		/* ignore duplicit record for root fs */
		if (!strcmp(mnt->mnt_fsname, "rootfs"))
			continue;

		prefix = strpref(path, mnt->mnt_dir);

		if (prefix > prefix_max) {
			prefix_max = prefix;
			has_noatime = hasmntopt(mnt, "noatime") != NULL;
		}
	}

	return has_noatime;
}
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
int mounted_noatime(const char *path)
{
	struct statfs _statfs;

	if (statfs(path, &_statfs) == -1) {
		printf("statfs for %s failed: %s", path, strerror(errno));
		return -1;
	}

	return (_statfs.f_flags & MNT_NOATIME);
}
#else
int mounted_noatime(const char *path LTP_ATTRIBUTE_UNUSED)
{
	return 0;
}
#endif
