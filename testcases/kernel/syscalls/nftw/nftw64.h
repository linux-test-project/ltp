/* IBM Corporation */
/* 10/31/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001   Port to Linux   nsharoff@us.ibm.com */


/*
 * NAME
 *	nftw64.h - Header file for nftw64.c
 */
/* $Copyright: $
 * Copyright (c) 1984-1999
 * Sequent Computer Systems, Inc.   All rights reserved.
 *  
 * This software is furnished under a license and may be used
 * only in accordance with the terms of that license and with the
 * inclusion of the above copyright notice.   This software may not
 * be provided or otherwise made available to, or used by, any
 * other person.  No title to or ownership of the software is
 * hereby transferred.
 */

/* $Header: /cvsroot/ltp/ltp/testcases/kernel/syscalls/nftw/nftw64.h,v 1.2 2009/08/28 10:18:24 vapier Exp $ */


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

#include "test.h"
#include "usctest.h"


#define STRLEN          512
#define MAX_FD          20
#define MAXOPENDIRS     1024    /* max opendirs to try to exhaust dir streams */
#define NUM_2_VISIT     4
#define RET_VAL         666
#define NDIRLISTENTS    100
#define ERR_BUF_SIZ     4096
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

extern void fail_exit(void);

/* These functions are found in test.c */
extern void test1A(void);
extern void test2A(void);
extern void test3A(void);
extern void test4A(void);
extern void test5A(void);
extern void test6A(void);
extern void test7A(void);
extern void test8A(void);
extern void test9A(void);
extern void test10A(void);
extern void test11A(void);
extern void test12A(void);
extern void test13A(void);
extern void test14A(void);
extern void test15A(void);
extern void test16A(void);
extern void test17A(void);
extern void test18A(void);
extern void test19A(void);
extern void test20A(void);
extern void test21A(void);
extern void test22A(void);
extern void test23A(void);
extern void test24A(void);
extern void test25A(void);
extern void test26A(void);
extern void test27A(void);
extern void test28A(void);
extern void test29A(void);
extern void test30A(void);

/* These functions are found in test_func.c */
extern int test_func1(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func3(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func4(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func5(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func7(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func8(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func9(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func10(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func11(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func12(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func13(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func14(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func15(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func16(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func17(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func18(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func19(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func20(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func21(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func22(const char *, const struct stat64 *, int, struct FTW *);
extern int test_func23(const char *, const struct stat64 *, int, struct FTW *);

/* These functions are found in tools.c */
extern void cleanup_function(void);
extern void setup_path(void);
extern int nftw64_fn(const char *, const struct stat64 *, int, struct FTW *);
extern char * ftw_mnemonic(int);
extern int getbase(const char *);
extern int getlev(const char *);
extern void do_info(const char *);

/* These functions are found in lib.c */
extern void remove_test_ENOTDIR_files(void);
extern void remove_test_ENOENT_files(void);
extern void test_ENAMETOOLONG_path(char *, int (*)(const char *), int);
extern void test_ENAMETOOLONG_name(char *, int (*)(const char *), int);
extern void test_ENOENT_empty(char *, int (*)(const char *), int);
extern void test_ENOTDIR(char *, int (*)(const char *), int);
extern void test_ENOENT_nofile(char *, int (*)(const char *), int);


#endif	/* _NFTW_H_ */
