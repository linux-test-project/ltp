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
 *      test64.c - Test functions for nftw64()
 */

#include "nftw64.h"

extern int callback(const char *path);

extern pathdata pathdat[];
extern struct list mnem[], badlist[];
extern char *dirlist[NDIRLISTENTS], *goodlist[];
extern int npathdats, ngoods, nbads, nmnem, visit, s2, next_fd[4];
extern FILE *temp;
/*
 *    void test1A()     - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall
 *      recursively descend the directory hierarchy rooted in path until it
 *      has traversed the whole tree, calling the function fn for each object
 *      in the directory tree, and return 0.
 */

void test1A(void)
{
	int i, j, ret;
	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: nftw64() succeeds\n");
#endif

	visit = 0;
	if ((ret = nftw64("./tmp/data/dirh", test_func1, MAX_FD, 0)) == -1) {
		perror("ERROR: nftw64 failed");
		cleanup_function();
		fail_exit();
	}

	if (ret == 999) {
		cleanup_function();
		fail_exit();
	}
#ifdef DEBUG
	fprintf(temp, "TEST: Whole tree traversed\n");
#endif

	if (visit != ngoods) {
		fprintf(temp, "ERROR: Count of objects visited incorrect\n");
		fprintf(temp, "       Expected %d, Received %d\n", ngoods,
			visit);
		cleanup_function();
		fail_exit();
	}

	for (i = 0; i < visit; i++) {
		for (j = 0; j < ngoods; j++) {
			if (strcmp(dirlist[i], goodlist[j]) == 0) {
				free(dirlist[i]);
				dirlist[i] = NULL;
				break;
			}
		}
	}

	for (i = 0; i < visit; i++) {
		if (dirlist[i] != NULL) {
			free(dirlist[i]);
			fprintf(temp, "ERROR: Unexpected visit to %s\n",
				dirlist[i]);
			cleanup_function();
			fail_exit();
		}
	}
}

/*
 *    void test2A()     - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) when flags
 *      contains FTW_PHYS shall not traverse symbolic links.
 */

void test2A(void)
{
	int i, ret;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: nftw64 with FTW_PHYS does not follow symbolic links\n");
#endif

	visit = 0;
	if ((ret = nftw64("./tmp/data/dirl", test_func1, MAX_FD, FTW_PHYS))
	    == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret == 999) {
		cleanup_function();
		fail_exit();
	}

	if (visit != NO_LINK_CNT) {
		fprintf(temp,
			"ERROR: Expected %d files to be visited.  nftw64() visited %d\n",
			NO_LINK_CNT, visit);
		cleanup_function();
		fail_exit();
	}

	for (i = 0; i < visit; i++) {
		if (dirlist[i] != NULL)
			free(dirlist[i]);
	}
}

/*
 *    void test3A()     - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) when flags
 *      does not contain FTW_PHYS shall follow links instead of reporting
 *      them and shall not report the same file twice.
 */

void test3A(void)
{
	int ret;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: nftw64 without FTW_PHYS follows symbolic links\n");
#endif

	visit = 0;

	if ((ret = nftw64("./tmp/data/dirl", test_func3, MAX_FD, 0)) == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}
	if (ret == 999) {
		cleanup_function();
		fail_exit();
	}

	if (visit != LINK_CNT - 1) {
		fprintf(temp,
			"ERROR: Expected %d files to be visited.  nftw64() visited %d\n",
			LINK_CNT - 1, visit);
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test4A()     - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) when flags
 *      contains FTW_DEPTH shall report all files in a directory before
 *      reporting the directory.
 */

void test4A(void)
{
	char path[] = "./tmp/data/d777";
	int ret_val;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: Verify traversal with FTW_DEPTH set\n");
#endif

	visit = 0;
	if ((ret_val = nftw64(path, test_func4, MAX_FD, FTW_DEPTH)) == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}
	if (ret_val != 999) {
		fprintf(temp, "ERROR: %s never visited\n", path);
		cleanup_function();
		fail_exit();
	}

	if (visit != 2) {
		fprintf(temp, "ERROR: Visited directory before contents\n");
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test5A()     - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) when flags
 *      does not contain FTW_DEPTH shall report a directory before reporting
 *      the files in that directory.
 */

void test5A(void)
{
	char path[] = "./tmp/data/d777";
	int ret_val;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: Verify traversal without FTW_DEPTH set\n");
#endif

	visit = 0;
	if ((ret_val = nftw64(path, test_func4, MAX_FD, 0)) == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}
	if (ret_val != 999) {
		fprintf(temp, "ERROR: %s never visited\n", path);
		cleanup_function();
		fail_exit();
	}

	if (visit != 1) {
		fprintf(temp, "ERROR: Visited contents before directory\n");
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test6A()     - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) when flags
 *      contains FTW_CHDIR shall change the current working directory to each
 *      directory as it reports files in that directory.
 */

void test6A(void)
{
	char path[PATH_MAX + NAME_MAX];
	int ret_val;

	if (getcwd(path, sizeof(path)) == NULL) {
		perror("getcwd");
		cleanup_function();
		fail_exit();
	}
	(void)strcat(path, "/tmp/data/dirh");

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: nftw64 with FTW_CHDIR changes to each dir before reporting files in it\n");
#endif

	ret_val = nftw64(path, test_func5, MAX_FD, FTW_CHDIR);
	if (ret_val == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}
	if ((ret_val == 998) || (ret_val == 999)) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test7A()     - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall pass
 *      the path-name of the current object as the first argument of the
 *      function fn.
 */

void test7A(void)
{
	int ret;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: nftw64 passes pathname as first argument to fn()\n");
#endif

	if ((ret = nftw64("./tmp/data/dirg", test_func7, MAX_FD, 0)) == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret == 999) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test8A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall pass a
 *      pointer to a stat structure containing information about the current
 *      object as the second argument to fn.
 */

void test8A(void)
{
	int ret;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: nftw64 passes stat struct as second argument to fn()\n");
#endif

	if ((ret = nftw64("./tmp/data/dirg", test_func8, MAX_FD, 0)) == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret == 999) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test9A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall pass
 *      FTW_F as the third argument of the function fn when the object is a
 *      file
 */

void test9A(void)
{
	int ret;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: nftw64 passes FTW_F as third arg to fn() for files\n");
#endif

	if ((ret = nftw64("./tmp/data/dirg", test_func9, MAX_FD, FTW_PHYS))
	    == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret == 999) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test10A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall pass
 *      FTW_D as the third argument of the function fn when the object is a
 *      directory.
 */

void test10A(void)
{
	int ret;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: nftw64 passes FTW_D as third arg to fn() when file is directory\n");
#endif

	if ((ret = nftw64("./tmp/data/dirg", test_func10, MAX_FD, FTW_PHYS))
	    == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret == 999) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test11A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall pass
 *      FTW_DP as the third argument of the function fn when the object is a
 *      directory and subdirectories have been visited.
 */

void test11A(void)
{
	int i, ret;

	for (i = 0; i < nbads; i++)
		if (badlist[i].i == FTW_D)
			badlist[i].i = FTW_DP;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: nftw64 passes FTW_DP when file is directory and subdirs already visited\n");
#endif

	if ((ret = nftw64("./tmp/data/dirg", test_func11, MAX_FD, FTW_DEPTH |
			  FTW_PHYS)) == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret == 999) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test12A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall pass
 *      FTW_SL as the third argument of the function fn when the object is a
 *      symbolic link.
 */

void test12A(void)
{
	int ret;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: nftw64 wth FTW_PHYS passes FTW_SL when file is symlink\n");
#endif
	if ((ret = nftw64("./tmp/data/dirg", test_func12, MAX_FD, FTW_PHYS))
	    == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret == 999) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test13A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall pass
 *      FTW_SLN as the third argument of the function fn when the object is a
 *      symbolic link that does not name an existing file.
 */

void test13A(void)
{
	int i, ret;

	if (unlink("./tmp/byebye") == -1) {
		perror("unlink");
		cleanup_function();
		fail_exit();
	}

	for (i = 0; i < nbads; i++)
		if (badlist[i].i == FTW_SL)
			badlist[i].i = FTW_SLN;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: nftw64 with FTW_PHYS passes FTW_SLN when file");
	fprintf(temp, " is symlink pointing \n to non-existent file\n");
#endif

	if ((ret = nftw64("./tmp/data/dirg", test_func13, MAX_FD, FTW_PHYS))
	    == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret == 999) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test14A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall pass
 *      FTW_DNR as the third argument of the function fn when the object is a
 *      directory that cannot be read.
 */

void test14A(void)
{
	int ret;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: nftw64 passes FTW_DNR when file is directory that cannot be read\n");
#endif

	if ((ret = nftw64("./tmp/data/d333", test_func14, MAX_FD, 0)) == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret == 999) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test15A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall pass
 *      FTW_NS as the third argument of the function fn when stat() failed on
 *      the object because of lack of appropriate permission.
 */

void test15A(void)
{
	int ret;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: nftw64(path, fn, depth, FTW_PHYS) passes FTW_NS when dir unsearchable\n");
#endif

	if ((ret = nftw64("./tmp/data/d666", test_func15, MAX_FD, FTW_PHYS))
	    == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret == 999) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test16A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall pass a
 *      structure which contains the offset into the pathname of the object
 *      and the depth relative to the root of the walk starting from 0 as the
 *      fourth argument of the function fn.
 */

void test16A(void)
{
	char path[PATH_MAX + NAME_MAX];
	char orig[PATH_MAX + NAME_MAX];

	if (getcwd(orig, sizeof(orig)) == NULL) {
		perror("getcwd on original wd");
		cleanup_function();
		fail_exit();
	}
	strcpy(path, orig);
	(void)strcat(path, "/tmp/data/dirg");

	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: nftw64 with absolute pathname %s\n", path);
#endif

	if ((s2 = nftw64(path, test_func16, MAX_FD, 0)) == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}
	if (s2 == 999) {
		cleanup_function();
		fail_exit();
	}

	(void)strcpy(path, "./tmp/data/dirg");

#ifdef DEBUG
	fprintf(temp, "TEST: nftw64 with relative pathname %s\n", path);
#endif

	if ((s2 = nftw64(path, test_func16, MAX_FD, 0)) == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (s2 == 999) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test17A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall pass
 *      FTW_SL as the third argument to the function fn if and only if the
 *      FTW_PHYS flag is included in flags.
 */

void test17A(void)
{
	int ret;

	visit = 0;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: nftw64 with FTW_PHYS passes FTW_SL for symlink\n");
#endif

	if ((ret = nftw64("./tmp/data/dirl", test_func17, MAX_FD, FTW_PHYS))
	    == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}
	if (ret != 999) {
		fprintf(temp, "ERROR: nftw64() failed to find symbolic link\n");
		cleanup_function();
		fail_exit();
	}

	visit = 0;

#ifdef DEBUG
	fprintf(temp,
		"TEST: nftw64 without FTW_PHYS does not pass FTW_SL for symlink\n");
#endif

	if ((ret = nftw64("./tmp/data/dirl", test_func17, MAX_FD, 0)) == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}
	if (ret == 999) {
		fprintf(temp, "ERROR: nftw64() found symbolic link\n");
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test18A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall pass
 *      FTW_SLN as the third argument to the function fn if and only if the
 *      FTW_PHYS flag is not included in flags.
 */

void test18A(void)
{
	int ret;

	unlink("./tmp/byebye");

	visit = 0;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: nftw64 with FTW_PHYS does not pass FTW_SLN\n");
#endif

	if ((ret = nftw64("./tmp/data/dirg", test_func18, MAX_FD, FTW_PHYS))
	    == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}
	if (ret == 999) {
		fprintf(temp, "ERROR: nftw64() passed FTW_SLN\n");
		cleanup_function();
		fail_exit();
	}

	visit = 0;

#ifdef DEBUG
	fprintf(temp, "TEST: nftw64 without FTW_PHYS passes FTW_SLN\n");
#endif

	if ((ret = nftw64("./tmp/data/dirg", test_func18, MAX_FD, 0)) == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (visit == 1) {
		if (ret == 999) {
			/* Test is passed */
			return;
		} else {
			fprintf(temp, "ERROR: nftw64 passed FTW_SLN but did");
			fprintf(temp, "not return value returned by fn()\n");
			cleanup_function();
			fail_exit();
		}
	} else {
		fprintf(temp, "ERROR: nftw64() did not pass FTW_SLN\n");
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test19A()    - tests the assertion:
 *      On a call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) when the
 *      third argument passed to the function fn is FTW_DNR then the
 *      descendants of the directory shall not be processed.
 */

void test19A(void)
{
	int ret_val;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: Can not traverse directory with no read permission\n");
#endif

	visit = 0;

	ret_val = nftw64("./tmp/data/d333", test_func19, MAX_FD, 0);
	if (ret_val == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret_val == 999) {
		cleanup_function();
		fail_exit();
	}
#ifdef DEBUG
	fprintf(temp, "TEST: fn only be called once\n");
#endif

	if (visit != 1) {
		fprintf(temp, "ERROR: %s",
			"Directory without read permission allows traversing\n");
		fprintf(temp, "       Visited %d files\n", visit);
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test20A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall close
 *      any file descriptors or directory streams used to traverse the
 *      directory tree.
 */

void test20A(void)
{
	int fd, nfd;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: File descriptors used in traversal are closed\n");
#endif

	if ((fd = open("./tmp/data/dirh", O_RDONLY)) == -1) {
		perror("close");
		cleanup_function();
		fail_exit();
	}

	if (close(fd) == -1) {
		perror("close");
		cleanup_function();
		fail_exit();
	}

	if (nftw64("./tmp/data/dirh", test_func20, 1, 0) == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if ((nfd = open("./tmp/data/dirh", O_RDONLY)) == -1) {
		perror("open");
		cleanup_function();
		fail_exit();
	}

	if (nfd != fd) {
		fprintf(temp, "ERROR: %s,fd == %d ofd = %d",
			"nftw64 did not close all file descriptors used in traversal\n",
			nfd, fd);
		cleanup_function();
		fail_exit();
	}

	if (close(nfd) == -1) {
		perror("close");
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test21A()    - tests the assertion:
 *      On a call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall
 *      be the maximum number of file descriptors used for the search.
 */

void test21A(void)
{
	char path[] = "./tmp/data/dirh";
	int ret_val;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: No more than depth file descriptors used in traversal\n");
#endif

	/*this is the fd we expect if 0 are used */
	if ((next_fd[0] = open(path, O_RDONLY)) == -1) {
		perror("open next_fd[0]");
		cleanup_function();
		fail_exit();
	}

	/*this is the fd we expect if 1 is used */
	if ((next_fd[1] = open(path, O_RDONLY)) == -1) {
		perror("open next_fd[1]");
		cleanup_function();
		fail_exit();
	}

	if (close(next_fd[0]) == -1) {
		perror("close next_fd[0]");
		cleanup_function();
		fail_exit();
	}

	if (close(next_fd[1]) == -1) {
		perror("close next_fd[1]");
		cleanup_function();
		fail_exit();
	}

	visit = 0;
	ret_val = nftw64(path, test_func21, 1, 0);
	if (ret_val == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret_val == 999) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test22A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) shall use at
 *      most one file descriptor for each directory level.
 */

void test22A(void)
{
	char path[] = "./tmp/data/dirh";
	int ret_val, i;

	for (i = 0; i < 4; i++) {
		if ((next_fd[i] = open(path, O_RDONLY)) == -1) {
			perror("open");
			cleanup_function();
			fail_exit();
		}
	}

	for (i = 0; i < 4; i++) {
		if (close(next_fd[i]) == -1) {
			perror("close");
			cleanup_function();
			fail_exit();
		}
	}

	visit = 0;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: No more than 1 fd per level is used in traversal\n");
#endif

	ret_val = nftw64(path, test_func22, MAX_FD, 0);

	if (ret_val == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret_val == 999) {
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test23A()    - tests the assertion:
 *      A call to int nftw64(const char *path, int (*fn)(const char *, const
 *      struct stat *, int, struct FTW *), int depth, int flags) when the
 *      function fn returns a non-zero value shall stop and return the value
 *      returned by fn.
 */

void test23A(void)
{
	int ret;

	visit = 0;

	temp = stderr;
#ifdef DEBUG
	fprintf(temp,
		"TEST: The function nftw64 should return with value set by fn\n");
#endif

	if ((ret = nftw64("./tmp/data/dirh", test_func23, MAX_FD, FTW_PHYS))
	    == -1) {
		perror("nftw64");
		cleanup_function();
		fail_exit();
	}

	if (ret != 999) {
		fprintf(temp,
			"ERROR: nftw64 did not return value returned by fn()\n");
		cleanup_function();
		fail_exit();
	}
	if (visit != 4) {
		fprintf(temp,
			"ERROR: nftw64() did not return immediately on non-zero fn() return\n");
		cleanup_function();
		fail_exit();
	}
}

/*
 *    void test24A()    - tests the assertion:
 *      ENAMETOOLONG in errno and return -1 on a call to int nftw64(const char
 *      *path, int (*fn)(const char *, const struct stat *, int, struct FTW
 *      *), int depth, int flags) when the length of path exceeds PATH_MAX.
 */

void test24A(void)
{
	test_ENAMETOOLONG_path("nftw64", callback, -1);
}

/*
 *    void test25A()    - tests the assertion:
 *      ENAMETOOLONG in errno and return -1 on a call to int nftw64(const char
 *      *path, int (*fn)(const char *, const struct stat *, int, struct FTW
 *      *), int depth, int flags) when a component of path exceeds NAME_MAX.
 */

void test25A(void)
{
	test_ENAMETOOLONG_name("nftw64", callback, -1);
}

/*
 *    void test26A()    - tests the assertion:
 *      ENOENT in errno and return -1 on a call to int nftw64(const char *path,
 *      int (*fn)(const char *, const struct stat *, int, struct FTW *), int
 *      depth, int flags) when path points to a file which does not exist.
 */

void test26A(void)
{
	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: [ENOENT] && -1 returned by nftw64\n");
#endif

	test_ENOENT_nofile("nftw64", callback, -1);
}

/*
 *    void test27A()    - tests the assertion:
 *      ENOENT in errno and return -1 on a call to int nftw64(const char *path,
 *      int (*fn)(const char *, const struct stat *, int, struct FTW *), int
 *      depth, int flags) when path points to an empty string.
 */

void test27A(void)
{
	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: The function nftw64 should return with a -1\n");
#endif

	test_ENOENT_empty("nftw64", callback, -1);
}

/*
 *    void test28A()    - tests the assertion:
 *      ENOTDIR in errno and return -1 on a call to int nftw64(const char
 *      *path, int (*fn)(const char *, const struct stat *, int, struct FTW
 *      *), int depth, int flags) when path is not a directory.
 */

void test28A(void)
{
	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: [ENOTDIR] && -1 returned by nftw64\n");
#endif

	test_ENOTDIR("nftw64", callback, -1);
}

/*
 *    void test29A()    - tests the assertion:
 *      EACCES in errno and return -1 on a call to int nftw64(const char *path,
 *      int (*fn)(const char *, const struct stat *, int, struct FTW *), int
 *      depth, int flags) when search permission is denied for any component
 *      of path.
 */

void test29A(void)
{
	if (chmod("./tmp/data/d333", (mode_t) S_IRUSR) == -1) {
		perror("chmod");
		cleanup_function();
		fail_exit();
	}

	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: [EACCES] && -1 returned by nftw64\n");
#endif

	test_ENOTDIR("nftw64", callback, -1);
}

/*
 *    void test30A()    - tests the assertion:
 *      EACCES in errno and return -1 on a call to int nftw64(const char *path,
 *      int (*fn)(const char *, const struct stat *, int, struct FTW *), int
 *      depth, int flags) when read permission is denied for path.
 */

void test30A(void)
{
	if (chmod("./tmp/data/d333", (mode_t) S_IXUSR) == -1) {
		perror("chmod");
		cleanup_function();
		fail_exit();
	}

	temp = stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: [EACCES] && -1 returned by nftw64\n");
#endif

	test_ENOTDIR("nftw64", callback, -1);
}
