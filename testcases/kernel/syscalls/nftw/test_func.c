/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* 10/31/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001   Port to Linux   nsharoff@us.ibm.com */

/*
 * NAME
 *	test_func.c - nftw() calls these functions.
 */

#include "nftw.h"

extern pathdata pathdat[];
extern struct list mnem[], badlist[];
extern char *dirlist[NDIRLISTENTS];
extern const char *rw_fs_name;
extern int npathdats, ngoods, nbads, nmnem, visit, next_fd[4];
extern FILE *temp;

/*
 * Calling function should free the dirlist array.
 */
int
test_func1(const char *path_name, const struct stat *stat_pointer,
	   int ftw_integer, struct FTW *ftwp)
{
	char *s;
	const char *p;

	temp = stderr;

	if ((s = malloc((size_t)(strlen((char *)path_name) + 1)))
	    == NULL) {
		perror("malloc in test_func1");
		return 999;
	}

	if ((p = strstr(path_name, NFTW)) != NULL) {
		p += strlen(NFTW);
	} else {
		p = path_name;
	}

	(void)strcpy(s, p);
	dirlist[visit++] = s;

#ifdef DEBUG
	fprintf(temp, "INFO: Call to fn() at %s\n", path_name);
#endif

	if (visit >= NDIRLISTENTS) {
		fprintf(temp, "ERROR: Too many paths traversed\n");
		return 999;
	}
	return 0;
}

int
test_func3(const char *path_name, const struct stat *stat_pointer,
	   int ftw_integer, struct FTW *ftwp)
{
	visit++;
	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "INFO: Call to fn() at %s\n", path_name);
#endif

	if (visit >= NDIRLISTENTS) {
		fprintf(temp, "ERROR: Too many paths traversed\n");
		return 999;
	}

	if (strcmp(path_name, "./tmp/data/dirl/dir_right.1/dir_right.2/right.3")
	    == 0) {
		fprintf(temp,
			"ERROR: Target of right.3 was already reported so this file should not be\n");
		return 999;
	}

	return 0;
}

int
test_func4(const char *path_name, const struct stat *stat_pointer,
	   int ftw_integer, struct FTW *ftwp)
{
	visit++;
	do_info(path_name);

	/* Stop traversal once directory is visited. */
	if (strcmp(path_name, "./tmp/data/d777") == 0)
		return 999;
	return 0;
}

int
test_func5(const char *path_name, const struct stat *stat_pointer,
	   int ftw_integer, struct FTW *ftwp)
{
	char pathcwd[PATH_MAX];

	if (ftw_integer == FTW_D)
		return (0);

	if (getcwd(pathcwd, sizeof(pathcwd)) == NULL) {
		perror("getcwd");
		return 998;
	}

	if (strstr(path_name, pathcwd) == 0) {
		fprintf(temp, "ERROR: For file %s cwd is %s\n", path_name,
			pathcwd);
		return 999;
	}

	return (0);
}

int
test_func7(const char *path_name, const struct stat *stat_pointer,
	   int ftw_integer, struct FTW *ftwp)
{
	int i, found;
	const char *p;

	do_info(path_name);

	if ((p = strstr(path_name, NFTW)) != NULL) {
		p += strlen(NFTW);
	} else {
		p = path_name;
	}

	for (found = i = 0; i < nbads; i++) {
		if (strcmp(p, badlist[i].s) == 0) {
			found++;
			break;
		}
	}

	if (!found) {
		fprintf(temp, "ERROR: Should not have traversed %s\n",
			path_name);
		return 999;
	}
	return 0;
}

int
test_func8(const char *path_name, const struct stat *stat_pointer,
	   int ftw_integer, struct FTW *ftwp)
{
	int i;
	const char *p;
	struct stat st_buf;

	do_info(path_name);

	if ((p = strstr(path_name, NFTW)) != NULL) {
		p += strlen(NFTW);
	} else {
		p = path_name;
	}

	for (i = 0; i < nbads; i++) {
		if (ftw_integer == FTW_D || ftw_integer == FTW_F ||
		    ftw_integer == FTW_SL) {
			if ((((ftw_integer == FTW_D) || (ftw_integer ==
							 FTW_F)) ?
			     stat(path_name, &st_buf) : lstat(path_name,
							      &st_buf)) == -1) {
				perror("stat");
				return 999;
			}

			if (st_buf.st_dev != stat_pointer->st_dev) {
				fprintf(temp,
					"ERROR: st_dev members do not match for %s\n",
					path_name);
				return 999;
			}

			if (st_buf.st_ino != stat_pointer->st_ino) {
				fprintf(temp,
					"ERROR: st_ino members do not match for %s\n",
					path_name);
				return 999;
			}

			if (st_buf.st_mode != stat_pointer->st_mode) {
				fprintf(temp,
					"ERROR: st_mode members do not match for %s\n",
					path_name);
				return 999;
			}

			if (st_buf.st_nlink != stat_pointer->st_nlink) {
				fprintf(temp,
					"ERROR: st_nlink members d o not match for %s\n",
					path_name);
				return 999;
			}

			if (st_buf.st_uid != stat_pointer->st_uid) {
				fprintf(temp,
					"ERROR: st_uid members do not match for %s\n",
					path_name);
				return 999;
			}

			if (st_buf.st_gid != stat_pointer->st_gid) {
				fprintf(temp,
					"ERROR: st_gid members do not match for %s\n",
					path_name);
				return 999;
			}

			if (st_buf.st_size != stat_pointer->st_size) {
				fprintf(temp,
					"ERROR: st_size members do not match for %s\n",
					path_name);
				return 999;
			}
		}

	}
	return 0;
}

int
test_func9(const char *path_name, const struct stat *stat_pointer,
	   int ftw_integer, struct FTW *ftwp)
{
	int i;
	const char *p;

	do_info(path_name);

	if ((p = strstr(path_name, NFTW)) != NULL) {
		p += strlen(NFTW);
	} else {
		p = path_name;
	}

	for (i = 0; i < nbads; i++) {
		if (strcmp(p, badlist[i].s) == 0) {

			if (ftw_integer == FTW_F) {
				if (ftw_integer != badlist[i].i) {
					fprintf(temp,
						"ERROR: Bad thrid arg to fn () for %s\n",
						path_name);
					fprintf(temp, "       Expected %s\n",
						ftw_mnemonic(badlist[i].i));
					fprintf(temp, "       Received %s\n",
						ftw_mnemonic(ftw_integer));
					return 999;
				}
			}
		}
	}
	return 0;
}

int
test_func10(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	int i;
	const char *p;

	do_info(path_name);

	if ((p = strstr(path_name, NFTW)) != NULL) {
		p += strlen(NFTW);
	} else {
		p = path_name;
	}

	for (i = 0; i < nbads; i++) {
		if (strcmp(p, badlist[i].s) == 0) {
			if (ftw_integer == FTW_D) {
				if (ftw_integer != badlist[i].i) {
					fprintf(temp,
						"ERROR: Bad third arg to fn () for %s\n",
						path_name);
					fprintf(temp, "       Expected %s\n",
						ftw_mnemonic(badlist[i].i));
					fprintf(temp, "       Received %s\n",
						ftw_mnemonic(ftw_integer));
					return 999;
				}
			}
		}
	}
	return 0;
}

int
test_func11(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	int i;
	const char *p;

	do_info(path_name);

	if ((p = strstr(path_name, NFTW)) != NULL) {
		p += strlen(NFTW);
	} else {
		p = path_name;
	}

	for (i = 0; i < nbads; i++) {
		if (strcmp(p, badlist[i].s) == 0) {
			if (ftw_integer == FTW_DP) {
				if (ftw_integer != badlist[i].i) {
					fprintf(temp,
						"ERROR: Bad third arg to fn () for %s\n",
						path_name);
					fprintf(temp, "       Expected %s\n",
						ftw_mnemonic(badlist[i].i));
					fprintf(temp, "       Received %s\n",
						ftw_mnemonic(ftw_integer));
					return 999;
				}
			}
		}
	}
	return 0;
}

int
test_func12(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	int i;
	const char *p;

	do_info(path_name);

	if ((p = strstr(path_name, NFTW)) != NULL) {
		p += strlen(NFTW);
	} else {
		p = path_name;
	}

	for (i = 0; i < nbads; i++) {
		if (strcmp(p, badlist[i].s) == 0) {
			if (ftw_integer == FTW_SL) {
				if (ftw_integer != badlist[i].i) {
					fprintf(temp,
						"ERROR: Bad third arg to fn() for %s.  Expected %s, Received %s\n",
						path_name,
						ftw_mnemonic(badlist[i].i),
						ftw_mnemonic(ftw_integer));
					return 999;
				}
			}
		}
	}
	return 0;
}

int
test_func13(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	int i;
	const char *p;

	do_info(path_name);

	if ((p = strstr(path_name, NFTW)) != NULL) {
		p += strlen(NFTW);
	} else {
		p = path_name;
	}

	for (i = 0; i < nbads; i++) {
		if (strcmp(p, badlist[i].s) == 0) {

			if (ftw_integer == FTW_SLN) {
				if (ftw_integer != badlist[i].i) {
					fprintf(temp,
						"ERROR: Bad third arg to fn() for %s\n",
						path_name);
					fprintf(temp, "       Expected %s\n",
						ftw_mnemonic(badlist[i].i));
					fprintf(temp, "       Received %s\n",
						ftw_mnemonic(ftw_integer));
					return 999;
				}
			}
		}
	}
	return 0;
}

int
test_func14(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	int i;
	const char *p;

	do_info(path_name);

	if ((p = strstr(path_name, NFTW)) != NULL) {
		p += strlen(NFTW);
	} else {
		p = path_name;
	}

	for (i = 0; i < nbads; i++) {
		if (strcmp(p, badlist[i].s) == 0) {

			if (ftw_integer == FTW_DNR) {
				if (ftw_integer != badlist[i].i) {
					fprintf(temp,
						"ERROR: Bad third arg to fn() for %s\n",
						path_name);
					fprintf(temp, "       Expected %s\n",
						ftw_mnemonic(badlist[i].i));
					fprintf(temp, "       Received %s\n",
						ftw_mnemonic(ftw_integer));
					return 999;
				}
			}
		}
	}
	return 0;
}

int
test_func15(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	do_info(path_name);
	if (strcmp(path_name, "./tmp/data/d666/errs") == 0) {
		if (ftw_integer != FTW_NS) {
			fprintf(temp,
				"ERROR: FTW_NS not passed for file in unsearchable dir\n");
			return 999;
		}
	}
	return 0;
}

int
test_func16(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	const char *p;

	if ((p = strstr(path_name, NFTW2)) != NULL) {
		p += strlen(NFTW2) + 1;
	} else {
		p = path_name;
	}

	if (ftwp->level != getlev(p)) {
		fprintf(temp, "ERROR: Incorrect value of level for %s\n",
			path_name);
		fprintf(temp, "       Expected %d, received %d\n",
			getlev(p), ftwp->level);
		return 999;
	}
	if (ftwp->base != getbase(path_name)) {
		fprintf(temp, "ERROR: Incorrect value of base for %s\n",
			path_name);
		fprintf(temp, "       Expected %d, received %d\n",
			getbase(path_name), ftwp->base);
		return 999;
	}
	return 0;
}

int
test_func17(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	do_info(path_name);

	if (ftw_integer == FTW_SL) {
		visit++;
		return 999;
	}
	return 0;
}

int
test_func18(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	do_info(path_name);
	if (ftw_integer == FTW_SLN) {
		visit++;
		return 999;
	}
	return 0;
}

int
test_func19(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	do_info(path_name);
	visit++;
	if (ftw_integer == FTW_DNR) {
		if (strcmp(path_name, "./tmp/data/d333") == 0) {
			return 0;
		} else {
			fprintf(temp,
				"ERROR: When FTW_DNR is passed to the function fn the\n");
			fprintf(temp,
				"       descendants of the directory should not have\n");
			fprintf(temp, "       Been processed\n");
			return 999;
		}
	} else {
		fprintf(temp,
			"ERROR: Directory has read permission or FTW_DNR was not passed to fn\n");
		return 999;
	}
}

int
test_func20(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	return 0;
}

int
test_func21(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	int fd;

	do_info(path_name);
	/* get next file descriptor available */
	if ((fd = open(path_name, O_RDONLY)) == -1) {
		perror("open");
		return 999;
	}

	if (close(fd) == -1) {
		perror("close");
		return 999;
	}

	if ((fd != next_fd[0]) && (fd != next_fd[1])) {
		fprintf(temp,
			"ERROR: Expected next fd available to be %d (none used) or %d (1 used)\n",
			next_fd[0], next_fd[1]);
		fprintf(temp, "       Next fd available is %d\n", fd);
		return 999;
	}
	return 0;
}

int
test_func22(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	int fd;
	int i;

	do_info(path_name);
	/* get next file descriptor available */
	if ((fd = open(path_name, O_RDONLY)) == -1) {
		perror("open");
		return 999;
	}

	if (close(fd) == -1) {
		perror("close");
		return 999;
	}

	for (i = 0; i <= ftwp->level + 1; i++) {
		if (fd == next_fd[i])
			return 0;
	}

	fprintf(temp,
		"ERROR: At the start of the traversal the next four fds were: %d, %d, %d, and %d\n",
		next_fd[0], next_fd[1], next_fd[2], next_fd[3]);
	fprintf(temp, "       Traversing level %d the next fd is %d\n",
		ftwp->level, fd);
	return 999;
}

int
test_func23(const char *path_name, const struct stat *stat_pointer,
	    int ftw_integer, struct FTW *ftwp)
{
	visit++;
	do_info(path_name);

	if (ftw_integer == FTW_F) {

#ifdef DEBUG
		fprintf(temp,
			"INFO: fn() returning non-zero after traversal of %d objects\n",
			visit);
#endif

		return 999;
	}

	return 0;
}
