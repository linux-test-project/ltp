/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2009 Cyril Hrubis chrubis@suse.cz
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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
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

/* $Id: test.h,v 1.26 2010/01/10 22:27:15 yaberauneya Exp $ */

#ifndef __TEST_H__
#define __TEST_H__

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "compiler.h"

/* Use low 6 bits to encode test type */
#define TTYPE_MASK 0x3f
#define TPASS      0    /* Test passed flag */
#define TFAIL      1    /* Test failed flag */
#define TBROK      2    /* Test broken flag */
#define TWARN      4    /* Test warning flag */
#define TRETR      8    /* Test retire flag */
#define TINFO      16   /* Test information flag */
#define TCONF      32   /* Test not appropriate for configuration flag */
#define TTYPE_RESULT(ttype) ((ttype) & TTYPE_MASK)

#define TERRNO     0x100   /* Append errno information to output */
#define TTERRNO    0x200   /* Append TEST_ERRNO information to output */

/*
 * To determine if you are on a Umk or Unicos system,
 * use sysconf(_SC_CRAY_SYSTEM).  But since _SC_CRAY_SYSTEM
 * is not defined until 90, it will be define here if not already
 * defined.
 * if ( sysconf(_SC_CRAY_SYSTEM) == 1 )
 *    on UMK
 * else   # returned 0 or -1
 *    on Unicos
 * This is only being done on CRAY systems.
 */
#ifdef CRAY
#ifndef _SC_CRAY_SYSTEM
#define _SC_CRAY_SYSTEM  140
#endif /* ! _SC_CRAY_SYSTEM */
#endif /* CRAY */

/*
 * Ensure that NUMSIGS is defined.
 * It should be defined in signal.h or sys/signal.h on
 * UNICOS/mk and IRIX systems.   On UNICOS systems,
 * it is not defined, thus it is being set to UNICOS's NSIG.
 * Note:  IRIX's NSIG (signals are 1-(NSIG-1))
 *      is not same meaning as UNICOS/UMK's NSIG  (signals 1-NSIG)
 */
#ifndef NUMSIGS
#define NUMSIGS NSIG
#endif


/* defines for unexpected signal setup routine (set_usig.c) */
#define FORK    1		/* SIGCLD is to be ignored */
#define NOFORK  0		/* SIGCLD is to be caught */
#define DEF_HANDLER SIG_ERR	/* tells set_usig() to use default signal handler */

/*
 * The following defines are used to control tst_res and t_result reporting.
 */

#define TOUTPUT	   "TOUTPUT"		/* The name of the environment variable */
					/* that can be set to one of the following */
					/* strings to control tst_res output */
					/* If not set, TOUT_VERBOSE_S is assumed */

#define TOUT_VERBOSE_S  "VERBOSE"	/* All test cases reported */
#define TOUT_CONDENSE_S "CONDENSE"	/* ranges are used where identical messages*/
					/* occur for sequential test cases */
#define TOUT_NOPASS_S   "NOPASS"	/* No pass test cases are reported */
#define TOUT_DISCARD_S  "DISCARD"	/* No output is reported */

#define TST_NOBUF	"TST_NOBUF"	/* The name of the environment variable */
					/* that can be set to control whether or not */
					/* tst_res will buffer output into 4096 byte */
					/* blocks of output */
					/* If not set, buffer is done.  If set, no */
					/* internal buffering will be done in tst_res */
					/* t_result does not have internal buffering */

/*
 * The following defines are used to control tst_tmpdir, tst_wildcard and t_mkchdir
 */

#define TDIRECTORY  "TDIRECTORY"	/* The name of the environment variable */
					/* that if is set, the value (directory) */
					/* is used by all tests as their working */
					/* directory.  tst_rmdir and t_rmdir will */
					/* not attempt to clean up. */
					/* This environment variable should only */
					/* be set when doing system testing since */
					/* tests will collide and break and fail */
					/* because of setting it. */

#define TEMPDIR	"/tmp"			/* This is the default temporary directory. */
					/* The environment variable TMPDIR is */
					/* used prior to this valid by tempnam(3). */
					/* To control the base location of the */
					/* temporary directory, set the TMPDIR */
					/* environment variable to desired path */

/*
 * The following define contains the name of an environmental variable
 * that can be used to specify the number of iterations.
 * It is supported in parse_opts.c and USC_setup.c.
 */
#define USC_ITERATION_ENV       "USC_ITERATIONS"

/*
 * The following define contains the name of an environmental variable
 * that can be used to specify to iteration until desired time
 * in floating point seconds has gone by.
 * Supported in USC_setup.c.
 */
#define USC_LOOP_WALLTIME	"USC_LOOP_WALLTIME"

/*
 * The following define contains the name of an environmental variable
 * that can be used to specify that no functional checks are wanted.
 * It is supported in parse_opts.c and USC_setup.c.
 */
#define USC_NO_FUNC_CHECK	"USC_NO_FUNC_CHECK"

/*
 * The following define contains the name of an environmental variable
 * that can be used to specify the delay between each loop iteration.
 * The value is in seconds (fractional numbers are allowed).
 * It is supported in parse_opts.c.
 */
#define USC_LOOP_DELAY		"USC_LOOP_DELAY"

/*
 * fork() can't be used on uClinux systems, so use FORK_OR_VFORK instead,
 * which will run vfork() on uClinux.
 * mmap() doesn't support MAP_PRIVATE on uClinux systems, so use
 * MAP_PRIVATE_EXCEPT_UCLINUX instead, which will skip the option on uClinux.
 * If MAP_PRIVATE really is required, the test can not be run on uClinux.
 */
#ifdef UCLINUX
#define FORK_OR_VFORK			vfork
#define MAP_PRIVATE_EXCEPT_UCLINUX	0
#else
#define FORK_OR_VFORK			fork
#define MAP_PRIVATE_EXCEPT_UCLINUX	MAP_PRIVATE
#endif

/*
 * Functions from lib/tst_res.c
 */
const char *strttype(int ttype);
void tst_res(int ttype, char *fname, char *arg_fmt, ...)
	__attribute__ ((format (printf, 3, 4)));
void tst_resm(int ttype, char *arg_fmt, ...)
	__attribute__ ((format (printf, 2, 3)));
void tst_brk(int ttype, char *fname, void (*func)(void), char *arg_fmt, ...)
	__attribute__ ((format (printf, 4, 5)));
void tst_brkloop(int ttype, char *fname, void (*func)(void), char *arg_fmt, ...)
	__attribute__ ((format (printf, 4, 5)));
void tst_brkm(int ttype, void (*func)(void), char *arg_fmt, ...)
	__attribute__ ((format (printf, 3, 4)));
void tst_brkloopm(int ttype, void (*func)(void), char *arg_fmt, ...)
	__attribute__ ((format (printf, 3, 4)));
void tst_require_root(void (*func)(void));
int  tst_environ(void);
void tst_exit(void) LTP_ATTRIBUTE_NORETURN;
void tst_flush(void);

extern int Tst_count;

/*
 * Function from lib/tst_sig.c
 */
void tst_sig(int fork_flag, void (*handler)(), void (*cleanup)());

/*
 * Functions from lib/tst_tmpdir.c
 */
void tst_tmpdir(void);
void tst_rmdir(void);

/*
 * Function from lib/get_high_address.c
 */
char *get_high_address(void);

/*
 * Functions from lib/tst_kvercmp.c
 */
void tst_getkver(int *k1, int *k2, int *k3);
int tst_kvercmp(int r1, int r2, int r3);

/*
 * Function from lib/tst_is_cwd.c
 */
int tst_is_cwd_nfs(void);
int tst_is_cwd_tmpfs(void);
int tst_is_cwd_ramfs(void);

/*
 * Function from lib/tst_cwd_has_free.c
 */
int tst_cwd_has_free(int required_kib);

/*
 * Functions from lib/self_exec.c
 */
void maybe_run_child(void (*child)(), char *fmt, ...);
int self_exec(char *argv0, char *fmt, ...);

/*
 * Functions from lib/cloner.c
 */
int ltp_clone(unsigned long clone_flags, int (*fn)(void *arg), void *arg,
		size_t stack_size, void *stack);
int ltp_clone_malloc(unsigned long clone_flags, int (*fn)(void *arg),
		void *arg, size_t stacksize);
int ltp_clone_quick(unsigned long clone_flags, int (*fn)(void *arg),
		void *arg);
#define clone(...) use_the_ltp_clone_functions,do_not_use_clone

/*
 * Functions from lib/mount_utils.c
 */
char *get_block_device(const char *path);
char *get_mountpoint(const char *path);

#ifdef TST_USE_COMPAT16_SYSCALL
#define TCID_BIT_SUFFIX "_16"
#elif  TST_USE_NEWER64_SYSCALL
#define TCID_BIT_SUFFIX "_64"
#else
#define TCID_BIT_SUFFIX ""
#endif
#define TCID_DEFINE(ID) char *TCID = (#ID TCID_BIT_SUFFIX)

#endif	/* __TEST_H__ */
