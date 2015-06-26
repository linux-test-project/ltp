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
 *	nftw.c - Test of nftw()
 */

#include <pwd.h>
#include "nftw.h"

void setup(void);
void blenter(void);
void blexit(void);
void anyfail(void);

char progname[] = "nftw.c";

/** LTP Port **/
#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
int block_number;

FILE *temp;
char *TCID = "nftw01";
int TST_TOTAL = 10;

struct passwd *ltpuser;		/* password struct for ltpuser */
/**************/

/* Used for error return for some library routines */
int s2;

/* error messages formatted here. */
char ebuf[ERR_BUF_SIZ];

/*
 * Local data declarations.
 */
char *dirlist[NDIRLISTENTS];

int visit;
int next_fd[4];

pathdata pathdat[] = {
	{
	 "./tmp/data",
	 S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH,
	 DIR, ""}, {
		    "./tmp/byebye",
		    S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH,
		    REG, "byebye!\n"}, {
					"./tmp/data/d333",
					S_IRWXU | S_IRWXG | S_IRWXO,
					DIR, ""}, {
						   "./tmp/data/d666",
						   S_IRWXU | S_IRWXG | S_IRWXO,
						   DIR, ""}, {
							      "./tmp/data/d777",
							      S_IRWXU | S_IRWXG
							      | S_IRWXO,
							      DIR, ""}, {
									 "./tmp/data/dirg",
									 S_IRWXU
									 |
									 S_IRWXG
									 |
									 S_IROTH
									 |
									 S_IWOTH,
									 DIR,
									 ""}, {
									       "./tmp/data/dirh",
									       S_IRWXU
									       |
									       S_IRWXG
									       |
									       S_IROTH
									       |
									       S_IWOTH,
									       DIR,
									       ""},
	{
	 "./tmp/data/dirl",
	 S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH,
	 DIR, ""}, {
		    "./tmp/data/d333/errs",
		    S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH,
		    REG, "Do not eat yellow snow!\n"}, {
							"./tmp/data/d666/errs",
							S_IRWXU | S_IRWXG |
							S_IROTH | S_IWOTH,
							REG,
							"Do not eat yellow snow!\n"},
	{
	 "./tmp/data/d777/errs",
	 S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH,
	 REG, "Do not eat yellow snow!\n"}, {
					     "./tmp/data/dirg/filebad",
					     S_IRUSR | S_IWUSR | S_IRGRP |
					     S_IROTH,
					     REG, ""}, {
							"./tmp/data/dirg/fileok",
							S_IRUSR | S_IWUSR |
							S_IRGRP | S_IROTH,
							REG, ""}, {
								   "./tmp/data/dirg/symlink",
								   S_IRWXU |
								   S_IRWXG |
								   S_IRWXO,
								   SYM,
								   "../../byebye"},
	{
	 "./tmp/data/dirg/dir_left.1",
	 S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH,
	 DIR, ""}, {
		    "./tmp/data/dirg/dir_left.1/dir_left.2",
		    S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH,
		    DIR, ""}, {
			       "./tmp/data/dirg/dir_right.1",
			       S_IRWXU | S_IRWXG | S_IRWXO,
			       DIR, ""}, {
					  "./tmp/data/dirg/dir_left.1/dir_left.2/left.3",
					  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
					  | S_IROTH,
					  REG, ""}, {
						     "./tmp/data/dirh/dir_left.1",
						     S_IRWXU | S_IRWXG | S_IROTH
						     | S_IWOTH,
						     DIR, ""}, {
								"./tmp/data/dirh/dir_right.1",
								S_IRWXU |
								S_IRWXG |
								S_IROTH |
								S_IWOTH,
								DIR, ""}, {
									   "./tmp/data/dirh/dir_left.1/dir_left.2",
									   S_IRWXU
									   |
									   S_IRWXG
									   |
									   S_IROTH
									   |
									   S_IWOTH,
									   DIR,
									   ""},
	{
	 "./tmp/data/dirh/dir_left.1/dir_left.2/left.3",
	 S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
	 REG, "left leaf\n"}, {
			       "./tmp/data/dirh/dir_right.1/dir_right.2",
			       S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH,
			       DIR, ""}, {
					  "./tmp/data/dirh/dir_right.1/dir_right.2/right.3",
					  S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH
					  | S_IWOTH,
					  REG, "right leaf\n"}, {
								 "./tmp/data/dirl/dir_left.1",
								 S_IRWXU |
								 S_IRWXG |
								 S_IROTH |
								 S_IWOTH,
								 DIR, ""}, {
									    "./tmp/data/dirl/dir_left.1/dir_left.2",
									    S_IRWXU
									    |
									    S_IRWXG
									    |
									    S_IROTH
									    |
									    S_IWOTH,
									    DIR,
									    ""},
	{
	 "./tmp/data/dirl/dir_left.1/dir_left.2/left.3",
	 0,
	 SYM, "../../../dirh"}, {
				 "./tmp/data/dirl/dir_right.1",
				 S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH,
				 DIR, ""}, {
					    "./tmp/data/dirl/dir_right.1/dir_right.2",
					    S_IRWXU | S_IRWXG | S_IROTH |
					    S_IWOTH,
					    DIR, ""}, {
						       "./tmp/data/dirl/dir_right.1/dir_right.2/right.3",
						       0,
						       SYM, "../dir_right.2"}, {
										"./tmp/data/loop",
										0,
										SYM,
										"./loop"}
};

char *goodlist[] = {
	"/dirh",
	"/dirh/dir_left.1",
	"/dirh/dir_right.1",
	"/dirh/dir_left.1/dir_left.2",
	"/dirh/dir_right.1/dir_right.2",
	"/dirh/dir_left.1/dir_left.2/left.3",
	"/dirh/dir_right.1/dir_right.2/right.3"
};

struct list badlist[] = {
	{"/dirg", FTW_D},
	{"/dirg/dir_left.1", FTW_D},
	/* not FTW_NS in following since stat can't fail if file exists */
	{"/dirg/filebad", FTW_F},
	{"/dirg/fileok", FTW_F},
	{"/dirg/symlink", FTW_SL},
	{"/dirg/dir_right.1", FTW_DNR},
	{"/dirg/dir_left.1/dir_left.2", FTW_D},
	{"/dirg/dir_left.1/dir_left.2/left.3", FTW_F},
};

struct list mnem[] = {
	{"FTW_F", FTW_F},
	{"FTW_D", FTW_D},
	{"FTW_DNR", FTW_DNR},
	{"FTW_NS", FTW_NS},
	{"FTW_SL", FTW_SL},
#ifndef __linux__
/* How do we define __USE_XOPEN_EXTENDED ? Following depends on that */
	{"FTW_DP", FTW_DP},
	{"FTW_SLN", FTW_SLN},
#endif
};

int npathdats, ngoods, nbads, nmnem;

/*--------------------------------------------------------------*/
int main(void)
{
	setup();		/* temp file is now open        */

	npathdats = ARRAY_SIZE(pathdat);
	ngoods = ARRAY_SIZE(goodlist);
	nbads = ARRAY_SIZE(badlist);
	nmnem = ARRAY_SIZE(mnem);

	setup_path();

/*---------------- ENTER BLOCK 0 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall recursively descend the\n");
	fprintf(temp, "directory hierarchy rooted in path until it has\n");
	fprintf(temp,
		"traversed the whole tree, calling the function fn for\n");
	fprintf(temp, "each object in the directory tree, and return 0.\n\n");
#endif
	test1A();
	blexit();
/*--------------- EXIT BLOCK 0 ---------------------------------*/

/*---------------- ENTER BLOCK 1 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) when flags contains FTW_PHYS shall\n");
	fprintf(temp, "not traverse symbolic links.\n\n");
#endif
	test2A();
	blexit();
/*--------------- EXIT BLOCK 1 ---------------------------------*/

/*---------------- ENTER BLOCK 2 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp,
		"depth, int flags) when flags does not contain FTW_PHYS\n");
	fprintf(temp,
		"shall follow links instead of reporting them and shall\n");
	fprintf(temp, "not report the same file twice.\n\n");
#endif
	test3A();
	blexit();
/*--------------- EXIT BLOCK 2 ---------------------------------*/

/*---------------- ENTER BLOCK 3 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp,
		"depth, int flags) when flags contains FTW_DEPTH shall\n");
	fprintf(temp, "report all files in a directory before reporting the\n");
	fprintf(temp, "directory.\n\n");
#endif
	test4A();
	blexit();
/*--------------- EXIT BLOCK 3 ---------------------------------*/

/*---------------- ENTER BLOCK 4 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) when flags does not contain\n");
	fprintf(temp, "FTW_DEPTH shall report a directory before reporting\n");
	fprintf(temp, "the files in that directory.\n\n");
#endif
	test5A();
	blexit();
/*--------------- EXIT BLOCK 4 ---------------------------------*/

/*---------------- ENTER BLOCK 5 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp,
		"depth, int flags) when flags contains FTW_CHDIR shall\n");
	fprintf(temp,
		"change the current working directory to each directory\n");
	fprintf(temp, "as it reports files in that directory.\n\n");
#endif
	test6A();
	blexit();
/*--------------- EXIT BLOCK 5 ---------------------------------*/

/*---------------- ENTER BLOCK 6 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall pass the path-name of the\n");
	fprintf(temp, "current object as the first argument of the function\n");
	fprintf(temp, "fn.\n\n");
#endif
	test7A();
	blexit();
/*--------------- EXIT BLOCK 6 ---------------------------------*/

/*---------------- ENTER BLOCK 7 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall pass a pointer to a stat\n");
	fprintf(temp, "structure containing information about the current\n");
	fprintf(temp, "object as the second argument to fn.\n\n");
#endif
	test8A();
	blexit();
/*--------------- EXIT BLOCK 7 ---------------------------------*/

/*---------------- ENTER BLOCK 8 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall pass FTW_F as the third\n");
	fprintf(temp,
		"argument of the function fn when the object is a file.\n\n");
#endif
	test9A();
	blexit();
/*--------------- EXIT BLOCK 8 ---------------------------------*/

/*---------------- ENTER BLOCK 9 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall pass FTW_D as the third\n");
	fprintf(temp, "argument of the function fn when the object is a\n");
	fprintf(temp, "directory.\n\n");
#endif
	test10A();
	blexit();
/*--------------- EXIT BLOCK 9 ---------------------------------*/

/*---------------- ENTER BLOCK 10 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall pass FTW_DP as the third\n");
	fprintf(temp, "argument of the function fn when the object is a\n");
	fprintf(temp, "directory and subdirectories have been visited.\n\n");
#endif
	test11A();
	blexit();
/*--------------- EXIT BLOCK 10 ---------------------------------*/

/*---------------- ENTER BLOCK 11 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall pass FTW_SL as the third\n");
	fprintf(temp, "argument of the function fn when the object is a\n");
	fprintf(temp, "symbolic link.\n\n");
#endif
	test12A();
	blexit();
/*--------------- EXIT BLOCK 11 ---------------------------------*/

/*---------------- ENTER BLOCK 12 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall pass FTW_SLN as the third\n");
	fprintf(temp, "argument of the function fn when the object is a\n");
	fprintf(temp, "symbolic link that does not name an existing file.\n\n");
#endif
	test13A();
	blexit();
/*--------------- EXIT BLOCK 12 ---------------------------------*/

/*---------------- ENTER BLOCK 13 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall pass FTW_DNR as the third\n");
	fprintf(temp, "argument of the function fn when the object is a\n");
	fprintf(temp, "directory that cannot be read.\n\n");
#endif
	test14A();
	blexit();
/*--------------- EXIT BLOCK 13 ---------------------------------*/

/*---------------- ENTER BLOCK 14 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall pass FTW_NS as the third\n");
	fprintf(temp,
		"argument of the function fn when stat() failed on the\n");
	fprintf(temp, "object because of lack of appropriate permission.\n\n");
#endif
	test15A();
	blexit();
/*--------------- EXIT BLOCK 14 ---------------------------------*/

/*---------------- ENTER BLOCK 15 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall pass a structure which\n");
	fprintf(temp, "contains the offset into the pathname of the object\n");
	fprintf(temp, "and the depth relative to the root of the walk\n");
	fprintf(temp,
		"starting from 0 as the fourth argument of the function\n");
	fprintf(temp, "fn.\n\n");
#endif
	test16A();
	blexit();
/*--------------- EXIT BLOCK 15 ---------------------------------*/

/*---------------- ENTER BLOCK 16 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall pass FTW_SL as the third\n");
	fprintf(temp, "argument to the function fn if and only if the\n");
	fprintf(temp, "FTW_PHYS flag is included in flags.\n\n");
#endif
	test17A();
	blexit();
/*--------------- EXIT BLOCK 16 ---------------------------------*/

/*---------------- ENTER BLOCK 17 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall pass FTW_SLN as the third\n");
	fprintf(temp, "argument to the function fn if and only if the\n");
	fprintf(temp, "FTW_PHYS flag is not included in flags.\n\n");
#endif
	test18A();
	blexit();
/*--------------- EXIT BLOCK 17 ---------------------------------*/

/*---------------- ENTER BLOCK 18 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "On a call to int nftw(const char *path, int\n");
	fprintf(temp, "(*fn)(const char *, const struct stat *, int, struct\n");
	fprintf(temp,
		"FTW *), int depth, int flags) when the third argument\n");
	fprintf(temp, "passed to the function fn is FTW_DNR then the\n");
	fprintf(temp,
		"descendants of the directory shall not be processed.\n\n");
#endif
	test19A();
	blexit();
/*--------------- EXIT BLOCK 18 ---------------------------------*/

/*---------------- ENTER BLOCK 19 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp,
		"depth, int flags) shall close any file descriptors or\n");
	fprintf(temp,
		"directory streams used to traverse the directory tree.\n\n");
#endif
	test20A();
	blexit();
/*--------------- EXIT BLOCK 19 ---------------------------------*/

/*---------------- ENTER BLOCK 20 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "On a call to int nftw(const char *path, int\n");
	fprintf(temp, "(*fn)(const char *, const struct stat *, int, struct\n");
	fprintf(temp, "FTW *), int depth, int flags) depth shall be the\n");
	fprintf(temp,
		"maximum number of file descriptors used for the search.\n\n");
#endif
	test21A();
	blexit();
/*--------------- EXIT BLOCK 20 ---------------------------------*/

/*---------------- ENTER BLOCK 21 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) shall use at most one file\n");
	fprintf(temp, "descriptor for each directory level.\n\n");
#endif
	test22A();
	blexit();
/*--------------- EXIT BLOCK 21 ---------------------------------*/

/*---------------- ENTER BLOCK 22 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "A call to int nftw(const char *path, int (*fn)(const\n");
	fprintf(temp, "char *, const struct stat *, int, struct FTW *), int\n");
	fprintf(temp, "depth, int flags) when the function fn returns a\n");
	fprintf(temp, "non-zero value shall stop and return the value\n");
	fprintf(temp, "returned by fn.\n\n");
#endif
	test23A();
	blexit();
/*--------------- EXIT BLOCK 22 ---------------------------------*/

/*---------------- ENTER BLOCK 23 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "ENAMETOOLONG in errno and return -1 on a call to int\n");
	fprintf(temp, "nftw(const char *path, int (*fn)(const char *, const\n");
	fprintf(temp, "struct stat *, int, struct FTW *), int depth, int\n");
	fprintf(temp, "flags) when the length of path exceeds PATH_MAX.\n\n");
#endif
	test24A();
	blexit();
/*--------------- EXIT BLOCK 23 ---------------------------------*/

/*---------------- ENTER BLOCK 24 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "ENAMETOOLONG in errno and return -1 on a call to int\n");
	fprintf(temp, "nftw(const char *path, int (*fn)(const char *, const\n");
	fprintf(temp, "struct stat *, int, struct FTW *), int depth, int\n");
	fprintf(temp, "flags) when a component of path exceeds NAME_MAX.\n\n");
#endif
	test25A();
	blexit();
/*--------------- EXIT BLOCK 24 ---------------------------------*/

/*---------------- ENTER BLOCK 25 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "ENOENT in errno and return -1 on a call to int\n");
	fprintf(temp, "nftw(const char *path, int (*fn)(const char *, const\n");
	fprintf(temp, "struct stat *, int, struct FTW *), int depth, int\n");
	fprintf(temp,
		"flags) when path points to a file which does not exist.\n\n");
#endif
	test26A();
	blexit();
/*--------------- EXIT BLOCK 25 ---------------------------------*/

/*---------------- ENTER BLOCK 26 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "ENOENT in errno and return -1 on a call to int\n");
	fprintf(temp, "nftw(const char *path, int (*fn)(const char *, const\n");
	fprintf(temp, "struct stat *, int, struct FTW *), int depth, int\n");
	fprintf(temp, "flags) when path points to an empty string.\n\n");
#endif
	test27A();
	blexit();
/*--------------- EXIT BLOCK 26 ---------------------------------*/

/*---------------- ENTER BLOCK 27 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "ENOTDIR in errno and return -1 on a call to int\n");
	fprintf(temp, "nftw(const char *path, int (*fn)(const char *, const\n");
	fprintf(temp, "struct stat *, int, struct FTW *), int depth, int\n");
	fprintf(temp, "flags) when path is not a directory.\n\n");
#endif
	test28A();
	blexit();
/*--------------- EXIT BLOCK 27 ---------------------------------*/

/*---------------- ENTER BLOCK 28 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "EACCES in errno and return -1 on a call to int\n");
	fprintf(temp, "nftw(const char *path, int (*fn)(const char *, const\n");
	fprintf(temp, "struct stat *, int, struct FTW *), int depth, int\n");
	fprintf(temp, "flags) when search permission is denied for any\n");
	fprintf(temp, "component of path.\n\n");
#endif
	test29A();
	blexit();
/*--------------- EXIT BLOCK 28 ---------------------------------*/

/*---------------- ENTER BLOCK 29 --------------------------------*/
	blenter();
#ifdef DEBUG
	fprintf(temp, "EACCES in errno and return -1 on a call to int\n");
	fprintf(temp, "nftw(const char *path, int (*fn)(const char *, const\n");
	fprintf(temp, "struct stat *, int, struct FTW *), int depth, int\n");
	fprintf(temp, "flags) when read permission is denied for path.\n\n");
#endif
	test30A();
	blexit();
/*--------------- EXIT BLOCK 29 ---------------------------------*/

	cleanup_function();

	anyfail();		/* THIS CALL DOES NOT RETURN - EXITS!!  */
	tst_exit();
/*--------------------------------------------------------------*/
}

/** LTP Port **/
/*
 * setup
 *
 * Do set up - here its a dummy function
 */
void setup(void)
{
	/* Direct debug output to stderr */
	temp = stderr;

	/* Get the user id "nobody" */
	if ((ltpuser = getpwnam("nobody")) == NULL) {
		perror("nobody not found in /etc/passwd");
		exit(1);
	}

	/* Switch to "nobody" */
	setuid(ltpuser->pw_uid);

	tst_tmpdir();
}

/*
 * Function: blenter()
 *
 * Description: Print message on entering a new block
 */
void blenter(void)
{
	local_flag = PASSED;
	return;
}

/*
 * Function: blexit()
 *
 * Description: This function will exit a block, a block may be a logical unit
 *              of a test. It will report the status if the test ie fail or
 *              pass.
 */
void blexit(void)
{
	(local_flag == PASSED) ? tst_resm(TPASS, "Test block %d", block_number)
	    : tst_resm(TFAIL, "Test block %d", block_number);
	block_number++;
	return;
}

/*
 *
 * Function: anyfail()
 *
 * Description: Exit a test.
 */
void anyfail(void)
{
	(local_flag == FAILED) ? tst_resm(TFAIL, "Test failed")
	    : tst_resm(TPASS, "Test passed");
	tst_rmdir();
	tst_exit();
}

/**************/
