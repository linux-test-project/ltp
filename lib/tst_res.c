/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    AUTHOR            : Kent Rogers (from Dave Fenner's original)
 *    CO-PILOT          : Rich Logan
 *    DATE STARTED      : 05/01/90 (rewritten 1/96)
 * Copyright (c) 2009-2016 Cyril Hrubis <chrubis@suse.cz>
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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

#define _GNU_SOURCE

#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "test.h"
#include "safe_macros.h"
#include "usctest.h"
#include "ltp_priv.h"
#include "tst_ansi_color.h"

long TEST_RETURN;
int TEST_ERRNO;
void *TST_RET_PTR;

#define VERBOSE      1
#define NOPASS       3
#define DISCARD      4

#define MAXMESG      80		/* max length of internal messages */
#define USERMESG     2048	/* max length of user message */
#define TRUE         1
#define FALSE        0

/*
 * EXPAND_VAR_ARGS - Expand the variable portion (arg_fmt) of a result
 *                   message into the specified string.
 *
 * NOTE (garrcoop):  arg_fmt _must_ be the last element in each function
 *		     argument list that employs this.
 */
#define EXPAND_VAR_ARGS(buf, arg_fmt, buf_len) do {\
	va_list ap;				\
	assert(arg_fmt != NULL);		\
	va_start(ap, arg_fmt);			\
	vsnprintf(buf, buf_len, arg_fmt, ap);	\
	va_end(ap);				\
	assert(strlen(buf) > 0);		\
} while (0)

#if !defined(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP) && defined(__ANDROID__)
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP PTHREAD_RECURSIVE_MUTEX_INITIALIZER
#endif

#ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
static pthread_mutex_t tmutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#else
static pthread_mutex_t tmutex;

__attribute__((constructor))
static void init_tmutex(void)
{
	pthread_mutexattr_t mutattr = {0};

	pthread_mutexattr_init(&mutattr);
	pthread_mutexattr_settype(&mutattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&tmutex, &mutattr);
	pthread_mutexattr_destroy(&mutattr);
}
#endif

static void check_env(void);
static void tst_condense(int tnum, int ttype, const char *tmesg);
static void tst_print(const char *tcid, int tnum, int ttype, const char *tmesg);

static int T_exitval = 0;	/* exit value used by tst_exit() */
static int passed_cnt;
static int T_mode = VERBOSE;	/* flag indicating print mode: VERBOSE, */
				/* NOPASS, DISCARD */

static char Warn_mesg[MAXMESG];	/* holds warning messages */

/*
 * These are used for condensing output when NOT in verbose mode.
 */
static int Buffered = FALSE;	/* TRUE if condensed output is currently */
				/* buffered (i.e. not yet printed) */
static char *Last_tcid;		/* previous test case id */
static int Last_num;		/* previous test case number */
static int Last_type;		/* previous test result type */
static char *Last_mesg;		/* previous test result message */

int tst_count = 0;

/*
 * These globals must be defined in the test.
 */
extern char *TCID;		/* Test case identifier from the test source */
extern int TST_TOTAL;		/* Total number of test cases from the test */


struct pair {
	const char *name;
	int val;
};

#define PAIR(def) [def] = {.name = #def, .val = def},
#define STRPAIR(key, value) [key] = {.name = value, .val = key},

#define PAIR_LOOKUP(pair_arr, idx) do {                       \
	if (idx < 0 || (size_t)idx >= ARRAY_SIZE(pair_arr) || \
	    pair_arr[idx].name == NULL)                       \
		return "???";                                 \
	return pair_arr[idx].name;                            \
} while (0)

const char *strttype(int ttype)
{
	static const struct pair ttype_pairs[] = {
		PAIR(TPASS)
		PAIR(TFAIL)
		PAIR(TBROK)
		PAIR(TCONF)
		PAIR(TWARN)
		PAIR(TINFO)
	};

	PAIR_LOOKUP(ttype_pairs, TTYPE_RESULT(ttype));
}

#include "errnos.h"
#include "signame.h"

static void tst_res__(const char *file, const int lineno, int ttype,
                      const char *arg_fmt, ...)
{
	pthread_mutex_lock(&tmutex);

	char tmesg[USERMESG];
	int len = 0;
	int ttype_result = TTYPE_RESULT(ttype);

	if (ttype_result == TDEBUG) {
		printf("%s: %i: TDEBUG is not supported\n", __func__, __LINE__);
		abort();
	}

	if (file && (ttype_result != TPASS && ttype_result != TINFO))
		len = sprintf(tmesg, "%s:%d: ", file, lineno);
	EXPAND_VAR_ARGS(tmesg + len, arg_fmt, USERMESG - len);

	/*
	 * Save the test result type by ORing ttype into the current exit
	 * value (used by tst_exit()).
	 */
	T_exitval |= ttype_result;

	if (ttype_result == TPASS)
		passed_cnt++;

	check_env();

	/*
	 * Set the test case number and print the results, depending on the
	 * display type.
	 */
	if (ttype_result == TWARN || ttype_result == TINFO) {
		tst_print(TCID, 0, ttype, tmesg);
	} else {
		if (tst_count < 0)
			tst_print(TCID, 0, TWARN,
				  "tst_res(): tst_count < 0 is not valid");

		/*
		 * Process each display type.
		 */
		switch (T_mode) {
		case DISCARD:
			break;
		case NOPASS:	/* filtered by tst_print() */
			tst_condense(tst_count + 1, ttype, tmesg);
			break;
		default:	/* VERBOSE */
			tst_print(TCID, tst_count + 1, ttype, tmesg);
			break;
		}

		tst_count++;
	}

	pthread_mutex_unlock(&tmutex);
}

static void tst_condense(int tnum, int ttype, const char *tmesg)
{
	int ttype_result = TTYPE_RESULT(ttype);

	/*
	 * If this result is the same as the previous result, return.
	 */
	if (Buffered == TRUE) {
		if (strcmp(Last_tcid, TCID) == 0 && Last_type == ttype_result &&
		    strcmp(Last_mesg, tmesg) == 0)
			return;

		/*
		 * This result is different from the previous result.  First,
		 * print the previous result.
		 */
		tst_print(Last_tcid, Last_num, Last_type, Last_mesg);
		free(Last_tcid);
		free(Last_mesg);
	}

	/*
	 * If a file was specified, print the current result since we have no
	 * way of retaining the file contents for comparing with future
	 * results.  Otherwise, buffer the current result info for next time.
	 */
	Last_tcid = malloc(strlen(TCID) + 1);
	strcpy(Last_tcid, TCID);
	Last_num = tnum;
	Last_type = ttype_result;
	Last_mesg = malloc(strlen(tmesg) + 1);
	strcpy(Last_mesg, tmesg);
	Buffered = TRUE;
}

void tst_old_flush(void)
{
	NO_NEWLIB_ASSERT("Unknown", 0);

	pthread_mutex_lock(&tmutex);

	/*
	 * Print out last line if in NOPASS mode.
	 */
	if (Buffered == TRUE && T_mode == NOPASS) {
		tst_print(Last_tcid, Last_num, Last_type, Last_mesg);
		Buffered = FALSE;
	}

	fflush(stdout);

	pthread_mutex_unlock(&tmutex);
}

static void tst_print(const char *tcid, int tnum, int ttype, const char *tmesg)
{
	int err = errno;
	const char *type;
	int ttype_result = TTYPE_RESULT(ttype);
	char message[USERMESG];
	size_t size = 0;

	/*
	 * Save the test result type by ORing ttype into the current exit value
	 * (used by tst_exit()).  This is already done in tst_res(), but is
	 * also done here to catch internal warnings.  For internal warnings,
	 * tst_print() is called directly with a case of TWARN.
	 */
	T_exitval |= ttype_result;

	/*
	 * If output mode is DISCARD, or if the output mode is NOPASS and this
	 * result is not one of FAIL, BROK, or WARN, just return.  This check
	 * is necessary even though we check for DISCARD mode inside of
	 * tst_res(), since occasionally we get to this point without going
	 * through tst_res() (e.g. internal TWARN messages).
	 */
	if (T_mode == DISCARD || (T_mode == NOPASS && ttype_result != TFAIL &&
				  ttype_result != TBROK
				  && ttype_result != TWARN))
		return;

	/*
	 * Build the result line and print it.
	 */
	type = strttype(ttype);

	if (T_mode == VERBOSE) {
		size += snprintf(message + size, sizeof(message) - size,
				"%-8s %4d  ", tcid, tnum);
	} else {
		size += snprintf(message + size, sizeof(message) - size,
				"%-8s %4d       ", tcid, tnum);
	}

	if (size >= sizeof(message)) {
		printf("%s: %i: line too long\n", __func__, __LINE__);
		abort();
	}

	if (tst_color_enabled(STDOUT_FILENO))
		size += snprintf(message + size, sizeof(message) - size,
		"%s%s%s  :  %s", tst_ttype2color(ttype), type, ANSI_COLOR_RESET, tmesg);
	else
		size += snprintf(message + size, sizeof(message) - size,
		"%s  :  %s", type, tmesg);

	if (size >= sizeof(message)) {
		printf("%s: %i: line too long\n", __func__, __LINE__);
		abort();
	}

	if (ttype & TERRNO) {
		size += snprintf(message + size, sizeof(message) - size,
				 ": errno=%s(%i): %s", tst_strerrno(err),
				 err, strerror(err));
	}

	if (size >= sizeof(message)) {
		printf("%s: %i: line too long\n", __func__, __LINE__);
		abort();
	}

	if (ttype & TTERRNO) {
		size += snprintf(message + size, sizeof(message) - size,
				 ": TEST_ERRNO=%s(%i): %s",
				 tst_strerrno(TEST_ERRNO), (int)TEST_ERRNO,
				 strerror(TEST_ERRNO));
	}

	if (size >= sizeof(message)) {
		printf("%s: %i: line too long\n", __func__, __LINE__);
		abort();
	}

	if (ttype & TRERRNO) {
		err = TEST_RETURN < 0 ? -(int)TEST_RETURN : (int)TEST_RETURN;
		size += snprintf(message + size, sizeof(message) - size,
				 ": TEST_RETURN=%s(%i): %s",
				 tst_strerrno(err), err, strerror(err));
	}

	if (size + 1 >= sizeof(message)) {
		printf("%s: %i: line too long\n", __func__, __LINE__);
		abort();
	}

	message[size] = '\n';
	message[size + 1] = '\0';

	fputs(message, stdout);
}

static void check_env(void)
{
	static int first_time = 1;
	char *value;

	if (!first_time)
		return;

	first_time = 0;

	/* BTOUTPUT not defined, use default */
	if ((value = getenv(TOUTPUT)) == NULL) {
		T_mode = VERBOSE;
		return;
	}

	if (strcmp(value, TOUT_NOPASS_S) == 0) {
		T_mode = NOPASS;
		return;
	}

	if (strcmp(value, TOUT_DISCARD_S) == 0) {
		T_mode = DISCARD;
		return;
	}

	T_mode = VERBOSE;
	return;
}

void tst_exit(void)
{
	NO_NEWLIB_ASSERT("Unknown", 0);

	pthread_mutex_lock(&tmutex);

	tst_old_flush();

	T_exitval &= ~TINFO;

	if (T_exitval == TCONF && passed_cnt)
		T_exitval &= ~TCONF;

	exit(T_exitval);
}

pid_t tst_fork(void)
{
	pid_t child;

	NO_NEWLIB_ASSERT("Unknown", 0);

	tst_old_flush();

	child = fork();
	if (child == 0)
		T_exitval = 0;

	return child;
}

void tst_record_childstatus(void (*cleanup)(void), pid_t child)
{
	int status, ttype_result;

	NO_NEWLIB_ASSERT("Unknown", 0);

	SAFE_WAITPID(cleanup, child, &status, 0);

	if (WIFEXITED(status)) {
		ttype_result = WEXITSTATUS(status);
		ttype_result = TTYPE_RESULT(ttype_result);
		T_exitval |= ttype_result;

		if (ttype_result == TPASS)
			tst_resm(TINFO, "Child process returned TPASS");

		if (ttype_result & TFAIL)
			tst_resm(TINFO, "Child process returned TFAIL");

		if (ttype_result & TBROK)
			tst_resm(TINFO, "Child process returned TBROK");

		if (ttype_result & TCONF)
			tst_resm(TINFO, "Child process returned TCONF");

	} else {
		tst_brkm(TBROK, cleanup, "child process(%d) killed by "
			 "unexpected signal %s(%d)", child,
			 tst_strsig(WTERMSIG(status)), WTERMSIG(status));
	}
}

/*
 * Make tst_brk reentrant so that one can call the SAFE_* macros from within
 * user-defined cleanup functions.
 */
static int tst_brk_entered = 0;

static void tst_brk__(const char *file, const int lineno, int ttype,
                      void (*func)(void), const char *arg_fmt, ...)
{
	pthread_mutex_lock(&tmutex);

	char tmesg[USERMESG];
	int ttype_result = TTYPE_RESULT(ttype);

	EXPAND_VAR_ARGS(tmesg, arg_fmt, USERMESG);

	/*
	 * Only FAIL, BROK, CONF, and RETR are supported by tst_brk().
	 */
	if (ttype_result != TFAIL && ttype_result != TBROK &&
	    ttype_result != TCONF) {
		sprintf(Warn_mesg, "%s: Invalid Type: %d. Using TBROK",
			__func__, ttype_result);
		tst_print(TCID, 0, TWARN, Warn_mesg);
		/* Keep TERRNO, TTERRNO, etc. */
		ttype = (ttype & ~ttype_result) | TBROK;
	}

	tst_res__(file, lineno, ttype, "%s", tmesg);
	if (tst_brk_entered == 0) {
		if (ttype_result == TCONF) {
			tst_res__(file, lineno, ttype,
				"Remaining cases not appropriate for "
				"configuration");
		} else if (ttype_result == TBROK) {
			tst_res__(file, lineno, TBROK,
				 "Remaining cases broken");
		}
	}

	/*
	 * If no cleanup function was specified, just return to the caller.
	 * Otherwise call the specified function.
	 */
	if (func != NULL) {
		tst_brk_entered++;
		(*func) ();
		tst_brk_entered--;
	}
	if (tst_brk_entered == 0)
		tst_exit();

	pthread_mutex_unlock(&tmutex);
}

void tst_resm_(const char *file, const int lineno, int ttype,
	const char *arg_fmt, ...)
{
	char tmesg[USERMESG];

	EXPAND_VAR_ARGS(tmesg, arg_fmt, USERMESG);

	if (tst_test)
		tst_res_(file, lineno, ttype, "%s", tmesg);
	else
		tst_res__(file, lineno, ttype, "%s", tmesg);
}

typedef void (*tst_res_func_t)(const char *file, const int lineno,
		int ttype, const char *fmt, ...);

void tst_resm_hexd_(const char *file, const int lineno, int ttype,
	const void *buf, size_t size, const char *arg_fmt, ...)
{
	char tmesg[USERMESG];
	static const size_t symb_num	= 2; /* xx */
	static const size_t size_max	= 16;
	size_t offset;
	size_t i;
	char *pmesg = tmesg;
	tst_res_func_t res_func;

	if (tst_test)
		res_func = tst_res_;
	else
		res_func = tst_res__;

	EXPAND_VAR_ARGS(tmesg, arg_fmt, USERMESG);
	offset = strlen(tmesg);

	if (size > size_max || size == 0 ||
		(offset + size * (symb_num + 1)) >= USERMESG)
		res_func(file, lineno, ttype, "%s", tmesg);
	else
		pmesg += offset;

	for (i = 0; i < size; ++i) {
		/* add space before byte except first one */
		if (pmesg != tmesg)
			*(pmesg++) = ' ';

		sprintf(pmesg, "%02x", ((unsigned char *)buf)[i]);
		pmesg += symb_num;
		if ((i + 1) % size_max == 0 || i + 1 == size) {
			res_func(file, lineno, ttype, "%s", tmesg);
			pmesg = tmesg;
		}
	}
}

void tst_brkm__(const char *file, const int lineno, int ttype,
	void (*func)(void), const char *arg_fmt, ...)
{
	char tmesg[USERMESG];

	EXPAND_VAR_ARGS(tmesg, arg_fmt, USERMESG);

	if (tst_test) {
		if (func) {
			tst_brk_(file, lineno, TBROK,
			         "Non-NULL cleanup in newlib!");
		}

		tst_brk_(file, lineno, ttype, "%s", tmesg);
	}

	tst_brk__(file, lineno, ttype, func, "%s", tmesg);

	/* Shouldn't be reached, but fixes build time warnings about noreturn. */
	abort();
}

void tst_require_root(void)
{
	NO_NEWLIB_ASSERT("Unknown", 0);

	if (geteuid() != 0)
		tst_brkm(TCONF, NULL, "Test needs to be run as root");
}
