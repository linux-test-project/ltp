/*
 *	@(#)subr.c	1.6 03/12/29 Connectathon Testsuite
 *	1.6 Lachman ONC Test Suite source
 *
 * Useful subroutines shared by all tests
 */

#if defined (DOS) || defined (WIN32)
/* If Dos, Windows or Win32 */
#define DOSorWIN32
#endif

#ifdef DOSorWIN32
#include <io.h>
#include <direct.h>
#else
#include <sys/param.h>
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/timeb.h>

#ifdef DOSorWIN32
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef STDARG
#include <stdarg.h>
#endif

#include "../tests.h"

char *Myname;
int Dflag = 0;

static void chdrive ARGS_((char *path));

/*
 * Build a directory tree "lev" levels deep
 * with "files" number of files in each directory
 * and "dirs" fan out.  Starts at the current directory.
 * "fname" and "dname" are the base of the names used for
 * files and directories.
 */
void
dirtree(lev, files, dirs, fname, dname, totfiles, totdirs)
	int lev;
	int files;
	int dirs;
	char *fname;
	char *dname;
	int *totfiles;
	int *totdirs;
{
	int fd;
	int f, d;
	char name[MAXPATHLEN];

	if (lev-- == 0) {
		return;
	}
	for ( f = 0; f < files; f++) {
		if (Dflag == 0) 
			sprintf(name, "%s%d", fname, f);
		else
			sprintf(name, "%s%d.%d", fname, lev, f);
		if ((fd = creat(name, CHMOD_RW)) < 0) {
			error("creat %s failed", name);
			exit(1);
		}
		(*totfiles)++;
		if (close(fd) < 0) {
			error("close %d failed", fd);
			exit(1);
		}
	}
	for ( d = 0; d < dirs; d++) {
		if (Dflag == 0) 
			sprintf(name, "%s%d", dname, d);
		else
			sprintf(name, "%s%d.%d", dname, lev, d);
		if (unix_mkdir(name, 0777) < 0) {
			error("mkdir %s failed", name);
			exit(1);
		}
		(*totdirs)++;
		if (unix_chdir(name) < 0) {
			error("chdir %s failed", name);
			exit(1);
		}
		dirtree(lev, files, dirs, fname, dname, totfiles, totdirs);
		if (unix_chdir("..") < 0) {
			error("chdir .. failed");
			exit(1);
		}
	}
}

/*
 * Remove a directory tree starting at the current directory.
 * "fname" and "dname" are the base of the names used for
 * files and directories to be removed - don't remove anything else!
 * "files" and "dirs" are used with fname and dname to generate
 * the file names to remove.
 *
 * This routine will fail if, say after removing known files,
 * the directory is not empty.
 *
 * This is used to test the unlink function and to clean up after tests.
 */
void
rmdirtree(lev, files, dirs, fname, dname, totfiles, totdirs, ignore)
	int lev;
	int files;
	int dirs;
	char *fname;
	char *dname;
	int *totfiles;		/* total removed */
	int *totdirs;		/* total removed */
	int ignore;
{
	int f, d;
	char name[MAXPATHLEN];

	if (lev-- == 0) {
		return;
	}
	for ( f = 0; f < files; f++) {
		if (Dflag == 0) 
			sprintf(name, "%s%d", fname, f);
		else
			sprintf(name, "%s%d.%d", fname, lev, f);
		if (unlink(name) < 0 && !ignore) {
			error("unlink %s failed", name);
			exit(1);
		}
		(*totfiles)++;
	}
	for ( d = 0; d < dirs; d++) {
		if (Dflag == 0) 
			sprintf(name, "%s%d", dname, d);
		else
			sprintf(name, "%s%d.%d", dname, lev, d);
		if (unix_chdir(name) < 0) {
			if (ignore)
				continue;
			error("chdir %s failed", name);
			exit(1);
		}
		rmdirtree(lev, files, dirs, fname, dname, totfiles, totdirs, ignore);
		if (unix_chdir("..") < 0) {
			error("chdir .. failed");
			exit(1);
		}
		if (rmdir(name) < 0) {
			error("rmdir %s failed", name);
			exit(1);
		}
		(*totdirs)++;
	}
}

#ifdef STDARG
void
error(char *str, ...)
{
	int oerrno;
	char *ret;
	char path[MAXPATHLEN];
	va_list ap;

	oerrno = errno;

	va_start(ap, str);
	if ((ret = getcwd(path, sizeof(path))) == NULL)
		fprintf(stderr, "%s: getcwd failed\n", Myname);
	else
		fprintf(stderr, "\t%s: (%s) ", Myname, path);
	vfprintf(stderr, str, ap);
	va_end(ap);

	if (oerrno) {
		errno = oerrno;
		perror(" ");
	} else {
		fprintf(stderr, "\n");
	}
	fflush(stderr);
	if (ret == NULL)
		exit(1);
}
#else
/* VARARGS */
error(str, ar1, ar2, ar3, ar4, ar5, ar6, ar7, ar8, ar9)
	char *str;
{
	int oerrno;
	char *ret;
	char path[MAXPATHLEN];

	oerrno = errno;
	if ((ret = getcwd(path, sizeof(path))) == NULL)
		fprintf(stderr, "%s: getcwd failed\n", Myname);
	else
		fprintf(stderr, "\t%s: (%s) ", Myname, path);

	fprintf(stderr, str, ar1, ar2, ar3, ar4, ar5, ar6, ar7, ar8, ar9);
	if (oerrno) {
		errno = oerrno;
		perror(" ");
	} else {
		fprintf(stderr, "\n");
	}
	fflush(stderr);
	if (ret == NULL)
		exit(1);
}
#endif /* STDARG */

static struct timeval ts, te;

/*
 * save current time in struct ts
 */
void
starttime()
{

	gettimeofday(&ts, (struct timezone *)0);
}

/*
 * sets the struct tv to the difference in time between
 * current time and the time in struct ts.
 */
void
endtime(tv)
	struct timeval *tv;
{

	gettimeofday(&te, (struct timezone *)0);
	if (te.tv_usec < ts.tv_usec) {
		te.tv_sec--;
		te.tv_usec += 1000000;
	}
	tv->tv_usec = te.tv_usec - ts.tv_usec;
	tv->tv_sec = te.tv_sec - ts.tv_sec;
#ifdef DOS
	/*
	 * DOS uses time since midnight, so it could go negative if the
	 * test spans midnight.  If that happens, add a day.
	 */
	if (tv->tv_sec < 0)
		tv->tv_sec += 24 * 3600;
#endif
}

/*
 * Set up and move to a test directory
 */
void
testdir(dir)
	char *dir;
{
	struct stat statb;
	char str[MAXPATHLEN];

	/*
	 *  If dir is non-NULL, use that dir.  If NULL, first
	 *  check for env variable NFSTESTDIR.  If that is not
	 *  set, use the compiled-in TESTDIR.
	 */
	if (dir == NULL)
		if ((dir = getenv("NFSTESTDIR")) == NULL)
			dir = TESTDIR;

	if (stat(dir, &statb) == 0) {
		sprintf(str, "rm -r %s", dir);
#ifdef WIN16
		if (rmdir(dir) < 0) {
#else
		if (system(str) != 0) {
#endif
			error("can't remove old test directory %s", dir);
			exit(1);
		}
	}

	if (unix_mkdir(dir, 0777) < 0) {
		error("can't create test directory %s", dir);
		exit(1);
	}
	if (unix_chdir(dir) < 0) {
		error("can't chdir to test directory %s", dir);
		exit(1);
	}
}

/*
 * Move to a test directory
 */
int
mtestdir(dir)
	char *dir;
{
	/*
	 *  If dir is non-NULL, use that dir.  If NULL, first
	 *  check for env variable NFSTESTDIR.  If that is not
	 *  set, use the compiled-in TESTDIR.
	 */
	if (dir == NULL)
		if ((dir = getenv("NFSTESTDIR")) == NULL)
			dir = TESTDIR;

	if (unix_chdir(dir) < 0) {
		error("can't chdir to test directory %s", dir);
		return(-1);
	}
	return(0);
}

/*
 *  get parameter at parm, convert to int, and make sure that
 *  it is at least min.
 */
long
getparm(parm, min, label)
	char *parm; 
	long min;
	char *label;
{
	long val;

	val = atol(parm);
	if (val < min) {
		error("Illegal %s parameter %ld, must be at least %ld",
			label, val, min);
		exit(1);
	}
	return(val);
}

/*
 *  exit point for successful test
 */
void
complete()
{

	fprintf(stdout, "\t%s ok.\n", Myname);
	chdrive(Myname);
	exit(0);
}

/*
 * Change to drive specified in path
 */
int
unix_chdir(path)
char *path;
{
	chdrive(path);
	return chdir(path);
}

#ifndef DOSorWIN32

static void
chdrive(path)
	char *path;
{
}

int
unix_mkdir(path, mode)
	char *path;
	mode_t mode;
{
	return mkdir(path, mode);
}

#endif /* DOSorWIN32 */


#ifdef NEED_STRERROR
/*
 * Hack replacement for strerror().  This could be made to include useful
 * error strings, but it will do for the time being.
 */
char *
strerror(errval)
	int errval;			/* errno value */
{
	static char buf[1024];

	sprintf(buf, "error %d", errval);
	return (buf);
}
#endif /* NEED_STRERROR */

/***********************************************************/
/* The following routines were ADDED specifically for      */
/* 			DOS AND WIN32.			   */
/***********************************************************/

#ifdef DOSorWIN32

/*
 * Return file statistics for the path specified
 */

int
lstat(char * path, struct stat * buf)
{
	return stat(path, buf);
}

int
unix_mkdir(const char * path, int mode)
{
	mode = mode;			/* keep lint, compiler happy */
	return mkdir(path);
}

#endif /* DOSorWIN32 */

/************************************************************/
/* The following routines were ADDED specifically for WIN32.*/
/************************************************************/

#ifdef WIN32

/*
 * Change to drive specified in path
 */
static void
chdrive(char * path)
{
	int desireddrive;

	if (path[1] == ':') {
		desireddrive = toupper(path[0]) - ('A' - 1);
		if (_chdrive(desireddrive)) {
			error("can't change to drive %c:", path[0]);
			exit(1);
		}
	}
}

void
gettimeofday(struct timeval *TV, struct timezone *TimeZone)
{
	struct _timeb  dostime;

	_ftime(&dostime);
	TV->tv_sec = dostime.time;
	TV->tv_usec = dostime.millitm * 1000L;
	TimeZone = TimeZone;    /* shut up compiler/lint */
}

int
statfs(char * path, struct statfs * buf)
{
	char *p = (char *) buf;
	int i;
	unsigned drive;
	
	unsigned    sect_per_clust;
	unsigned    bytes_per_sect;
	unsigned    free_clust;
	unsigned    clust;
	char rootpath[MAXPATHLEN];
	
	for (i = 0; i < sizeof(*buf); i++)
		*p++ = (char) -1;
	buf->f_type = 0;        /* that's what the man page says */
	if (path[1] == ':')
		drive = toupper(path[0]) - ('A' - 1);
	else
		drive = _getdrive();
	
	// GetDiskFreeSpace must have the #$%^&* root!
	// be simple-minded: must be "d:\<whatever>"
	strcpy(rootpath, path);
	p = strtok(rootpath, "\\");
	*p++ = '\\';
	*p = '\0';
	if (! GetDiskFreeSpace(rootpath, &sect_per_clust, &bytes_per_sect,
			 &free_clust, &clust)) {
		printf("GetDiskFreeSpace failed\n");
		return -1;
	}
	buf->f_bsize = bytes_per_sect;
	buf->f_blocks = clust * sect_per_clust;
	buf->f_bfree = free_clust * sect_per_clust;
	buf->f_bavail = buf->f_bfree;

	return 0;
}

/***************************************************************
DIRENT emulation for Win32
***************************************************************/
char pattern[MAXNAMLEN];
struct _finddata_t findtst;
long findhandle;
int maxentry;
int currententry;
int diropen = 0;
struct dirent *dirlist;
DIR dirst;

static void copynametolower(char *dest, char *src);
static void findt_to_dirent(struct dirent *d);
static int win32_findfirst(char *pattern);
static int win32_findnext(void);

int
win32_findfirst(char *pattern)
{
	findhandle = _findfirst(pattern, &findtst);
	return findhandle == -1;
}

int
win32_findnext(void)
{
	return _findnext(findhandle, &findtst);
}

int
win32_findclose(void)
{
	return _findclose(findhandle);
}

DIR *
opendir(char * dirname)
{
	int i;

	strcpy(pattern, dirname);
	strcat(pattern, "\\*.*");
	if (diropen)
		return NULL;
	diropen = 1;
	dirlist = (struct dirent *) malloc(512 * sizeof(struct dirent));
	if (dirlist == NULL)
		return NULL;

	if (win32_findfirst(pattern))
		return NULL;
	findt_to_dirent(&dirlist[0]);
	for (i = 1; ! win32_findnext(); i++) {
		findt_to_dirent(&dirlist[i]);
	}
	win32_findclose();

	maxentry = i - 1;
	currententry = 0;
	return &dirst;
}

void
rewinddir(DIR * dirp)
{
	int i;
	unsigned int attributes = _A_NORMAL|_A_RDONLY|_A_HIDDEN|_A_SUBDIR;

	dirp = dirp;    /* shut up compiler */

	if (win32_findfirst(pattern)) {
		error("rewind failed");
		exit(1);
	}
	findt_to_dirent(&dirlist[0]);
	for (i = 1; ! win32_findnext(); i++) {
		findt_to_dirent(&dirlist[i]);
	}
	win32_findclose();

	maxentry = i - 1;
	currententry = 0;
}

long
telldir(DIR * dirp)
{
	dirp = dirp;    /* keep compiler happy */
	return (long) currententry;
}

void
seekdir(DIR * dirp, long loc)
{
	dirp = dirp;    /* keep compiler happy */
	if (loc <= (long) maxentry)
		currententry = (int) loc;
	/* else seekdir silently fails */
}

struct dirent *
readdir(DIR * dirp)
{
	dirp = dirp;    /* shut up compiler */
	if (currententry > maxentry)
		return (struct dirent *) NULL;
	else {
		return &dirlist[currententry++];
	}
}

void
findt_to_dirent(struct dirent * d)
{
	copynametolower(d->d_name, findtst.name);
}

static void
copynametolower(char * dest, char * src)
{
	int i;
	for (i = 0; dest[i] = (char) tolower((int) src[i]); i++) {
		/* null body */
	}
}

void
closedir(DIR * dirp)
{
	dirp = dirp;    /* keep compiler happy */
	diropen = 0;
}


#endif /* WIN32 */

/***********************************************************/
/* The following routines were ADDED specifically for DOS  */
/***********************************************************/

#if defined DOS

/*
 * Change to drive specified in path
 */

static void
chdrive(path)
	char *path;
{
	int desireddrive, drive;
	if (path[1] == ':') {
		desireddrive = toupper(path[0]) - ('A' - 1);
		_dos_setdrive(desireddrive, &drive);
		_dos_getdrive(&drive);
		if (drive != desireddrive) {
			error("can't change to drive %c:", path[0]);
			exit(1);
		}
	}
}


void
gettimeofday(struct timeval *TV, struct timezone *TimeZone)
{
	struct dostime_t dostime;

	_dos_gettime(&dostime);
	TV->tv_sec = dostime.hour * 3600L
		+ dostime.minute * 60L
		+ dostime.second;
	TV->tv_usec = dostime.hsecond * 10000L;
	TimeZone = TimeZone;	/* shut up compiler/lint */
}

int
statfs(path, buf)
	char *path;
	struct statfs *buf;
{
	char *p = (char *) buf;
	int i;
	unsigned drive;
	struct diskfree_t diskspace;
	
	for (i = 0; i < sizeof(*buf); i++)
		*p++ = (char) -1;
	buf->f_type = 0;	/* that's what the man page says */
	if (path[1] == ':')
		drive = toupper(path[0]) - ('A' - 1);
	else
		_dos_getdrive(&drive);
	if (_dos_getdiskfree(drive, &diskspace))
		return -1;
	buf->f_bsize = diskspace.bytes_per_sector;
	buf->f_blocks = (long) diskspace.total_clusters
		* diskspace.sectors_per_cluster;
	buf->f_bfree = (long) diskspace.avail_clusters
		* diskspace.sectors_per_cluster;
	buf->f_bavail = buf->f_bfree;
	return 0;
}

/***************************************************************
DIRENT emulation for DOS
***************************************************************/
char pattern[MAXNAMLEN];
struct find_t findtst;
int maxentry;
int currententry;
int diropen = 0;
struct dirent *dirlist;
DIR dirst;

static void copynametolower(char *dest, char *src);
static void findt_to_dirent(struct find_t *f, struct dirent *d);

DIR *
opendir(dirname)
	char *dirname;
{
	int i;
	unsigned int attributes = _A_NORMAL|_A_RDONLY|_A_HIDDEN|_A_SUBDIR;

	strcpy(pattern, dirname);
	strcat(pattern, "\\*.*");
	if (diropen)
		return NULL;
	diropen = 1;
	dirlist = (struct dirent *) malloc(512 * sizeof(struct dirent));
	if (dirlist == NULL)
		return NULL;
	if (_dos_findfirst(pattern, attributes, &findtst))
		return NULL;
	findt_to_dirent(&findtst, &dirlist[0]);
	for (i = 1; ! _dos_findnext(&findtst); i++) {
		findt_to_dirent(&findtst, &dirlist[i]);
	}
	maxentry = i - 1;
	currententry = 0;
	return &dirst;
}

void
rewinddir(dirp)
	DIR *dirp;
{
	int i;
	unsigned int attributes = _A_NORMAL|_A_RDONLY|_A_HIDDEN|_A_SUBDIR;

	dirp = dirp;	/* shut up compiler */
	if (_dos_findfirst(pattern, attributes, &findtst)) {
		error("rewind failed");
		exit(1);
	}
	findt_to_dirent(&findtst, &dirlist[0]);
	for (i = 1; ! _dos_findnext(&findtst); i++) {
		findt_to_dirent(&findtst, &dirlist[i]);
	}
	maxentry = i - 1;
	currententry = 0;
}

long
telldir(dirp)
	DIR *dirp;
{
	dirp = dirp;	/* keep compiler happy */
	return (long) currententry;
}

void
seekdir(dirp, loc)
	DIR *dirp;
	long loc;
{
	dirp = dirp;	/* keep compiler happy */
	if (loc <= (long) maxentry)
		currententry = (int) loc;
	/* else seekdir silently fails */
}

struct dirent *
readdir(dirp)
	DIR *dirp;
{
	dirp = dirp;	/* shut up compiler */
	if (currententry > maxentry)
		return (struct dirent *) NULL;
	else {
		return &dirlist[currententry++];
	}
}

void
findt_to_dirent(f, d)
	struct find_t *f;
	struct dirent *d;
{
	copynametolower(d->d_name, f->name);
}

static void
copynametolower(dest, src)
	char *dest;
	char *src;
{
	int i;
	for (i = 0; dest[i] = (char) tolower((int) src[i]); i++) {
		/* null body */
	}
}

void
closedir(dirp)
	DIR *dirp;
{
	dirp = dirp;	/* keep compiler happy */
	diropen = 0;
}

#endif /* DOS */
