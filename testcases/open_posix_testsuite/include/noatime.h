/*
 * Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

#ifdef __linux__

#include <mntent.h>

/*
 * Returns if prefix is prefix of a string and the lenght of prefix.
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

	/* returns lenght of prefix */
	return i;
}

#endif /* __linux__ */

/*
 * Scans through mounted filesystems and check for longest prefix
 * contained in path.
 */
int mounted_noatime(const char *path)
{
#ifdef __linux__
	struct mntent *mnt;
	int prefix_max = 0, prefix;
	int has_noatime;
	FILE *f;
	
	f = setmntent("/proc/mounts", "r");	

	if (f == NULL) {
		printf("Couldn't mount /proc/mounts\n");
		return -1;
	}

	while ((mnt = getmntent(f))) {
		
		/* ignore all pseudo fs */
		if (mnt->mnt_fsname[0] != '/')
			continue;
		
		prefix = strpref(path, mnt->mnt_dir);

		if (prefix > prefix_max) {
			prefix_max = prefix;
			has_noatime = hasmntopt(mnt, "noatime") != NULL;
		}
	}

	return has_noatime;
#else
	return 0;
#endif /* __linux__ */
}
