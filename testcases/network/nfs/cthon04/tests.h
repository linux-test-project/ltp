/*	@(#)tests.h	1.7 2003/12/01 Connectathon Testsuite	*/
/*	1.4 Lachman ONC Test Suite source	*/

#include <errno.h>

#ifdef WIN32
#define DOSorWIN32
#include <windows.h>
#endif
#ifdef DOS
#define DOSorWIN32
#include <dos.h>
#endif
#ifdef DOSorWIN32
#define ANSI
#include "unixdos.h"
#endif

#define	DNAME	"dir."
#define	FNAME	"file."
#define	DDIRS	2
#define	DLEVS	5
#define	DFILS	5

#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif

#ifndef DOSorWIN32
#define	TESTDIR	"/mnt/nfstestdir"
#define	DCOUNT	10
#define CHMOD_MASK	0777
#define CHMOD_NONE	0
#define	CHMOD_RW	0666
#else
#define TESTDIR "o:\\nfstestd"
#define DCOUNT  2			/* takes too long with10 */
#define	CHMOD_MASK	(S_IREAD | S_IWRITE)
#define	CHMOD_NONE	(S_IREAD)
#define	CHMOD_RW	CHMOD_MASK
#endif /* DOSorWIN32 */

#ifndef MAXPATHLEN
#define MAXPATHLEN	1024
#endif

extern char *Myname;		/* name I was invoked with (for error msgs */

#ifdef STDARG
extern void error(char *, ...);
#endif

#ifdef __STDC__
#define ARGS_(x) x
#else
#define ARGS_(x) ()
#endif

extern void starttime ARGS_((void));
extern void endtime ARGS_((struct timeval *tv));
extern long getparm ARGS_((char *parm, long min, char *label));
extern void dirtree ARGS_((int lev, int files, int dirs, char *fname,
		    char *dname, int *totfiles, int *totdirs));
extern void rmdirtree ARGS_((int lev, int files, int dirs, char *fname,
		      char *dname, int *totfiles, int *totdirs, int ignore));
extern void testdir ARGS_((char *dir));
extern int mtestdir ARGS_((char *dir));
extern void complete ARGS_((void));

#ifdef NEED_STRERROR
extern char *strerror ARGS_((int));
#endif

#ifdef DOSorWIN32
extern int unix_chdir(char *path);

/* These are redfined so stderr and stdout go to the same redirected file */
#define stdin  (&_iob[0])
#define stdout (&_iob[2])
#define stderr (&_iob[2])
#endif /* DOSorWIN32 */
