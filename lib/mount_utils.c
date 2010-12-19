#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <mntent.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Return the block device for a given path. You must free the string after you
 * call this function.
 *
 * Returns NULL if the device isn't found, memory couldn't be allocated, or if
 * the `block device' isn't a real block device (e.g. nfs mounts, etc).
 */
char *
get_block_device(const char *path)
{

	char *mnt_dir = NULL, *mnt_fsname = NULL;
	char *resolved_path = NULL;
	FILE *mtab_f;
	int done = 0;
	struct mntent *entry;
	struct stat junk;

	if (path == NULL) {
		errno = EINVAL;
	} else if ((resolved_path = realpath(path, NULL)) != NULL &&
		   (mtab_f = setmntent("/etc/mtab", "r")) != NULL)
	{

		do {

			entry = getmntent(mtab_f);

			if (entry != NULL) {

				if (!strncmp(entry->mnt_dir, resolved_path, strlen(entry->mnt_dir))) {

					char copy_string = 0;

					if (mnt_dir == NULL) {

						mnt_dir = malloc(strlen(entry->mnt_dir)+1);
						mnt_fsname = malloc(strlen(entry->mnt_fsname)+1);

						copy_string = mnt_dir != NULL && mnt_fsname != NULL;

					} else {

						if (!strncmp(entry->mnt_dir, mnt_dir, strlen(entry->mnt_dir))) {

							mnt_dir = realloc(mnt_dir, strlen(entry->mnt_dir));
							mnt_fsname = realloc(mnt_fsname, strlen(entry->mnt_fsname));
							copy_string = 1;

						}

					}

					if (copy_string != 0) {
						strcpy(mnt_dir, entry->mnt_dir);
						strcpy(mnt_fsname, entry->mnt_fsname);
#if DEBUG
						printf("%s is a subset of %s\n", path, entry->mnt_dir);
					} else {
						printf("%s is not a subset of %s\n", path, entry->mnt_dir);
#endif
					}

#if DEBUG
				} else {
					printf("%s is not a subset of %s\n", path, entry->mnt_dir);
#endif
				}

			}

		} while (done == 0 && entry != NULL);

		endmntent(mtab_f);

	}

	if (mnt_dir != NULL) {
		free(mnt_dir);
	}
	if (resolved_path != NULL) {
		free(resolved_path);
	}

	if (mnt_fsname != NULL && stat(mnt_fsname, &junk) < 0 &&
	    errno == ENOENT) {
		free(mnt_fsname);
		mnt_fsname = NULL;
		errno = ENODEV;
	}

	return mnt_fsname;

}

/*
 * Return the mountpoint for a given path. You must free the string after you
 * call this function.
 *
 * Returns NULL if memory couldn't be allocated.
 */
char *
get_mountpoint(const char *path)
{

	char *mnt_dir = NULL;
	char *resolved_path = NULL;
	FILE *mtab_f;
	int done = 0;
	struct mntent *entry;

	if (path == NULL) {
		errno = EINVAL;
	} else if ((resolved_path = realpath(path, NULL)) != NULL &&
		   (mtab_f = setmntent("/etc/mtab", "r")) != NULL)
	{

		do {

			entry = getmntent(mtab_f);

			if (entry != NULL) {

				if (!strncmp(entry->mnt_dir, resolved_path, strlen(entry->mnt_dir))) {

					char copy_string = 0;

					if (mnt_dir == NULL) {

						mnt_dir = malloc(strlen(entry->mnt_dir)+1);
						copy_string = mnt_dir != NULL;

					} else {

						if (!strncmp(entry->mnt_dir, mnt_dir, strlen(entry->mnt_dir))) {

							mnt_dir = realloc(mnt_dir, strlen(entry->mnt_dir));
							copy_string = 1;

						}

					}

					if (copy_string != 0) {
						strcpy(mnt_dir, entry->mnt_dir);
#if DEBUG
						printf("%s is a subset of %s\n", path, entry->mnt_dir);
					} else {
						printf("%s is not a subset of %s\n", path, entry->mnt_dir);
#endif
					}

#if DEBUG
				} else {
					printf("%s is not a subset of %s\n", path, entry->mnt_dir);
#endif
				}

			}

		} while (done == 0 && entry != NULL);

		endmntent(mtab_f);

	}

	if (resolved_path != NULL) {
		free(resolved_path);
	}

	return mnt_dir;

}
#if UNIT_TEST
int
main(void)
{

	char *mnt_fsname;
	char *paths[] = {
		"/home",
		"/mnt",
		"/tmp/foo", /* mkdir /tmp/foo; mount -t tmpfs none /tmp/foo/ */
		"/proc",
		"/optimus/store"
	};
	int i;

	for (i = 0; i < sizeof(paths) / sizeof(*paths); i++) {
		mnt_fsname = get_block_device(paths[i]);
		printf("%s - %s\n", paths[i], mnt_fsname);
		if (mnt_fsname != NULL) {
			free(mnt_fsname);
			mnt_fsname = NULL;
		}
	}

	return 0;

}
#endif