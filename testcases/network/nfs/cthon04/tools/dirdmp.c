/*
 *	@(#)dirdmp.c	1.8 2003/12/30 Connectathon Testsuite
 *	1.2 Lachman ONC Test Suite source
 */

#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#ifdef OSF1
#undef BSD
#define dd_bsize dd_bufsiz
#define dd_bbase dd_seek	/* Totally bogus, but it works */
#endif

#ifndef ARGS_
#ifdef __STDC__
#define ARGS_(x) x
#else
#define ARGS_(x) ()
#endif
#endif

static DIR * my_opendir ARGS_((char *name));
static void print ARGS_((char *));
static struct dirent *my_readdir ARGS_((DIR *dirp));

main(argc, argv)
	int argc;
	char *argv[];
{
#if defined(LINUX) || defined (AIX)
	fprintf(stderr, "dirdmp is not supported on this platform.\n");
	exit(1);
#else
	argv++;
	argc--;
	while (argc--) {
		print(*argv++);
	}
#endif /* LINUX || AIX */
}

#if !(defined(LINUX) || defined(AIX))

static void
print(dir)
	char *dir;
{
	DIR *dirp;

	dirp = my_opendir(dir);
	if (dirp == NULL) {
		perror(dir);
		return;
	}
	while (my_readdir(dirp) != NULL) 
		;
	closedir(dirp);
}

/*
 * open a directory.
 */
static DIR *
my_opendir(name)
	char *name;
{
	register DIR *dirp;
	register int fd;
	struct stat sb;

	if ((fd = open(name, 0)) == -1) {
		printf("open failed\n");
		return (NULL);
	}
	if (fstat(fd, &sb) == -1) {
		printf("stat failed\n");
		return (NULL);
	}
	/*
	if ((sb.st_mode & S_IFMT) != S_IFDIR) {
		printf("not a directory\n");
		return (NULL);
	}
	*/
	printf("%s mode %o dir %o\n", name, (int)sb.st_mode, S_IFDIR);
	if (((dirp = (DIR *)malloc(sizeof(DIR))) == NULL) ||
#if defined(SVR3) || defined(SVR4)
	    ((dirp->dd_buf = malloc(DIRBUF)) == NULL)) {
#else
	    ((dirp->dd_buf = malloc((int)sb.st_blksize)) == NULL)) {
#endif
		if (dirp) {
			if (dirp->dd_buf) {
				free(dirp->dd_buf);
			}
			free(dirp);
		}
		close(fd);
		return (NULL);
	}
#if !defined(SVR3) && !defined(SVR4) && !defined(BSD)
	dirp->dd_bsize = sb.st_blksize;
#if !defined(SUNOS4X)
	dirp->dd_bbase = 0;
#if !defined(OSF1)
	dirp->dd_entno = 0;
#endif
#endif
#endif
	dirp->dd_fd = fd;
	dirp->dd_loc = 0;
	return (dirp);
}

/*
 * get next entry in a directory.
 */
static
struct dirent *
my_readdir(dirp)
	DIR *dirp;
{
	register struct dirent *dp;
	static int header;	/* 1 after header prints */

	for (;;) {
		if (dirp->dd_loc == 0) {
#if defined(SVR3) || defined(SVR4)
			dirp->dd_size = getdents(dirp->dd_fd,
#if defined(__STDC__)
			    (struct dirent *) dirp->dd_buf, DIRBUF);
#else
			    dirp->dd_buf, DIRBUF);
#endif
#else
#if defined(SUNOS4X)
			dirp->dd_size = getdents(dirp->dd_fd,
			    dirp->dd_buf, dirp->dd_bsize);
#elif defined(MACOSX)
			dirp->dd_size = getdirentries(dirp->dd_fd,
			    dirp->dd_buf, dirp->dd_len, &dirp->dd_seek);
#elif defined(BSD)
			dirp->dd_size = getdents(dirp->dd_fd,
			    dirp->dd_buf, dirp->dd_size);
#else
			dirp->dd_size = getdirentries(dirp->dd_fd,
			    dirp->dd_buf, dirp->dd_bsize, &dirp->dd_bbase);
#endif
#endif
			if (dirp->dd_size < 0) {
				perror("getdirentries");
				return (NULL);
			}
			if (dirp->dd_size == 0) {
				printf("EOF\n");
				return (NULL);
			}
#if !defined(SVR3) && !defined(SVR4) && !defined(SUNOS4X) && \
	!defined(OSF1) && !defined(BSD)
			dirp->dd_entno = 0;
#endif
		}
		if (dirp->dd_loc >= dirp->dd_size) {
#ifdef BSD
			printf("EOB offset %ld\n", 
			       (long)lseek(dirp->dd_fd, 0, SEEK_CUR));
#else
			printf("EOB offset %ld\n", (long)tell(dirp->dd_fd));
#endif
			dirp->dd_loc = 0;
			header = 0;
			continue;
		}
		dp = (struct dirent *)(dirp->dd_buf + dirp->dd_loc);
		if (dp->d_reclen <= 0) {
			printf("0 reclen\n");
			return (NULL);
		}
#if defined(SVR3) || defined(SVR4)
		if (!header) {
			header = 1;
			printf("  loc    ino reclen name\n");
		}
		printf("%5d %6ld %6d %s\n",
		    dirp->dd_loc, (long)dp->d_ino, dp->d_reclen,
		    dp->d_name);
#else
		if (!header) {
			header = 1;
			printf("  loc fileno reclen namelen name\n");
		}
		printf("%5d %6d %6d %7d %s\n",
		    dirp->dd_loc, dp->d_fileno, dp->d_reclen,
		    dp->d_namlen, dp->d_name);
#endif
		dirp->dd_loc += dp->d_reclen;
#if !defined(SVR3) && !defined(SVR4) && !defined(SUNOS4X) && \
	!defined(OSF1) && !defined(BSD)
		dirp->dd_entno++;
#endif
#if defined(SVR3) || defined(SVR4)
		if (dp->d_ino == 0) {
#else
		if (dp->d_fileno == 0) {
#endif
			continue;
		}
		return (dp);
	}
}

#endif /* LINUX || AIX */
