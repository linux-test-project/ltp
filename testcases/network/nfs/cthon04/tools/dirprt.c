/*
 *	@(#)dirprt.c	1.6 2002/12/13 Connectathon Testsuite
 *	1.2 Lachman ONC Test Suite source
 */

#ifdef SOLARIS2X
/* 
 * XXX if this is lacking, i-numbers get printed as zero.  Not clear why;
 * should investigate sometime.
 */
#define _FILE_OFFSET_BITS	64
#endif

#include <sys/param.h>
#ifndef major
#include <sys/types.h>
#endif
#ifdef use_directs
#include <sys/dir.h>
#else
#include <dirent.h>
#endif
#include <stdio.h>
#include <ctype.h>

#ifndef ARGS_
#ifdef __STDC__
#define ARGS_(x) x
#else
#define ARGS_(x) ()
#endif
#endif

static DIR *my_opendir ARGS_((char *));
static void print ARGS_((char *));

main(argc, argv)
	int argc;
	char *argv[];
{
#if defined (AIX)
	fprintf(stderr, "dirprt is not supported on this platform.\n");
	exit(1);
#else
	argv++;
	argc--;
	while (argc--) {
		print(*argv++);
	}
#endif /* AIX */
}

#if !defined(AIX)

static void
print(dir)
	char *dir;
{
	DIR *dirp;
#ifdef use_directs
	struct direct *dp;
#else
	struct dirent *dp;
#endif

	dirp = my_opendir(dir);
	if (dirp == NULL) {
		perror(dir);
		return;
	}
	while ((dp = readdir(dirp)) != NULL) {
#if defined(SVR3) || defined(SVR4) || defined(LINUX)
		printf("%5ld %5ld %5d %s\n", (long)telldir(dirp),
		       (long)dp->d_ino,
		       dp->d_reclen, dp->d_name);
#else
		printf("%5ld %5d %5d %5d %s\n", (long)telldir(dirp), 
		       dp->d_fileno,
		       dp->d_reclen, dp->d_namlen, dp->d_name);
#endif
	}
	closedir(dirp);
}

#include <sys/stat.h>

/*
 * open a directory.
 */
static DIR *
my_opendir(name)
	char *name;
{
	struct stat sb;

	if (stat(name, &sb) == -1) {
		printf("stat failed\n");
		return (NULL);
	}
	if ((sb.st_mode & S_IFMT) != S_IFDIR) {
		printf("not a directory\n");
		return (NULL);
	}
	printf("%s mode %o dir %o\n", name, (int)sb.st_mode, S_IFDIR);
	return(opendir(name));
}

#endif /* AIX */
