/*	@(#)tlock.c	1.21 2003/12/30 Connectathon Testsuite	*/
/*
 *	System V NFS
 *
 *	Copyright 1986, 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
 *
 *	All Rights Reserved.
 *
 *	The copyright above and this notice must be preserved in all
 *	copies of this source code.  The copyright above does not
 *	evidence any actual or intended publication of this source
 *	code.
 *
 *	This is unpublished proprietary trade secret source code of
 *	Lachman Associates.  This source code may not be copied,
 *	disclosed, distributed, demonstrated or licensed except as
 *	expressly authorized by Lachman Associates.
 */

/*
 * #ifndef lint
 * static char SysVr3NFSID[] = "@(#)tlock.c	4.11 System V NFS source";
 * #endif
 */

/*
 * Test program for record locking.  If USE_LOCKF is defined, lockf is used
 * (System V, Release 3).  Otherwise, fcntl is used (Posix).
 */

/*
 * Large file support, added June 1996
 *
 * These tests exercise the large file locking capabilities defined
 * by the large file summit proposal as defined in the final draft:
 * http://www.sas.com:80/standards/large.file/x_open.20Mar96.html
 *
 * The tests done may be modified by the following flags:
 *	LF_SUMMIT:	accept EOVERFLOW in place of EINVAL where
 *			large file summit spec calls for that, rather
 *			than warning about it.
 *	LARGE_LOCKS:	use 64-bit API to test locking in [0-2^^63]
 *			range rather than the [0-2^^31] range.  This flag
 *			should not be used if the OS supports a native
 *			64-bit API (e.g., Alpha).
 */

#if defined(LARGE_LOCKS)
#undef _FILE_OFFSET_BITS
#define	_FILE_OFFSET_BITS	64
#else
#if !defined(_FILE_OFFSET_BITS) && !defined(__alpha) && !defined(__sparcv9)
#define	_FILE_OFFSET_BITS	32
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#ifdef STDARG
#include <stdarg.h>
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/times.h>
#ifdef MMAP
#include <sys/mman.h>
#endif
#include <inttypes.h>

#if defined(LARGE_LOCKS) && !defined(_LFS64_LARGEFILE) && !defined(MACOSX)
This machine cannot compile the 64 bit version of this test using the
LARGE_LOCKS symbol.
If your machine uses native 64-bit offsets, you should not define
LARGE_LOCKS.
#endif

#ifndef HZ
#define	HZ	60		/* a common default */
#endif

static off_t maxeof;

#define	PARENT	0		/* Who am I? */
#define	CHILD	1

#define	PASS	0		/* Passed test. */
#define	EQUAL	-1		/* Compared equal. */
#define	UNEQUAL	-2		/* Compared unequal. */

#define	WARN	1		/* Warning, may be a problem. */
#define	FATAL	2		/* Fatal testing error. */

#define	END	(0)		/* A bit more readable. */

#define	COMMENT		1	/* write_testfile */
#define	NO_COMMENT	0

#define	DO_TEST(n)	((testnum == 0) || (testnum == (n)))
#define	DO_RATE(n)	((ratetest > 0) || (testnum == (n)))
#define	DO_MAND(n)	((mandtest > 0) || (testnum == (n)))

#define	DO_UNLINK	1
#define	JUST_CLOSE	0

/*
 * Size of read/write buffer for test14, in bytes.  Should be reasonably
 * large compared to the default file size (iorate_kb).  Must be big
 * enough to hold the test strings used in test7 and test9.
 */
#define	IORATE_BUFSIZE	32768

static int ratetest = 0;
static int ratecount = 1000;		/* test 8 */
static int mandtest = 0;

static int iorate_kb = 256;		/* test 14 */
static int iorate_count = 10;		/* test 14 */

static int wait_time = 3;

static char arr[1];		/* dummy buffer for pipe */

static int parentpipe[2];
static int childpipe[2];
static int pidpipe[2];

static char testfile[256];	/* file for locking test */
static char *filepath = ".";
static int testfd;
#ifdef MMAP
static caddr_t mappedaddr;	/* address file is mapped to (some tests) */
static off_t mappedlen;
#endif

/*
 * Protocol version.  Assume v3 unless told otherwise.  Version 2 doesn't
 * support large files, which restricts some of the tests that can be run.
 */
static int proto_vers = 3;

static int testnum;	/* current test number */
static int passnum;	/* current pass number */
static int passcnt;	/* repeat pass count */
static int cumpass;	/* pass for all passes */
static int cumwarn;	/* warn for all passes */
static int cumfail;	/* fail for all passes */
static int tstpass;	/* pass for last pass */
static int tstwarn;	/* warn for last pass */
static int tstfail;	/* fail for last pass */

static int parentpid, childpid;
static int who;

#ifdef O_SYNC
#define	OPENFLAGS	(O_CREAT | O_RDWR | O_SYNC)
#else
#define	OPENFLAGS	(O_CREAT | O_RDWR)
#endif
#define	OPENMODES	(0666)
#define	MANDMODES	(02666)

/*
 * Some tests try to provoke a failure by seeking to a bad value.  The
 * specific errno value depends on whether the Large File Summit API is
 * being followed.
 */
#ifdef LF_SUMMIT
static int oflow_err = EOVERFLOW;
#else
static int oflow_err = EINVAL;
#endif

/*
 * If a lock request is denied because of a conflicting lock, System V
 * defines the return value to be EAGAIN.  BSD UNIX uses EACCES.  Posix
 * allows either. IRIX is generally SVR4-like, but in this case uses EACCES.
 */
#if defined(SVR4) && !defined(IRIX)
static int denied_err = EAGAIN;
#else
static int denied_err = EACCES;
#endif

#ifdef BSD
#define	SIGCLD	SIGCHLD
#endif

#ifndef ARGS_
#ifdef __STDC__
#define	ARGS_(x) x
#else
#define	ARGS_(x) ()
#endif
#endif

static void close_testfile ARGS_((int));
#ifdef STDARG
static void comment(char *fmt, ...);
#else
static void comment();
#endif

static void
initialize()
{
	maxeof = (off_t)1 << (sizeof (off_t) * 8 - 2);
	maxeof += maxeof - 1;

	/*
	 * NFS Version 2 only supports 32-bit offsets.  This has a couple
	 * consequences.  First, we have to throttle back maxeof to what
	 * the protocol supports.  Second, the Large File Summit defines
	 * EOVERFLOW for locking in terms of what can be represented by
	 * off_t.  This means that an overflow error with v2 on a 64-bit
	 * system will return EINVAL, not EOVERFLOW.
	 */
	if (proto_vers == 2) {
		off_t orig_maxeof = maxeof;

		maxeof = 0x7fffffff;
		if (maxeof < orig_maxeof)
			oflow_err = EINVAL;
	}

	parentpid = getpid();
	sprintf(&testfile[0], "%s/lockfile%d", filepath, parentpid);

	printf("Creating parent/child synchronization pipes.\n");
	pipe(parentpipe);
	pipe(childpipe);
	pipe(pidpipe);

	fflush(stdout);

#ifdef MMAP
#ifdef MACOSX
	mappedlen = getpagesize();
#else
	mappedlen = sysconf(_SC_PAGESIZE);
#endif
#endif
}

static void
testreport(nok)
	int nok;
{
	FILE *outf;
	char *sp;

	cumpass += tstpass;
	cumwarn += tstwarn;
	cumfail += tstfail;
	outf = (nok ? stderr : stdout);
	sp = ((who == PARENT) ? "PARENT" : " CHILD");
	fprintf(outf, "\n** %s pass %d results: ", sp, passnum);
	fprintf(outf, "%d/%d pass, %d/%d warn, %d/%d fail (pass/total).\n",
		tstpass, cumpass, tstwarn, cumwarn, tstfail, cumfail);
	tstpass = tstwarn = tstfail = 0;
	fflush(outf);
}

static void
testexit(nok)
	int nok;
{

	close_testfile(DO_UNLINK);
	if (nok) {
		testreport(1);
	}
	if (who == PARENT) {
		signal(SIGCLD, SIG_DFL);
		if (nok) {
			signal(SIGINT, SIG_IGN);
			kill(childpid, SIGINT);
		}
		wait((int *)0);
	} else {
		if (nok) {
			signal(SIGINT, SIG_IGN);
			kill(parentpid, SIGINT);
		}
	}
	exit(nok);
	/* NOTREACHED */
}

/* ARGSUSED */
static void
parentsig(sig)
	int sig;
{

	testexit(1);
}

/* ARGSUSED */
static void
childsig(sig)
	int sig;
{

	testexit(1);
}

/* ARGSUSED */
static void
childdied(sig)
	int sig;
{

	comment("Child died");
}

static void
header(test, string)
	int test;
	char *string;
{

	printf("\nTest #%d - %s", test, string);
	printf("\n");
	fflush(stdout);
}

#ifdef STDARG

static void
comment(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	printf("\t%s", ((who == PARENT) ? "Parent: " : "Child:  "));
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	printf("\n");
	fflush(stdout);
}

#else /* STDARG */

/* VARARGS1 */
static void
comment(fmt, arg1, arg2, arg3, arg4)
	char *fmt;
#ifdef OSF1
	void *arg1, *arg2, *arg3, *arg4;
#else
	int arg1, arg2, arg3, arg4;
#endif
{

	printf("\t%s", ((who == PARENT) ? "Parent: " : "Child:  "));
	printf(fmt, arg1, arg2, arg3, arg4);
	printf("\n");
	fflush(stdout);
}
#endif /* STDARG */

static void
childwait()
{

	if (read(parentpipe[0], arr, 1) != 1) {
		perror("tlock: child pipe read");
		testexit(1);
	}
}

static void
childfree(wait)
	int wait;
{

	if (write(parentpipe[1], arr, 1) != 1) {
		perror("tlock: childfree pipe write");
		testexit(1);
	}
	if (wait)
		sleep(wait);
}

static void
parentwait()
{

	if (read(childpipe[0], arr, 1) != 1) {
		perror("tlock: parentwait pipe read");
		testexit(1);
	}
}

static void
parentfree(wait)
	int wait;
{

	if (write(childpipe[1], arr, 1) != 1) {
		perror("tlock: child pipe write");
		testexit(1);
	}
	if (wait)
		sleep(wait);
}

static char tmpstr[16];

static char *
terrstr(err)
	int err;
{

	switch (err) {
	case UNEQUAL:
		return ("unequal");
	case EQUAL:
		return ("equal");
	case PASS:
		return ("success");
	case EAGAIN:
		return ("EAGAIN");
	case EBADF:
		return ("EBADF");
	case EACCES:
		return ("EACCES");
	case EFAULT:
		return ("EFAULT");
	case EINVAL:
		return ("EINVAL");
#ifdef EOVERFLOW
	case EOVERFLOW:
		return ("EOVERFLOW");
#endif
	case EFBIG:
		return ("EFBIG");
	case EDEADLK:
		return ("EDEADLK");
#ifdef ECOMM
	case ECOMM:
		return ("ECOMM");
#endif
#ifdef ENOLINK
	case ENOLINK:
		return ("ENOLINK");
#endif
	default:
		sprintf(tmpstr, "errno=%d", err);
		return (tmpstr);
	}
}

/*
 * Return a string representing the range starting at offset, with length
 * bytes.
 */

static char *
fmtrange(offset, length)
	off_t offset;
	off_t length;
{
	static char buf[256];

#ifdef LARGE_LOCKS			/* non-native 64-bit */
	if (length != 0)
		sprintf(buf, "[%16llx,%16llx] ", offset, length);
	else
		sprintf(buf, "[%16llx,          ENDING] ", offset);
#else /* LARGE_LOCKS */
	if (sizeof (offset) == 4) {
		if (length != 0)
			sprintf(buf, "[%8lx,%8lx] ", (int32_t)offset,
				(int32_t)length);
		else
			sprintf(buf, "[%8lx,  ENDING] ", (int32_t)offset);
	} else {
		if (length != 0)
			sprintf(buf, "[%16llx,%16llx] ", offset, length);
		else
			sprintf(buf, "[%16llx,          ENDING] ", offset);
	}
#endif /* LARGE_LOCKS */

	return (buf);
}

static void
report(num, sec, what, offset, length, pass, result, fail)
	int num;			/* test number */
	int sec;			/* test section */
	char *what;
	off_t offset;
	off_t length;
	int pass;			/* expected result */
	int result;			/* actual result */
	int fail;			/* fail or warning */
{

	printf("\t%s", ((who == PARENT) ? "Parent: " : "Child:  "));
	printf("%d.%-2d - %s %s", num, sec, what,
	    fmtrange(offset, length));
	if (pass == result) {
		printf("PASSED.\n");
		tstpass++;
	}
#ifdef EOVERFLOW
	else if (pass == EOVERFLOW && result == EINVAL) {
		printf("WARNING!\n");
		comment("**** Expected %s, returned %s...",
			terrstr(pass), terrstr(result));
		comment("**** Okay if expecting pre-large file semantics.");
		tstwarn++;
	} else if (pass == EINVAL && result == EOVERFLOW) {
		printf("WARNING!\n");
		comment("**** Expected %s, returned %s...",
			terrstr(pass), terrstr(result));
		comment("**** Okay if expecting large file semantics.");
		tstwarn++;
	}
#endif /* EOVERFLOW */
	else if (pass == EAGAIN && result == EACCES) {
		printf("WARNING!\n");
		comment("**** Expected %s, returned %s...",
			terrstr(pass), terrstr(result));
		comment("**** Probably BSD semantics instead of SVID.");
		tstwarn++;
	} else if (pass == EACCES && result == EAGAIN) {
		printf("WARNING!\n");
		comment("**** Expected %s, returned %s...",
			terrstr(pass), terrstr(result));
		comment("**** Probably SVID semantics instead of BSD.");
		tstwarn++;
	} else if (fail == WARN) {
		printf("WARNING!\n");
		comment("**** Expected %s, returned %s...",
			terrstr(pass), terrstr(result));
		tstwarn++;
	} else {
		printf("FAILED!\n");
		comment("**** Expected %s, returned %s...",
			terrstr(pass), terrstr(result));
		if (pass == PASS && result == EFBIG)
			comment("**** Filesystem doesn't support large files.");
		else
			comment("**** Probably implementation error.");
		tstfail++;
		testexit(1);
	}
	fflush(stdout);
}

static char *
tfunstr(fun)
	int fun;
{

	switch (fun) {
	case F_ULOCK:
		return ("F_ULOCK");
	case F_LOCK:
		return ("F_LOCK ");
	case F_TLOCK:
		return ("F_TLOCK");
	case F_TEST:
		return ("F_TEST ");
	default:
		fprintf(stderr, "tlock: unknown lockf() F_<%d>.\n", fun);
		testexit(1);
	}
	/* NOTREACHED */
}

static void
open_testfile(flags, modes)
	int flags;
	int modes;
{

	testfd = open(testfile, flags, modes);
	if (testfd < 0) {
		perror("tlock: open");
		testexit(1);
	}
}

static void
close_testfile(cleanup)
	int cleanup;
{

	if (cleanup == JUST_CLOSE)
		comment("Closed testfile.");
	close(testfd);
	if (cleanup == DO_UNLINK)
		(void) unlink(testfile);
}

static void
write_testfile(datap, offset, count, do_comment)
	char *datap;
	off_t offset;
	int count;
	int do_comment;
{
	int result;

	(void) lseek(testfd, offset, 0);
	result = write(testfd, datap, count);
	if (result < 0) {
		perror("tlock: testfile write");
		testexit(1);
	}
	if (result != count) {
		fprintf(stderr, "tlock: short write (got %d, expected %d)\n",
			result, count);
		testexit(1);
	}

	if (do_comment) {
#ifdef LARGE_LOCKS
		comment("Wrote '%.40s' to testfile [ %lld, %d ].",
			datap, offset, count);
#else
		comment("Wrote '%.40s' to testfile [ %ld, %d ].",
			datap, offset, count);
#endif
	}
}

/*
 * Read count bytes from the given offset.  If datap is non-null, make sure
 * the bytes read are the same as what datap points to.
 */

static void
read_testfile(test, sec, datap, offset, count, pass, fail)
	int test;
	int sec;
	char *datap;
	off_t offset;
	unsigned int count;
	int pass;
	int fail;
{
	int result;
	static char *array = NULL;

	if (array == NULL)
		array = malloc(IORATE_BUFSIZE);
	if (array == NULL) {
		perror("read_testfile: can't allocate buffer");
		exit(1);
	}

	(void) lseek(testfd, offset, 0);
	if (count > IORATE_BUFSIZE)
		count = IORATE_BUFSIZE;
	result = read(testfd, array, count);
	if (result < 0) {
		perror("tlock: testfile read");
		testexit(1);
	}
	if (result != count) {
		fprintf(stderr, "tlock: short read (got %d, expected %d)\n",
			result, count);
		testexit(1);
	}

	if (datap != NULL) {
#ifdef LARGE_LOCKS
		comment("Read '%.40s' from testfile [ %lld, %d ].",
			datap, offset, count);
#else
		comment("Read '%.40s' from testfile [ %ld, %d ].",
			datap, offset, count);
#endif
		array[count] = '\0';
		if (strncmp(datap, array, count) != 0) {
			comment("**** Test expected '%s', read '%s'.",
				datap, array);
			result = UNEQUAL;
		} else {
			result = EQUAL;
		}
		report(test, sec, "COMPARE", offset, (off_t)count, pass,
		    result, fail);
	}
}

static void
testdup2(fd1, fd2)
	int fd1;
	int fd2;
{

	if (dup2(fd1, fd2) < 0) {
		perror("tlock: dup2");
		testexit(1);
	}
}

static void
testtruncate()
{

	comment("Truncated testfile.");
	if (ftruncate(testfd, (off_t)0) < 0) {
		perror("tlock: ftruncate");
		testexit(1);
	}
}

#ifdef MMAP

#ifndef MAP_FAILED
#define	MAP_FAILED	(-1)
#endif

/*
 * Try to mmap testfile.  It's not necessarily a fatal error if the request
 * fails.  Returns 0 on success and sets mappedaddr to the mapped address.
 * Returns an errno value on failure.
 */
static int
testmmap()
{
	mappedaddr = mmap(0, mappedlen, PROT_READ | PROT_WRITE, MAP_SHARED,
			testfd, (off_t)0);
	if (mappedaddr == (caddr_t)MAP_FAILED)
		return (errno);
	return (0);
}

static void
testmunmap()
{
	comment("unmap testfile.");
	if (munmap(mappedaddr, mappedlen) < 0) {
		perror("Can't unmap testfile.");
		testexit(1);
	}
	mappedaddr = (caddr_t)0xdeadbeef;
}
#endif /* MMAP */

#ifdef USE_LOCKF

static void
test(num, sec, func, offset, length, pass, fail)
	int num;			/* test number */
	int sec;			/* section number */
	int func;			/* lockf function to invoke */
	off_t offset;			/* starting offset of lock */
	off_t length;			/* length of lock */
	int pass;			/* expected return code */
	int fail;			/* error vs warning */
{
	int result = PASS;

	/*
	 * Don't ignore lseek errors.  If the lseek fails, we won't be
	 * testing what you'd think we are testing just looking at the
	 * test() calls.
	 */
	if (lseek(testfd, offset, 0) < 0) {
		result = errno;
	}
	if (result == 0) {
		if ((result = lockf(testfd, func, length)) != 0) {
			if (result != -1) {
				fprintf(stderr, "tlock: lockf() returned %d.\n",
					result);
				testexit(1);
			}
			result = errno;
		}
	}
	report(num, sec, tfunstr(func), offset, length, pass, result, fail);
}

#else /* USE_LOCKF */

/*
 * Map a lockf function to the corresponding fcntl locking function and
 * command.   Prints an error message and exits if the function if invalid.
 */

static void
lockf2fcntl(func, typep, cmdp)
	int func;
	short *typep;			/* OUT */
	int *cmdp;			/* OUT */
{
	switch (func) {
	case F_ULOCK:
		*typep = F_UNLCK;
		*cmdp = F_SETLK;
		break;
	case F_LOCK:
		*typep = F_WRLCK;
		*cmdp = F_SETLKW;
		break;
	case F_TLOCK:
		*typep = F_WRLCK;
		*cmdp = F_SETLK;
		break;
	case F_TEST:
		*typep = F_WRLCK;
		*cmdp = F_GETLK;
		break;
	default:
		fprintf(stderr, "invalid lockf function: %d\n", func);
		testexit(1);
		break;
	}
}

static void
test(num, sec, func, offset, length, pass, fail)
	int num;			/* test number */
	int sec;			/* section number */
	int func;			/* lockf function to invoke */
	off_t offset;			/* starting offset of lock */
	off_t length;			/* length of lock */
	int pass;			/* expected return code */
	int fail;			/* error vs warning */
{
	struct flock request;
	int result = PASS;
	int fcntlcmd;

	lockf2fcntl(func, &request.l_type, &fcntlcmd);
	request.l_whence = 0;
	request.l_start = offset;
	request.l_len = length;

	if (fcntl(testfd, fcntlcmd, &request) < 0)
		result = errno;
	else if (func == F_TEST && request.l_type != F_UNLCK)
		result = denied_err;
	report(num, sec, tfunstr(func), offset, length, pass, result,
	    fail);
}

#endif /* USE_LOCKF */

static void
rate(cnt)
	int cnt;
{
	int i;
	long beg;
	long end;
	struct tms tms;
	long delta;

	beg = times(&tms);
	for (i = 0; i < cnt; i++) {
		if ((lockf(testfd, F_LOCK, 1) != 0) ||
		    (lockf(testfd, F_ULOCK, 1) != 0)) {
			fprintf(stderr, "tlock: rate error=%d.\n", errno);
			tstfail++;
			break;
		}
	}
	/* See makefile: Sun must use SysV times()! */
	end = times(&tms);
	delta = ((end - beg) * 1000) / HZ;
	if (delta == 0) {
		fprintf(stderr, "tlock: rate time=0.\n");
		return;
	}
	comment("Performed %d lock/unlock cycles in %d msecs. [%d lpm].",
		i, delta, (i * 120000) / delta);
	fflush(stdout);
}

static void
iorate(kb, count)
	int kb;
	int count;
{
	int i;
	long beg;
	long end;
	struct tms tms;
	long delta;
	static char *buf = NULL;
	long msec_sum = 0;
	long msec_sqsum = 0;
	double mean_time;
	double stddev_time;

	if (buf == NULL)
		buf = malloc(IORATE_BUFSIZE);
	if (buf == NULL) {
		perror("iorate: can't allocate buffer");
		exit(1);
	}

	for (i = 0; i < IORATE_BUFSIZE; i++) {
		buf[i] = 'a' + (i % 26);
	}

	for (i = 0; i < count; i++) {
		off_t xferred;

		beg = times(&tms);
		testtruncate();

		for (xferred = 0;
		    xferred < kb * 1024;
		    xferred += IORATE_BUFSIZE) {
			write_testfile(buf, xferred, IORATE_BUFSIZE,
				    NO_COMMENT);
		}
		for (xferred = 0;
		    xferred < kb * 1024;
		    xferred += IORATE_BUFSIZE) {
			read_testfile(14, 0, (char *)NULL, xferred,
				    IORATE_BUFSIZE, 0, 0);
		}

		end = times(&tms);
		delta = ((end - beg) * 1000) / HZ;
		msec_sum += delta;
		msec_sqsum += delta * delta;
	}

	mean_time = msec_sum / (1000.0 * count);
	stddev_time = 1.0 * count * msec_sqsum - 1.0 * msec_sum * msec_sum;
	stddev_time /= (count * (count - 1));
	stddev_time = sqrt(stddev_time);

#define	TO_THRUPUT(t)	(kb * 2.0 / (t))

	comment("Wrote and read %d KB file %d times; [%.2lf +/- %.2lf KB/s].",
		kb, count, TO_THRUPUT(mean_time), TO_THRUPUT(stddev_time));
	fflush(stdout);
}

static void
test1()
{

	if (who == PARENT) {
		parentwait();
		open_testfile(OPENFLAGS, OPENMODES);
		header(1, "Test regions of an unlocked file.");
		test(1, 1, F_TEST, (off_t)0, (off_t)1, PASS, FATAL);
		test(1, 2, F_TEST, (off_t)0, (off_t)END, PASS, FATAL);
		test(1, 3, F_TEST, (off_t)0, maxeof, PASS, FATAL);
		test(1, 4, F_TEST, (off_t)1, (off_t)1, PASS, FATAL);
		test(1, 5, F_TEST, (off_t)1, (off_t)END, PASS, FATAL);
		test(1, 6, F_TEST, (off_t)1, maxeof, PASS, FATAL);
		test(1, 7, F_TEST, maxeof, (off_t)1, PASS, FATAL);
		test(1, 8, F_TEST, maxeof, (off_t)END, PASS, FATAL);
		/*
		 * Some systems might not rigorously enforce Posix
		 * restrictions on the offset and length (signed positive
		 * integer, no overflow).  So treat those failures as
		 * warnings, not fatal.
		 */
		test(1, 9, F_TEST, maxeof, maxeof, oflow_err,
			WARN);
		close_testfile(DO_UNLINK);
		childfree(0);
	} else {
		parentfree(0);
		childwait();
	}
}

static void
test2()
{

	if (who == PARENT) {
		parentwait();
		header(2, "Try to lock the whole file.");
		open_testfile(OPENFLAGS, OPENMODES);
		test(2, 0, F_TLOCK, (off_t)0, (off_t)END, PASS, FATAL);
		childfree(0);
		parentwait();
		test(2, 10, F_ULOCK, (off_t)0, (off_t)END, PASS, FATAL);
		close_testfile(DO_UNLINK);
	} else {
		parentfree(0);
		childwait();
		open_testfile(OPENFLAGS, OPENMODES);
		test(2, 1, F_TEST, (off_t)0, (off_t)1, denied_err, FATAL);
		test(2, 2, F_TEST, (off_t)0, (off_t)END, denied_err, FATAL);
		test(2, 3, F_TEST, (off_t)0, maxeof, denied_err, FATAL);
		test(2, 4, F_TEST, (off_t)1, (off_t)1, denied_err, FATAL);
		test(2, 5, F_TEST, (off_t)1, (off_t)END, denied_err, FATAL);
		test(2, 6, F_TEST, (off_t)1, maxeof, denied_err, FATAL);
		test(2, 7, F_TEST, maxeof, (off_t)1, denied_err, FATAL);
		test(2, 8, F_TEST, maxeof, (off_t)END, denied_err,
		    FATAL);
		test(2, 9, F_TEST, maxeof, maxeof, oflow_err,
			WARN);
		close_testfile(DO_UNLINK);
		parentfree(0);
	}
}

static void
test3()
{

	if (who == PARENT) {
		parentwait();
		header(3, "Try to lock just the 1st byte.");
		open_testfile(OPENFLAGS, OPENMODES);
		test(3, 0, F_TLOCK, (off_t)0, (off_t)1, PASS, FATAL);
		childfree(0);
		parentwait();
		test(3, 5, F_ULOCK, (off_t)0, (off_t)1, PASS, FATAL);
		close_testfile(DO_UNLINK);
	} else {
		parentfree(0);
		childwait();
		open_testfile(OPENFLAGS, OPENMODES);
		test(3, 1, F_TEST, (off_t)0, (off_t)1, denied_err, FATAL);
		test(3, 2, F_TEST, (off_t)0, (off_t)END, denied_err, FATAL);
		test(3, 3, F_TEST, (off_t)1, (off_t)1, PASS, FATAL);
		test(3, 4, F_TEST, (off_t)1, (off_t)END, PASS, FATAL);
		close_testfile(DO_UNLINK);
		parentfree(0);
	}
}

static void
test4()
{

	if (who == PARENT) {
		parentwait();
		header(4, "Try to lock the 2nd byte, test around it.");
		open_testfile(OPENFLAGS, OPENMODES);
		test(4, 0, F_TLOCK, (off_t)1, (off_t)1, PASS, FATAL);
		childfree(0);
		parentwait();
		test(4, 10, F_ULOCK, (off_t)1, (off_t)1, PASS, FATAL);
		close_testfile(DO_UNLINK);
	} else {
		parentfree(0);
		childwait();
		open_testfile(OPENFLAGS, OPENMODES);
		test(4, 1, F_TEST, (off_t)0, (off_t)1, PASS, FATAL);
		test(4, 2, F_TEST, (off_t)0, (off_t)2, denied_err, FATAL);
		test(4, 3, F_TEST, (off_t)0, (off_t)END, denied_err, FATAL);
		test(4, 4, F_TEST, (off_t)1, (off_t)1, denied_err, FATAL);
		test(4, 5, F_TEST, (off_t)1, (off_t)2, denied_err, FATAL);
		test(4, 6, F_TEST, (off_t)1, (off_t)END, denied_err, FATAL);
		test(4, 7, F_TEST, (off_t)2, (off_t)1, PASS, FATAL);
		test(4, 8, F_TEST, (off_t)2, (off_t)2, PASS, FATAL);
		test(4, 9, F_TEST, (off_t)2, (off_t)END, PASS, FATAL);
		close_testfile(DO_UNLINK);
		parentfree(0);
	}
}

static void
test5()
{

	if (who == PARENT) {
		parentwait();
		header(5, "Try to lock 1st and 2nd bytes, test around them.");
		open_testfile(OPENFLAGS, OPENMODES);
		test(5, 0, F_TLOCK, (off_t)0, (off_t)1, PASS, FATAL);
		test(5, 1, F_TLOCK, (off_t)2, (off_t)1, PASS, FATAL);
		childfree(0);
		parentwait();
		test(5, 14, F_ULOCK, (off_t)0, (off_t)1, PASS, FATAL);
		test(5, 15, F_ULOCK, (off_t)2, (off_t)1, PASS, FATAL);
		close_testfile(DO_UNLINK);
	} else {
		parentfree(0);
		childwait();
		open_testfile(OPENFLAGS, OPENMODES);
		test(5, 2, F_TEST, (off_t)0, (off_t)1, denied_err, FATAL);
		test(5, 3, F_TEST, (off_t)0, (off_t)2, denied_err, FATAL);
		test(5, 4, F_TEST, (off_t)0, (off_t)END, denied_err, FATAL);
		test(5, 5, F_TEST, (off_t)1, (off_t)1, PASS, FATAL);
		test(5, 6, F_TEST, (off_t)1, (off_t)2, denied_err, FATAL);
		test(5, 7, F_TEST, (off_t)1, (off_t)END, denied_err, FATAL);
		test(5, 8, F_TEST, (off_t)2, (off_t)1, denied_err, FATAL);
		test(5, 9, F_TEST, (off_t)2, (off_t)2, denied_err, FATAL);
		test(5, 10, F_TEST, (off_t)2, (off_t)END, denied_err, FATAL);
		test(5, 11, F_TEST, (off_t)3, (off_t)1, PASS, FATAL);
		test(5, 12, F_TEST, (off_t)3, (off_t)2, PASS, FATAL);
		test(5, 13, F_TEST, (off_t)3, (off_t)END, PASS, FATAL);
		close_testfile(DO_UNLINK);
		parentfree(0);
	}
}

static void
test6()
{
#ifdef LARGE_LOCKS
	unsigned long long maxplus1 = (unsigned long long)maxeof + 1;
#else
	unsigned long maxplus1 = (unsigned long)maxeof + 1;
#endif

	if (who == PARENT) {
		parentwait();
		header(6, "Try to lock the MAXEOF byte.");
		open_testfile(OPENFLAGS, OPENMODES);
		test(6, 0, F_TLOCK, maxeof, (off_t)1, PASS, FATAL);
		childfree(0);
		parentwait();
		test(6, 11, F_ULOCK, maxeof, (off_t)1, PASS, FATAL);
		close_testfile(DO_UNLINK);
	} else {
		parentfree(0);
		childwait();
		open_testfile(OPENFLAGS, OPENMODES);
		test(6, 1, F_TEST, maxeof - 1, (off_t)1, PASS, FATAL);
		test(6, 2, F_TEST, maxeof - 1, (off_t)2, denied_err,
		    FATAL);
		test(6, 3, F_TEST, maxeof - 1, (off_t)END,
		    denied_err, FATAL);
		test(6, 4, F_TEST, maxeof, (off_t)1, denied_err, FATAL);
		test(6, 5, F_TEST, maxeof, (off_t)2, oflow_err, WARN);
		test(6, 6, F_TEST, maxeof, (off_t)END, denied_err,
		    FATAL);
		test(6, 7, F_TEST, (off_t)maxplus1, (off_t)END, EINVAL,
		    WARN);
		test(6, 8, F_TEST, (off_t)maxplus1, (off_t)1, EINVAL,
		    WARN);
		test(6, 9, F_TEST, (off_t)maxplus1, maxeof, EINVAL,
		    WARN);
		test(6, 10, F_TEST, (off_t)maxplus1, (off_t)maxplus1, EINVAL,
		    WARN);
		close_testfile(DO_UNLINK);
		parentfree(0);
	}
}

static void
test7()
{
	/*
	 * Considerations for strings:
	 * 1. the lengths should not be multiples of 4, and the parent
	 *    length should be different from the child length, in order to
	 *    exercise the XDR code as well as possible.
	 * 2. make the child's string longer than the parent's, to ensure
	 *    that the file length gets extended correctly.
	 */
	char *parent_msg = "aaaa eh";
	size_t parent_len = strlen(parent_msg);
	char *child_msg = "bebebebeb";
	size_t child_len = strlen(child_msg);
	int locklen;

	off_t pagesize;
	off_t start;

	locklen = (parent_len > child_len ? parent_len : child_len);

	/*
	 * Try to get the I/O requests to cross a page boundary.  If the
	 * system can't tell us how big a page is, default to something
	 * big, like 64KB.
	 */
#ifdef MACOSX
	pagesize = getpagesize();
#else
	pagesize = sysconf(_SC_PAGESIZE);
#endif
	if (pagesize < 0) {
		pagesize = 64 * 1024;
	}
	start = pagesize - 4;

	if (who == PARENT) {
		parentwait();
		header(7, "Test parent/child mutual exclusion.");
		open_testfile(OPENFLAGS, OPENMODES);
		test(7, 0, F_TLOCK, start, (off_t)locklen, PASS, FATAL);
		write_testfile(parent_msg, start, (int)parent_len, COMMENT);
		comment("Now free child to run, should block on lock.");
		childfree(wait_time);
		comment("Check data in file to insure child blocked.");
		read_testfile(7, 1, parent_msg, start, parent_len,
			    EQUAL, FATAL);
		comment("Now unlock region so child will unblock.");
		test(7, 2, F_ULOCK, start, (off_t)locklen, PASS, FATAL);
		parentwait();	/* wait for child to claim it has lock */
		comment("Now try to regain lock, parent should block.");
		test(7, 5, F_LOCK, start, (off_t)locklen, PASS, FATAL);
		comment("Check data in file to insure child unblocked.");
		read_testfile(7, 6, child_msg, start, child_len,
			    EQUAL, FATAL);
		test(7, 7, F_ULOCK, start, (off_t)locklen, PASS, FATAL);
		close_testfile(DO_UNLINK);
		childfree(0);
	} else {
		parentfree(0);
		childwait();
		open_testfile(OPENFLAGS, OPENMODES);
		test(7, 3, F_LOCK, start, (off_t)locklen, PASS, FATAL);
		parentfree(0);
		comment("Write child's version of the data and release lock.");
		write_testfile(child_msg, start, (int)child_len, COMMENT);
		test(7, 4, F_ULOCK, start, (off_t)locklen, PASS, FATAL);
		close_testfile(DO_UNLINK);
		childwait();
	}
}

static void
test8()
{

	if (who == PARENT) {
		parentwait();
		header(8, "Rate test performing lock/unlock cycles.");
		open_testfile(OPENFLAGS, OPENMODES);
		rate(ratecount);
		close_testfile(DO_UNLINK);
		childfree(0);
	} else {
		parentfree(0);
		childwait();
	}
}

static void
test9()
{

	if (who == PARENT) {
		parentwait();
		header(9, "Test mandatory locking (LAI NFS client).");
		open_testfile(OPENFLAGS, MANDMODES);
		test(9, 0, F_TLOCK, (off_t)0, (off_t)4, PASS, FATAL);
		write_testfile("aaaa", (off_t)0, 4, COMMENT);
		comment("Now free child to run, should block on write.");
		childfree(wait_time);
		comment("Check data in file before child writes.");
		read_testfile(9, 1, "aaaa", (off_t)0, 4, EQUAL, FATAL);
		comment("Now unlock region so child will write.");
		test(9, 2, F_ULOCK, (off_t)0, (off_t)4, PASS, FATAL);
		comment("Check data in file after child writes.");
		read_testfile(9, 3, "bbbb", (off_t)0, 4, EQUAL, FATAL);
		close_testfile(DO_UNLINK);
		childfree(0);
	} else {
		parentfree(0);
		childwait();
		open_testfile(OPENFLAGS, OPENMODES);
		write_testfile("bbbb", (off_t)0, 4, COMMENT);
		close_testfile(DO_UNLINK);
		childwait();
	}
}

static void
test10()
{

	if (who == PARENT) {
		parentwait();
		header(10, "Make sure a locked region is split properly.");
		open_testfile(OPENFLAGS, OPENMODES);
		test(10, 0, F_TLOCK, (off_t)0, (off_t)3, PASS, FATAL);
		test(10, 1, F_ULOCK, (off_t)1, (off_t)1, PASS, FATAL);
		childfree(0);
		parentwait();
		test(10, 6, F_ULOCK, (off_t)0, (off_t)1, PASS, FATAL);
		test(10, 7, F_ULOCK, (off_t)2, (off_t)1, PASS, FATAL);
		childfree(0);
		parentwait();
		test(10, 9, F_ULOCK, (off_t)0, (off_t)1, PASS, FATAL);
		test(10, 10, F_TLOCK, (off_t)1, (off_t)3, PASS, FATAL);
		test(10, 11, F_ULOCK, (off_t)2, (off_t)1, PASS, FATAL);
		childfree(0);
		parentwait();
		close_testfile(DO_UNLINK);
	} else {
		parentfree(0);
		childwait();
		open_testfile(OPENFLAGS, OPENMODES);
		test(10, 2, F_TEST, (off_t)0, (off_t)1, denied_err, FATAL);
		test(10, 3, F_TEST, (off_t)2, (off_t)1, denied_err, FATAL);
		test(10, 4, F_TEST, (off_t)3, (off_t)END, PASS, FATAL);
		test(10, 5, F_TEST, (off_t)1, (off_t)1, PASS, FATAL);
		parentfree(0);
		childwait();
		test(10, 8, F_TEST, (off_t)0, (off_t)3, PASS, FATAL);
		parentfree(0);
		childwait();
		test(10, 12, F_TEST, (off_t)1, (off_t)1, denied_err, FATAL);
		test(10, 13, F_TEST, (off_t)3, (off_t)1, denied_err, FATAL);
		test(10, 14, F_TEST, (off_t)4, (off_t)END, PASS, FATAL);
		test(10, 15, F_TEST, (off_t)2, (off_t)1, PASS, FATAL);
		test(10, 16, F_TEST, (off_t)0, (off_t)1, PASS, FATAL);
		close_testfile(DO_UNLINK);
		parentfree(0);
	}
}

static void
test11()
{
	int dupfd;
	char *data = "123456789abcdef";
	int datalen;

	datalen = strlen(data) + 1;	/* including trailing NULL */

	if (who == PARENT) {
		parentwait();
		header(11, "Make sure close() releases the process's locks.");
		open_testfile(OPENFLAGS, OPENMODES);
		dupfd = dup(testfd);
		test(11, 0, F_TLOCK, (off_t)0, (off_t)0, PASS, FATAL);
		close_testfile(JUST_CLOSE);
		childfree(0);

		parentwait();
		testdup2(dupfd, testfd);
		test(11, 3, F_TLOCK, (off_t)29, (off_t)1463, PASS, FATAL);
		test(11, 4, F_TLOCK, (off_t)0x2000, (off_t)87, PASS, FATAL);
		close_testfile(JUST_CLOSE);
		childfree(0);

		parentwait();
		testdup2(dupfd, testfd);
		write_testfile(data, (off_t)0, datalen, COMMENT);
		test(11, 7, F_TLOCK, (off_t)0, (off_t)0, PASS, FATAL);
		write_testfile(data, (off_t)(datalen - 3), datalen, COMMENT);
		close_testfile(JUST_CLOSE);
		childfree(0);

		parentwait();
		testdup2(dupfd, testfd);
		write_testfile(data, (off_t)0, datalen, COMMENT);
		test(11, 10, F_TLOCK, (off_t)0, (off_t)0, PASS, FATAL);
		testtruncate();
		close_testfile(JUST_CLOSE);
		childfree(0);

		parentwait();
		close(dupfd);
		close_testfile(DO_UNLINK);
	} else {
		parentfree(0);
		childwait();
		open_testfile(OPENFLAGS, OPENMODES);
		test(11, 1, F_TLOCK, (off_t)0, (off_t)0, PASS, FATAL);
		test(11, 2, F_ULOCK, (off_t)0, (off_t)0, PASS, FATAL);
		parentfree(0);

		childwait();
		test(11, 5, F_TLOCK, (off_t)0, (off_t)0, PASS, FATAL);
		test(11, 6, F_ULOCK, (off_t)0, (off_t)0, PASS, FATAL);
		parentfree(0);

		childwait();
		test(11, 8, F_TLOCK, (off_t)0, (off_t)0, PASS, FATAL);
		test(11, 9, F_ULOCK, (off_t)0, (off_t)0, PASS, FATAL);
		parentfree(0);

		childwait();
		test(11, 11, F_TLOCK, (off_t)0, (off_t)0, PASS, FATAL);
		test(11, 12, F_ULOCK, (off_t)0, (off_t)0, PASS, FATAL);
		close_testfile(DO_UNLINK);
		parentfree(0);
	}
}

static void
test12()
{
	if (who == PARENT) {
		pid_t target;

		close(pidpipe[1]);

		parentwait();
		header(12, "Signalled process should release locks.");
		open_testfile(OPENFLAGS, OPENMODES);
		childfree(0);

		parentwait();
		(void) lseek(testfd, (off_t)0, 0);
		if (read(pidpipe[0], &target, sizeof (target)) !=
		    sizeof (target)) {
			perror("can't read pid to kill");
			testexit(1);
		}
		kill(target, SIGINT);
		comment("Killed child process.");
		sleep(wait_time);
		test(12, 1, F_TLOCK, (off_t)0, (off_t)0, PASS, FATAL);
		childfree(0);
		close_testfile(DO_UNLINK);
	} else {
		pid_t subchild;

		close(pidpipe[0]);

		parentfree(0);
		childwait();
		open_testfile(OPENFLAGS, OPENMODES);
		/*
		 * Create a subprocess to obtain a lock and get killed.  If
		 * the parent kills the regular child, tlock will stop
		 * after the first pass.
		 */
		subchild = fork();
		if (subchild < 0) {
			perror("can't fork off subchild");
			testexit(1);
		}
		/*
		 * Record the pid of the subprocess and wait for the parent
		 * to tell the child that the test is done.  Note that the
		 * child and subchild share the file offset; keep this in
		 * mind if you change this test.
		 */
		if (subchild > 0) {
			/* original child */
			sleep(wait_time);
			(void) lseek(testfd, (off_t)0, 0);
			if (write(pidpipe[1], &subchild, sizeof (subchild)) !=
			    sizeof (subchild)) {
				perror("can't record pid to kill");
				kill(subchild, SIGINT);
				testexit(1);
			}
			parentfree(0);
			childwait();
			close_testfile(DO_UNLINK);
		} else {
			/* subchild */
			signal(SIGINT, SIG_DFL);
			test(12, 0, F_TLOCK, (off_t)0, (off_t)0, PASS, FATAL);
			for (;;)
				sleep(1);
			/* NOTREACHED */
		}
	}
}

#ifdef MMAP
static void
test13()
{
	if (who == PARENT) {
		int lock1err;
		int err;

		parentwait();
		open_testfile(OPENFLAGS, OPENMODES);
		header(13, "Check locking and mmap semantics.");

		/*
		 * Can a file be locked and mapped at same time?  It's
		 * potentially safe if the whole file is locked, or if the
		 * locked region is page-aligned.  Otherwise, there is a
		 * race condition between two clients that lock disjoint
		 * regions of the same page.  So for this test we
		 * deliberately lock a region that is not
		 * page-aligned--that should cause the mmap to fail.  But if
		 * it doesn't, that's arguably not an interoperability
		 * problem, so make it a warning, not a fatal error.
		 */
		test(13, 0, F_TLOCK, (off_t)mappedlen - 2, (off_t)END, PASS,
			FATAL);
		lock1err = testmmap();
		report(13, 1, "mmap", (off_t)0, (off_t)mappedlen, EAGAIN,
			lock1err, WARN);
		test(13, 2, F_ULOCK, (off_t)0, (off_t)END, PASS, FATAL);
		if (lock1err == 0)
			testmunmap();

		/*
		 * Does the order of lock/mmap matter?  This also verifies
		 * that releasing the lock makes the file mappable again.
		 * Again, allowing an unsafe map and lock combination is
		 * not an interoperability problem, so flag it as a
		 * warning, not an error.
		 */
		err = testmmap();
		report(13, 3, "mmap", (off_t)0, (off_t)mappedlen, PASS, err,
			FATAL);
		test(13, 4, F_TLOCK, (off_t)mappedlen - 2, (off_t)END,
			lock1err, WARN);
		close_testfile(DO_UNLINK);

		childfree(0);
	} else {
		parentfree(0);
		childwait();
	}
}
#endif /* MMAP */

static void
test14()
{
	if (who == PARENT) {
		header(14,
		    "Rate test performing I/O on unlocked and locked file.");
		open_testfile(OPENFLAGS, OPENMODES);
		comment("File Unlocked");
		iorate(iorate_kb, iorate_count);
		test(14, 0, F_TLOCK, (off_t)0, (off_t)END, PASS, FATAL);
		comment("File Locked");
		iorate(iorate_kb, iorate_count);
		test(14, 1, F_ULOCK, (off_t)0, (off_t)END, PASS, FATAL);
		close_testfile(DO_UNLINK);
	}
}

static void
test15()
{
	char *data = "abcdefghij";
	int datalen;
	int testfd2;

	if (who == CHILD)
		return;

	datalen = strlen(data) + 1;	/* including trailing NULL */

	header(15, "Test 2nd open and I/O after lock and close.");
	open_testfile(OPENFLAGS, OPENMODES);
	testfd2 = open(testfile, OPENFLAGS, OPENMODES);
	if (testfd2 < 0) {
		perror("second open");
		testexit(1);
	}
	comment("Second open succeeded.");

	test(15, 0, F_LOCK, (off_t)0, (off_t)END, PASS, FATAL);
	test(15, 1, F_ULOCK, (off_t)0, (off_t)END, PASS, FATAL);
	close_testfile(JUST_CLOSE);
	open_testfile(OPENFLAGS, OPENMODES);
	write_testfile(data, (off_t)0, datalen, COMMENT);
	read_testfile(15, 2, data, (off_t)0, (unsigned int)datalen, EQUAL,
		    FATAL);
	close(testfd2);
	close_testfile(DO_UNLINK);
}

static void
runtests()
{

	if (DO_TEST(1)) {
		test1();
	}
	if (DO_TEST(2)) {
		test2();
	}
	if (DO_TEST(3)) {
		test3();
	}
	if (DO_TEST(4)) {
		test4();
	}
	if (DO_TEST(5)) {
		test5();
	}
	if (DO_TEST(6)) {
		test6();
	}
	if (DO_TEST(7)) {
		test7();
	}
	if (DO_RATE(8)) {
		test8();
	}
	if (DO_MAND(9)) {
		test9();
	}
	if (DO_TEST(10)) {
		test10();
	}
	if (DO_TEST(11)) {
		test11();
	}
	if (DO_TEST(12)) {
		test12();
	}
#ifdef MMAP
	if (DO_TEST(13)) {
		test13();
	}
#endif
	if (DO_TEST(14)) {
		test14();
	}
	if (DO_TEST(15)) {
		test15();
	}
}

/*
 * Main record locking test loop.
 */
int
main(argc, argv)
	int argc;
	char **argv;
{
	int c;
	extern int optind;
	extern char *optarg;
	int errflg = 0;

	passcnt = 1;	/* default, test for 1 pass */

	while ((c = getopt(argc, argv, "p:t:rmv:w:")) != -1) {
		switch (c) {
		case 'p':
			sscanf(optarg, "%d", &passcnt);
			break;
		case 't':
			sscanf(optarg, "%d", &testnum);
			break;
		case 'r':
			ratetest++;
			break;
		case 'm':
			mandtest++;
			break;
		case 'v':
			sscanf(optarg, "%d", &proto_vers);
			break;
		case 'w':
			sscanf(optarg, "%d", &wait_time);
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		fprintf(stderr,
"usage: tlock [-p passcnt] [-t testnum] [-r] [-m] [-w wait_time] [dirpath]\n");
		exit(2);
	}
	if (optind < argc) {
		filepath = argv[optind];
	}
	initialize();

	/*
	 * Fork child...
	 */
	if ((childpid = fork()) == 0) {
		who = CHILD;
		signal(SIGINT, parentsig);
	} else {
		who = PARENT;
		signal(SIGINT, childsig);
		signal(SIGCLD, childdied);
	}

	/*
	 * ...and run the tests for count passes.
	 */
	for (passnum = 1; passnum <= passcnt; passnum++) {
		runtests();
		if (who == CHILD) {
			childwait();
			testreport(0);
		} else {
			testreport(0);
			childfree(0);
		}
	}
	if (who == CHILD) {
		childwait();
	} else {
		signal(SIGCLD, SIG_DFL);
		childfree(0);
	}
	testexit(0);
	/* NOTREACHED */
}
