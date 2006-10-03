/*
 *	@(#)op_chmod.c	1.5 2003/12/30 Connectathon Testsuite
 *	1.3 Lachman ONC Test Suite source
 *
 *  tests operation on open file which has been chmod'd to 0.
 *  steps taken:
 *	1.  create file
 *	2.  open for read/write
 *	3.  chmod 0
 *	4.  write data
 *	5.  rewind
 *	6.  read data back
 */

#if defined (DOS) || defined (WIN32)
#define DOSorWIN32
#endif

#ifndef DOSorWIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(SVR3) || defined(SVR4)
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#endif /* DOSorWIN32 */

#include "../tests.h"

#define TBUFSIZ 100
static char wbuf[TBUFSIZ], rbuf[TBUFSIZ];
static char buf[BUFSIZ];
#define TMSG "This is a test message written to the chmod'd file\n"

static void
xxit(s)
	char *s;
{
	perror(s);
	exit(1);
}

/*ARGSUSED*/
int
main(argc, argv)
	int argc;
	char *argv[];
{
	int fd, ret;
	char *tname;
	int errcount = 0;
	long lret;
	int oflags;			/* open flags */

	setbuf(stdout, NULL);

#ifdef MACOSX
	/* make sure TMPDIR isn't set */
	unsetenv("TMPDIR");
#endif
	if ((tname = tempnam(".", "nfs")) == NULL) {
		fprintf(stderr, "can't construct a temporary filename: ");
		xxit("tempnam");
	}

	oflags = O_CREAT|O_TRUNC|O_RDWR;
#ifdef DOSorWIN32
	oflags |= O_BINARY;
#endif
	if ((fd = open(tname, oflags, CHMOD_RW)) < 0) {
		fprintf(stderr, "can't create %s: ", tname);
		xxit("open");
	}

	printf("testfile before chmod:\n  ");
	sprintf(buf, "ls -l %s", tname);
	system(buf);
	ret = chmod(tname, CHMOD_NONE);
	printf("%s open; chmod ret = %d\n", tname, ret);
	if (ret)
		xxit(" chmod");
	printf("testfile after chmod:\n  ");
	system(buf);
	strcpy(wbuf, TMSG);
	if ((ret = write(fd, wbuf, TBUFSIZ)) != TBUFSIZ) {
		fprintf(stderr, "write ret %d; expected %d\n", ret, TBUFSIZ);
		if (ret < 0)
			perror(" write");
		exit(1);
	}
	if ((lret = lseek(fd, 0L, 0)) != 0L) {
		fprintf(stderr, "lseek ret %ld; expected 0\n", lret);
		if (lret < 0)
			perror(" lseek");
		exit(1);
	}
	if ((ret = read(fd, rbuf, TBUFSIZ)) != TBUFSIZ) {
		fprintf(stderr, "read ret %d; expected %d\n", ret, TBUFSIZ);
		if (ret < 0)
			perror(" read");
		exit(1);
	}
	if (strcmp(wbuf, rbuf) != 0) {
		errcount++;
		printf("read data not same as written data\n");
		printf(" written: '%s'\n read:    '%s'\n", wbuf, rbuf);
	} else {
		printf("data compare ok\n");
	}

	printf("testfile after write/read:\n  ");
	system(buf);

#ifndef DOSorWIN32
	if (close(fd))
		xxit("error on close");
	if (unlink(tname) < 0) {
		fprintf(stderr, "can't unlink %s", tname);
		xxit(" ");
	}
#else
	/* 
	 * For DOS you can't delete the file if it is RO, even if you
	 * opened it RW. So, change to RW and close before the unlink.
	 */

	ret = chmod(tname, CHMOD_RW);
	printf("%s open; chmod ret = %d\n", tname, ret);
	if (ret) 
		xxit(" chmod RW");
	printf("testfile after chmod RW:\n  ");
	system(buf);

	if (close(fd))
		xxit("error on close");

	if (unlink(tname) < 0) {
		fprintf(stderr, "can't unlink %s", tname);
		xxit(" ");
	}
#endif /* DOSorWIN32 */

	printf("test completed successfully.\n");
	exit(0);
}

