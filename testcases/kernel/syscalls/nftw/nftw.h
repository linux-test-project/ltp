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
 *	nftw.h - Header file for nftw.c
 */


#ifndef _NFTW_H_
#define _NFTW_H_

#include <stdlib.h>
#include <ftw.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <linux/limits.h>

#include "test.h"

#define STRLEN          512
#define MAX_FD          20
#define MAXOPENDIRS     1024    /* max opendirs to try to exhaust dir streams */
#define NUM_2_VISIT     4
#define RET_VAL         666
#define NDIRLISTENTS    100
#define ERR_BUF_SIZ	4096
#define NFTW            "./tmp/data"
#define NFTW2           "/tmp/data"
#define LINK_CNT        13
#define NO_LINK_CNT     7
#define DIR             0
#define REG             1
#define SYM             2

typedef struct pathdata {
        char    name[PATH_MAX];
        mode_t  mode;
        int     type;
        char    contents[STRLEN];
} pathdata;

struct list {
	char	*s;
	int	 i;
};

 void fail_exit(void);

/* These functions are found in test.c */
 void test1A(void);
 void test2A(void);
 void test3A(void);
 void test4A(void);
 void test5A(void);
 void test6A(void);
 void test7A(void);
 void test8A(void);
 void test9A(void);
 void test10A(void);
 void test11A(void);
 void test12A(void);
 void test13A(void);
 void test14A(void);
 void test15A(void);
 void test16A(void);
 void test17A(void);
 void test18A(void);
 void test19A(void);
 void test20A(void);
 void test21A(void);
 void test22A(void);
 void test23A(void);
 void test24A(void);
 void test25A(void);
 void test26A(void);
 void test27A(void);
 void test28A(void);
 void test29A(void);
 void test30A(void);

/* These functions are found in test_func.c */
 int test_func1(const char *, const struct stat *, int, struct FTW *);
 int test_func3(const char *, const struct stat *, int, struct FTW *);
 int test_func4(const char *, const struct stat *, int, struct FTW *);
 int test_func5(const char *, const struct stat *, int, struct FTW *);
 int test_func7(const char *, const struct stat *, int, struct FTW *);
 int test_func8(const char *, const struct stat *, int, struct FTW *);
 int test_func9(const char *, const struct stat *, int, struct FTW *);
 int test_func10(const char *, const struct stat *, int, struct FTW *);
 int test_func11(const char *, const struct stat *, int, struct FTW *);
 int test_func12(const char *, const struct stat *, int, struct FTW *);
 int test_func13(const char *, const struct stat *, int, struct FTW *);
 int test_func14(const char *, const struct stat *, int, struct FTW *);
 int test_func15(const char *, const struct stat *, int, struct FTW *);
 int test_func16(const char *, const struct stat *, int, struct FTW *);
 int test_func17(const char *, const struct stat *, int, struct FTW *);
 int test_func18(const char *, const struct stat *, int, struct FTW *);
 int test_func19(const char *, const struct stat *, int, struct FTW *);
 int test_func20(const char *, const struct stat *, int, struct FTW *);
 int test_func21(const char *, const struct stat *, int, struct FTW *);
 int test_func22(const char *, const struct stat *, int, struct FTW *);
 int test_func23(const char *, const struct stat *, int, struct FTW *);

/* These functions are found in tools.c */
 void cleanup_function(void);
 void setup_path(void);
 int nftw_fn(const char *, const struct stat *, int, struct FTW *);
 char * ftw_mnemonic(int);
 int getbase(const char *);
 int getlev(const char *);
 void do_info(const char *);

/* These functions are found in lib.c */
 void remove_test_ENOTDIR_files(void);
 void remove_test_ENOENT_files(void);
 void test_ENAMETOOLONG_path(char *, int (*)(const char *), int);
 void test_ENAMETOOLONG_name(char *, int (*)(const char *), int);
 void test_ENOENT_empty(char *, int (*)(const char *), int);
 void test_ENOTDIR(char *, int (*)(const char *), int);
 void test_ENOENT_nofile(char *, int (*)(const char *), int);

#endif	/* _NFTW_H_ */
