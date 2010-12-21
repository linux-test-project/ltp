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


/* $Id: tst_res.c,v 1.14 2009/12/01 08:57:20 yaberauneya Exp $ */

/**********************************************************
 *
 *    OS Testing - Silicon Graphics, Inc.
 *
 *    FUNCTION NAME     :
 *      tst_res() -       Print result message (include file contents)
 *      tst_resm() -      Print result message
 *      tst_brk() -       Print result message (include file contents)
 *                        and break remaining test cases
 *      tst_brkm() -      Print result message and break remaining test
 *                        cases
 *      tst_flush() -     Print any messages pending in the output stream
 *      tst_exit() -      Exit test with a meaningful exit value.
 *      tst_environ() -   Keep results coming to original stdout
 *
 *    FUNCTION TITLE    : Standard automated test result reporting mechanism
 *
 *    SYNOPSIS:
 *      #include "test.h"
 *
 *      void tst_res(ttype, fname, tmesg [,arg]...)
 *      int  ttype;
 *      char *fname;
 *      char *tmesg;
 *
 *      void tst_resm(ttype, tmesg [,arg]...)
 *      int  ttype;
 *      char *tmesg;
 *
 *      void tst_brk(ttype, fname, cleanup, tmesg, [,argv]...)
 *      int  ttype;
 *      char *fname;
 *      void (*cleanup)();
 *      char *tmesg;
 *
 *      void tst_brkm(ttype, cleanup, tmesg [,arg]...)
 *      int  ttype;
 *      void (*cleanup)();
 *      char *tmesg;
 *
 *      void tst_flush()
 *
 *      void tst_exit()
 *
 *      int  tst_environ()
 *
 *    AUTHOR            : Kent Rogers (from Dave Fenner's original)
 *
 *    CO-PILOT          : Rich Logan
 *
 *    DATE STARTED      : 05/01/90 (rewritten 1/96)
 *
 *    MAJOR CLEANUPS BY : Cyril Hrubis
 *
 *    DESCRIPTION
 *      See the man page(s).
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

/* Break bad habits. */
#ifdef GARRETT_IS_A_PEDANTIC_BASTARD
pid_t		spawned_program_pid;
#endif

#define VERBOSE      1     /* flag values for the T_mode variable */
#define NOPASS       3
#define DISCARD      4

#define MAXMESG      80    /* max length of internal messages */
#define USERMESG     2048  /* max length of user message */
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

/*
 * Define local function prototypes.
 */
static void check_env(void);
static void tst_condense(int tnum, int ttype, char *tmesg);
static void tst_print(char *tcid, int tnum, int ttype, char *tmesg);
static void cat_file(char *filename);

/*
 * Define some static/global variables.
 */
static FILE *T_out = NULL;    /* tst_res() output file descriptor */
static char *File;            /* file whose contents is part of result */
static int  T_exitval = 0;    /* exit value used by tst_exit() */
static int  T_mode = VERBOSE; /* flag indicating print mode: VERBOSE, */
                              /* NOPASS, DISCARD */

static char Warn_mesg[MAXMESG];  /* holds warning messages */

/*
 * These are used for condensing output when NOT in verbose mode.
 */
static int  Buffered = FALSE; /* TRUE if condensed output is currently */
                              /* buffered (i.e. not yet printed) */
static char *Last_tcid;       /* previous test case id */
static int  Last_num;         /* previous test case number */
static int  Last_type;        /* previous test result type */
static char *Last_mesg;       /* previous test result message */


/*
 * These globals may be externed by the test.
 */
int Tst_count = 0;      /* current count of test cases executed; NOTE: */
                        /* Tst_count may be externed by other programs */

/*
 * These globals must be defined in the test.
 */
extern char *TCID;      /* Test case identifier from the test source */
extern int  TST_TOTAL;  /* Total number of test cases from the test */
                        /* source */

struct pair {
	const char *name;
	int val;
};
#define PAIR(def) [def] = { .name = #def, .val = def, },
const char *pair_lookup(struct pair *pair, int pair_size, int idx)
{
	if (idx < 0 || idx >= pair_size || pair[idx].name == NULL)
		return "???";
	return pair[idx].name;
}
#define pair_lookup(pair, idx) pair_lookup(pair, ARRAY_SIZE(pair), idx)

/*
 * strttype() - convert a type result to the human readable string
 */
const char *strttype(int ttype)
{
	struct pair ttype_pairs[] = {
		PAIR(TPASS)
		PAIR(TFAIL)
		PAIR(TBROK)
		PAIR(TRETR)
		PAIR(TCONF)
		PAIR(TWARN)
		PAIR(TINFO)
	};
	return pair_lookup(ttype_pairs, TTYPE_RESULT(ttype));
}

/*
 * strerrnodef() - convert an errno value to its C define
 */
static const char *strerrnodef(int err)
{
	struct pair errno_pairs[] = {
		PAIR(EPERM)
		PAIR(ENOENT)
		PAIR(ESRCH)
		PAIR(EINTR)
		PAIR(EIO)
		PAIR(ENXIO)
		PAIR(E2BIG)
		PAIR(ENOEXEC)
		PAIR(EBADF)
		PAIR(ECHILD)
		PAIR(EAGAIN)
		PAIR(ENOMEM)
		PAIR(EACCES)
		PAIR(EFAULT)
		PAIR(ENOTBLK)
		PAIR(EBUSY)
		PAIR(EEXIST)
		PAIR(EXDEV)
		PAIR(ENODEV)
		PAIR(ENOTDIR)
		PAIR(EISDIR)
		PAIR(EINVAL)
		PAIR(ENFILE)
		PAIR(EMFILE)
		PAIR(ENOTTY)
		PAIR(ETXTBSY)
		PAIR(EFBIG)
		PAIR(ENOSPC)
		PAIR(ESPIPE)
		PAIR(EROFS)
		PAIR(EMLINK)
		PAIR(EPIPE)
		PAIR(EDOM)
		PAIR(ERANGE)
		PAIR(ENAMETOOLONG)
	};
	return pair_lookup(errno_pairs, err);
}

/*
 * tst_res() - Main result reporting function.  Handle test information
 *             appropriately depending on output display mode.  Call
 *             tst_condense() or tst_print() to actually print results.
 *             All result functions (tst_resm(), tst_brk(), etc.)
 *             eventually get here to print the results.
 */
void tst_res(int ttype, char *fname, char *arg_fmt, ...)
{
	char tmesg[USERMESG];
	int ttype_result = TTYPE_RESULT(ttype);

#if DEBUG
	printf("IN tst_res; Tst_count = %d\n", Tst_count);
	fflush(stdout);
#endif

	EXPAND_VAR_ARGS(tmesg, arg_fmt, USERMESG);

	/*
	 * Save the test result type by ORing ttype into the current exit
	 * value (used by tst_exit()).
	 */
	T_exitval |= ttype_result;

	/*
	 * Unless T_out has already been set by tst_environ(), make tst_res()
	 * output go to standard output.
	 */
	if (T_out == NULL)
		T_out = stdout;

	/*
	 * Check TOUTPUT environment variable (if first time) and set T_mode
	 * flag.
	 */
	check_env();

	if (fname != NULL && access(fname, F_OK) == 0)
		File = fname;

	/*
	 * Set the test case number and print the results, depending on the
	 * display type.
	 */
	if (ttype_result == TWARN || ttype_result == TINFO) {
		tst_print(TCID, 0, ttype, tmesg);
	} else {
		if (Tst_count < 0)
			tst_print(TCID, 0, TWARN,
			    "tst_res(): Tst_count < 0 is not valid");

		/*
		 * Process each display type.
		 */
		switch (T_mode) {
		case DISCARD:
			break;
		case NOPASS: /* filtered by tst_print() */
			tst_condense(Tst_count+1, ttype, tmesg);
			break;
		default:      /* VERBOSE */
			tst_print(TCID, Tst_count+1, ttype, tmesg);
			break;
		}

		Tst_count++;
	}

}


/*
 * tst_condense() - Handle test cases in NOPASS mode (i.e.
 *                  buffer the current result and print the last result
 *                  if different than the current).  If a file was
 *                  specified, print the current result and do not
 *                  buffer it.
 */
static void tst_condense(int tnum, int ttype, char *tmesg)
{
	char *file;
	int ttype_result = TTYPE_RESULT(ttype);

#if DEBUG
	printf( "IN tst_condense: tcid = %s, tnum = %d, ttype = %d, "
		"tmesg = %s\n",
		TCID, tnum, ttype, tmesg);
	fflush(stdout);
#endif

	/*
	 * If this result is the same as the previous result, return.
	 */
	if (Buffered == TRUE) {
		if (strcmp(Last_tcid, TCID) == 0 && Last_type == ttype_result &&
		    strcmp(Last_mesg, tmesg) == 0 && File == NULL)
			return;

		/*
		 * This result is different from the previous result.  First,
		 * print the previous result.
		 */
		file = File;
		File = NULL;
		tst_print(Last_tcid, Last_num, Last_type, Last_mesg);
		free(Last_tcid);
		free(Last_mesg);
		File = file;
	}

	/*
	 * If a file was specified, print the current result since we have no
	 * way of retaining the file contents for comparing with future
	 * results.  Otherwise, buffer the current result info for next time.
	 */
	if (File != NULL) {
		tst_print(TCID, tnum, ttype, tmesg);
		Buffered = FALSE;
	} else {
		Last_tcid = (char *)malloc(strlen(TCID) + 1);
		strcpy(Last_tcid, TCID);
		Last_num = tnum;
		Last_type = ttype_result;
		Last_mesg = (char *)malloc(strlen(tmesg) + 1);
		strcpy(Last_mesg, tmesg);
		Buffered = TRUE;
	}
}


/*
 * tst_flush() - Print any messages pending because due to tst_condense,
 *               and flush T_out.
 */
void tst_flush(void)
{
#if DEBUG
	printf("IN tst_flush\n");
	fflush(stdout);
#endif

	/*
	 * Print out last line if in NOPASS mode.
	 */
	if (Buffered == TRUE && T_mode == NOPASS) {
		tst_print(Last_tcid, Last_num, Last_type, Last_mesg);
		Buffered = FALSE;
	}

	fflush(T_out);
}


/*
 * tst_print() - Print a line to the output stream.
 */
static void tst_print(char *tcid, int tnum, int ttype, char *tmesg)
{
	/*
	 * avoid unintended side effects from failures with fprintf when
	 * calling write(2), et all.
	 */
	int err = errno;
	const char *type;
	int ttype_result = TTYPE_RESULT(ttype);

#ifdef GARRETT_IS_A_PEDANTIC_BASTARD
	/* Don't execute these APIs unless you have the same pid as main! */
	if (spawned_program_pid != 0) {
		/*
		 * Die quickly and noisily so people get the cluebat that the
		 * test needs to be fixed. These APIs should _not_ be called
		 * from forked processes because of the fact that it can confuse
		 * end-users with printouts, cleanup will potentially blow away
		 * directories and/or files still in use introducing
		 * non-determinism, etc.
		 *
		 * assert will not return (by design in accordance with POSIX
		 * 1003.1) if the assertion fails. Read abort(3) for more
		 * details. So don't worry about saving / restoring the signal
		 * handler, unless you have a buggy OS that you've hacked 15
		 * different ways to Sunday.
		 */
		assert(spawned_program_pid == getpid());
	}
#endif

#if DEBUG
	printf("IN tst_print: tnum = %d, ttype = %d, tmesg = %s\n",
	    tnum, ttype, tmesg);
	fflush(stdout);
#endif

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
	    ttype_result != TBROK && ttype_result != TWARN))
		return;

	/*
	 * Build the result line and print it.
	 */
	type = strttype(ttype);
	if (T_mode == VERBOSE) {
		fprintf(T_out, "%-8s %4d  %s  :  %s", tcid, tnum, type, tmesg);
	} else {
		fprintf(T_out, "%-8s %4d       %s  :  %s",
			tcid, tnum, type, tmesg);
	}
	if (ttype & TERRNO) {
		fprintf(T_out, ": errno=%s(%i): %s", strerrnodef(err),
			err, strerror(err));
	}
	if (ttype & TTERRNO) {
		fprintf(T_out, ": TEST_ERRNO=%s(%i): %s",
			strerrnodef(TEST_ERRNO), (int)TEST_ERRNO,
			strerror(TEST_ERRNO));
	}
	fprintf(T_out, "\n");

	/*
	 * If tst_res() was called with a file, append file contents to the
	 * end of last printed result.
	 */
	if (File != NULL)
		cat_file(File);

	File = NULL;
}


/*
 * check_env() - Check the value of the environment variable TOUTPUT and
 *               set the global variable T_mode.  The TOUTPUT environment
 *               variable should be set to "VERBOSE", "NOPASS", or "DISCARD".
 *               If TOUTPUT does not exist or is not set to a valid value, the
 *               default is "VERBOSE".
 */
static void check_env(void)
{
	static int first_time = 1;
	char      *value;

#if DEBUG
	printf("IN check_env\n");
	fflush(stdout);
#endif

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

	/* default */
	T_mode = VERBOSE;
	return;
}


/*
 * tst_exit() - Call exit() with the value T_exitval, set up by
 *              tst_res().  T_exitval has a bit set for most of the
 *              result types that were seen (including TPASS, TFAIL,
 *              TBROK, TWARN, TCONF).  Also, print the last result (if
 *              necessary) before exiting.
 */
void tst_exit(void)
{
#if DEBUG
	printf("IN tst_exit\n"); fflush(stdout);
	fflush(stdout);
#endif

	/* Call tst_flush() flush any output in the buffer. */
	tst_flush();

	/* Mask out TRETR, TINFO, and TCONF results from the exit status. */
	exit(T_exitval & ~(TRETR | TINFO | TCONF));
}


/*
 * tst_environ() - Preserve the tst_res() output location, despite any
 *                 changes to stdout.
 */
int tst_environ(void)
{
	if ((T_out = fdopen(dup(fileno(stdout)), "w")) == NULL)
		return -1;
	else
		return 0;
}


/*
 * Make tst_brk reentrant so that one can call the SAFE_* macros from within
 * user-defined cleanup functions.
 */
static int tst_brk_entered = 0;

/*
 * tst_brk() - Fail or break current test case, and break the remaining
 *             tests cases.
 */
void tst_brk(int ttype, char *fname, void (*func)(void), char *arg_fmt, ...)
{
	char tmesg[USERMESG];
	int ttype_result = TTYPE_RESULT(ttype);

#if DEBUG
	printf("IN tst_brk\n"); fflush(stdout);
	fflush(stdout);
#endif

	EXPAND_VAR_ARGS(tmesg, arg_fmt, USERMESG);

	/*
	 * Only FAIL, BROK, CONF, and RETR are supported by tst_brk().
	 */
	if (ttype_result != TFAIL && ttype_result != TBROK &&
	    ttype_result != TCONF && ttype_result != TRETR) {
		sprintf(Warn_mesg, "%s: Invalid Type: %d. Using TBROK",
			__func__, ttype_result);
		tst_print(TCID, 0, TWARN, Warn_mesg);
		/* Keep TERRNO, TTERRNO, etc. */
		ttype = (ttype & ~ttype_result) | TBROK;
	}

	tst_res(ttype, fname, "%s", tmesg);
	if (tst_brk_entered == 0) {
		if (ttype_result == TCONF)
			tst_res(ttype, NULL,
			    "Remaining cases not appropriate for "
			    "configuration");
		else if (ttype_result == TRETR)
			tst_res(ttype, NULL, "Remaining cases retired");
		else if (ttype_result == TBROK)
			tst_res(TBROK, NULL, "Remaining cases broken");
	}

	/*
	 * If no cleanup function was specified, just return to the caller.
	 * Otherwise call the specified function.
	 */
	if (func != NULL) {
		tst_brk_entered++;
		(*func)();
		tst_brk_entered--;
	}
	if (tst_brk_entered == 0)
		tst_exit();

}

/*
 * tst_resm() - Interface to tst_res(), with no filename.
 */
void tst_resm(int ttype, char *arg_fmt, ...)
{
	char tmesg[USERMESG];

#if DEBUG
	printf("IN tst_resm\n"); fflush(stdout);
	fflush(stdout);
#endif

	EXPAND_VAR_ARGS(tmesg, arg_fmt, USERMESG);

	tst_res(ttype, NULL, "%s", tmesg);
}


/*
 * tst_brkm() - Interface to tst_brk(), with no filename.
 */
void tst_brkm(int ttype, void (*func)(void), char *arg_fmt, ...)
{
	char tmesg[USERMESG];

#if DEBUG
	printf("IN tst_brkm\n"); fflush(stdout);
	fflush(stdout);
#endif

	EXPAND_VAR_ARGS(tmesg, arg_fmt, USERMESG);

	tst_brk(ttype, NULL, func, "%s", tmesg);
}


/*
 * tst_require_root() - Test for root permissions and abort if not.
 */
void tst_require_root(void (*func)(void))
{
	if (geteuid() != 0)
		tst_brkm(TCONF, func, "Test needs to be run as root");
}


/*
 * cat_file() - Print the contents of a file to standard out.
 */
static void cat_file(char *filename)
{
	FILE *fp;
	int  b_read, b_written;
	char buffer[BUFSIZ];

#if DEBUG
	printf("IN cat_file\n"); fflush(stdout);
#endif

	if ((fp = fopen(filename, "r")) == NULL) {
		sprintf(Warn_mesg,
			"tst_res(): fopen(%s, \"r\") failed; errno = %d: %s",
			filename, errno, strerror(errno));
			tst_print(TCID, 0, TWARN, Warn_mesg);
		return;
	}

	errno = 0;

	while ((b_read = fread(buffer, 1, BUFSIZ, fp)) != 0) {
		if ((b_written = fwrite(buffer, 1, b_read, T_out)) != b_read) {
			sprintf(Warn_mesg,
				"tst_res(): While trying to cat \"%s\", "
				"fwrite() wrote only %d of %d bytes",
				filename, b_written, b_read);
			tst_print(TCID, 0, TWARN, Warn_mesg);
			break;
		}
	}

	if (!feof(fp)) {
		sprintf(Warn_mesg,
			"tst_res(): While trying to cat \"%s\", fread() "
			"failed, errno = %d: %s",
			filename, errno, strerror(errno));
		tst_print(TCID, 0, TWARN, Warn_mesg);
	}

	if (fclose(fp) != 0) {
		sprintf(Warn_mesg,
			"tst_res(): While trying to cat \"%s\", fclose() "
			"failed, errno = %d: %s",
			filename, errno, strerror(errno));
		tst_print(TCID, 0, TWARN, Warn_mesg);
	}
}


#ifdef UNIT_TEST
/****************************************************************************
 * Unit test code: Takes input from stdin and can make the following
 *                 calls: tst_res(), tst_resm(), tst_brk(), tst_brkm(),
 *                 tst_flush_buf(), tst_exit().
 ****************************************************************************/
int  TST_TOTAL = 10;
char *TCID = "TESTTCID";

#define RES  "tst_res.c UNIT TEST message; ttype = %d; contents of \"%s\":"
#define RESM "tst_res.c UNIT TEST message; ttype = %d"

int main(void)
{
	int  ttype;
	char chr;
	char fname[MAXMESG];

	printf("UNIT TEST of tst_res.c.  Options for ttype:\n\
	       -1 : call tst_exit()\n\
	       -2 : call tst_flush()\n\
	       -3 : call tst_brk()\n\
	       -4 : call tst_res()\n\
	       %2i : call tst_res(TPASS, ...)\n\
	       %2i : call tst_res(TFAIL, ...)\n\
	       %2i : call tst_res(TBROK, ...)\n\
	       %2i : call tst_res(TWARN, ...)\n\
	       %2i : call tst_res(TRETR, ...)\n\
	       %2i : call tst_res(TINFO, ...)\n\
	       %2i : call tst_res(TCONF, ...)\n\n",
		TPASS, TFAIL, TBROK, TWARN, TRETR, TINFO, TCONF);

	while (1) {
		printf("Enter ttype (-5,-4,-3,-2,-1,%i,%i,%i,%i,%i,%i,%i): ",
			TPASS, TFAIL, TBROK, TWARN, TRETR, TINFO, TCONF);
		scanf("%d%c", &ttype, &chr);

		switch (ttype) {
		case -1:
			tst_exit();
			break;

		case -2:
			tst_flush();
			break;

		case -3:
			printf("Enter the current type (%i=FAIL, %i=BROK, %i=RETR, %i=CONF): ",
				TFAIL, TBROK, TRETR, TCONF);
			scanf("%d%c", &ttype, &chr);
			printf("Enter file name (<cr> for none): ");
			gets(fname);
			if (strcmp(fname, "") == 0)
				tst_brkm(ttype, tst_exit, RESM, ttype);
			else
				tst_brk(ttype, fname, tst_exit, RES, ttype, fname);
		break;

		case -4:
			printf("Enter the current type (%i,%i,%i,%i,%i,%i,%i): ",
				TPASS, TFAIL, TBROK, TWARN, TRETR, TINFO, TCONF);
			scanf("%d%c", &ttype, &chr);
		default:
			printf("Enter file name (<cr> for none): ");
			gets(fname);

			if (strcmp(fname, "") == 0)
				tst_resm(ttype, RESM, ttype);
			else
				tst_res(ttype, fname, RES, ttype, fname);
			break;
		}

	}
}

#endif /* UNIT_TEST */
