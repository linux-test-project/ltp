/*	@(#)nfsidem.c	1.7 2001/08/25 Connectathon Testsuite	*/
/*
 * Idempotency test:
 *
 * forever {
 *	mkdir	TEST
 *	mkdir	TEST/DIR
 *	echo	"..." >TEST/FOO
 *	chmod	0600 TEST/FOO
 *	rename	TEST/FOO TEST/DIR/BAR
 *	if link	TEST/DIR/BAR TEST/BAR == 0
 *	 rename	TEST/BAR TEST/DIR/BAR (noop)
 *	symlink	../TEST/DIR/BAR TEST/SBAR
 *	unlink	TEST/DIR/BAR
 *	unlink	TEST/BAR
 *	unlink	TEST/SBAR (if it was created)
 *	rmdir	TEST/DIR
 *	rmdir	TEST
 *	lookup	TEST (expect failure)
 * }
 *
 * Tom Talpey, 1992, Open Software Foundation.
 * Freely redistributable.
 */

#if defined (DOS) || defined (WIN32)
#define DOSorWIN32
#include "../tests.h"
#endif

#ifndef DOSorWIN32
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#endif /* DOSorWIN32 */

#ifndef MAXPATHLEN
#define MAXPATHLEN	128
#endif

/* maximum number of chars for the message string */
#define STRCHARS	100

static char DIR[MAXPATHLEN];
static char FOO[MAXPATHLEN];
static char BAR[MAXPATHLEN];
static char SBAR[MAXPATHLEN];
static char TBAR[MAXPATHLEN];
static char LBAR[MAXPATHLEN];
static char str[STRCHARS];

main(ac,av)
	int ac;
	char *av[];
{
	int count, fd, slen, lerr, slerr;
	char *fn;
	struct stat sb;

#ifdef DOSorWIN32
	fprintf(stderr, "This Test Not Executable on DOS or Windows\n");
	exit(1);
#else
	if (ac >= 2)
		count = atoi(av[1]);
	else
		count = 1;
	if (ac >= 3)
		fn = av[2];
	else
		fn = "./TEST";

	snprintf(DIR, MAXPATHLEN, "%s/DIR", fn);
	snprintf(FOO, MAXPATHLEN, "%s/FOO", fn);
	snprintf(BAR, MAXPATHLEN, "%s/BAR", fn);
	snprintf(SBAR, MAXPATHLEN, "%s/SBAR", fn);
	snprintf(TBAR, MAXPATHLEN, "%s/DIR/BAR", fn);
	snprintf(LBAR, MAXPATHLEN, "../%s/DIR/BAR", fn);
	snprintf(str, STRCHARS, "Idempotency test %ld running\n",
		 (long)getpid());
	slen = strlen(str);

	printf("testing %d idempotencies in directory \"%s\"\n", count, fn);

#ifndef S_IFLNK
	slerr = 1;	/* just something non-zero */
#endif

	while (count--) {
		if (mkdir(fn, 0755)) {
			perror("mkdir");
			break;
		}
		if (mkdir(DIR, 0755)) {
			perror("mkdir DIR");
			break;
		}
		fd = open(FOO, O_RDWR|O_CREAT, 0666);
		if (fd < 0) {
			perror("creat");
			break;
		}
		if (write(fd, str, slen) != slen) {
			perror("write");
			(void) close(fd);
			break;
		}
		if (close(fd)) {
			perror("close");
			break;
		}
		if (chmod(FOO, 0611)) {
			perror("chmod");
			break;
		}
		if (rename(FOO, TBAR)) {
			perror("rename FOO DIR/BAR");
			break;
		}
		if (lerr = link(TBAR, BAR)) {
			if (errno != EOPNOTSUPP) {
				perror("link");
				break;
			}
		} else if (rename(BAR, TBAR)) {
			perror("rerename");
			break;
		}
#ifdef S_IFLNK
		if (slerr = symlink(LBAR, SBAR)) {
			if (errno != EOPNOTSUPP) {
				perror("symlink");
				break;
			}
		}
#endif
		if (stat(!slerr ? SBAR : !lerr ? BAR : TBAR, &sb)) {
			perror("stat 1");
			break;
		}
		if ((sb.st_mode & (S_IFMT|07777)) != (S_IFREG|0611) ||
		    sb.st_size != slen) {
			fprintf(stderr, "stat 1: bad file type/size 0%o/%ld\n",
				(int)sb.st_mode, (long)sb.st_size);
#ifdef EFTYPE
			errno = EFTYPE;
#else
			errno = EINVAL;
#endif
			break;
		}
		if (unlink(TBAR)) {
			perror("unlink 1");
			break;
		}
#ifdef BSD
		printf("BSD workaround: skipping unlink(BAR)\n");
#else
		if (lerr == 0 && unlink(BAR)) {
			perror("unlink 2");
			break;
		}
#endif
		if (slerr == 0 && unlink(SBAR)) {
			perror("unlink 3");
			break;
		}
		if (rmdir(DIR)) {
			perror("rmdir 1");
			break;
		}
		if (rmdir(fn)) {
			perror("rmdir 2");
			break;
		}
		if (stat(fn, &sb) == 0 || errno != ENOENT) {
			perror("stat 2");
			break;
		}
		errno = 0;	/* clear error from last stat(2) */
	}

	exit(errno);
#endif /* DOSorWIN32 */
}
