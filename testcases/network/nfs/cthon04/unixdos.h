/*	
 *	@(#)unixdos.h	1.1	98/10/26 Connectathon Testsuite
 *
 * typedefs used in the DOSorWIN version only
 */

#if ! defined(_WINSOCKAPI_)
struct timeval
{
	long tv_sec;  /* seconds since midnight for DOS */
	long tv_usec; /* and microseconds */
};
#endif

typedef unsigned char u_char;

#define MAXPATHLEN 256	/* tune later */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void gettimeofday(struct timeval *TV, struct timezone *TimeZone);
int unix_chdir(char * path);
int lstat(char *path, struct stat *buf);

/************************************************************
statfs stuff
************************************************************/

          typedef struct {
                 long    val[2];
          } fsid_t;
          struct statfs {
                 long    f_type;     /* type of info, zero for now */
                 long    f_bsize;    /* fundamental file system block size */
                 long    f_blocks;   /* total blocks in file system */
                 long    f_bfree;    /* free blocks */
                 long    f_bavail;   /* free blocks available to non-super-user */
                 long    f_files;    /* total file nodes in file system */
                 long    f_ffree;    /* free file nodes in fs */
                 fsid_t  f_fsid;     /* file system id */
                 long    f_spare[7]; /* spare for later */
          };
          
int statfs(char *path, struct statfs *buf);

/************************************************************
From /usr/include/directory.h, simplified:
************************************************************/

#ifndef	__dirent_h
#define	__dirent_h

/*
 * Definitions for library routines operating on directories.
 */
typedef int DIR;	/* just a dummy */

DIR *opendir(char *dirname);
struct dirent *readdir(DIR *dirp);
void rewinddir(DIR *dirp);
void closedir(DIR *dirp);
#ifndef	_POSIX_SOURCE
void seekdir(DIR *dirp, long loc);
long telldir(DIR *dirp);
#endif	/* POSIX_SOURCE */

#endif	/* !__dirent_h */

/*************************************************************
From /usr/include/sys/dirent.h:
*************************************************************/

/*
 * Filesystem-independent directory information.
 * Directory entry structures are of variable length.
 * Each directory entry is a struct dirent containing its file number, the
 * offset of the next entry (a cookie interpretable only the filesystem
 * type that generated it), the length of the entry, and the length of the
 * name contained in the entry.  These are followed by the name. The
 * entire entry is padded with null bytes to a 4 byte boundary. All names
 * are guaranteed null terminated. The maximum length of a name in a
 * directory is MAXNAMLEN, plus a null byte.
 */

#ifndef	__sys_dirent_h
#define	__sys_dirent_h

struct	dirent {
	/* just need d_name field for Cthon tests */
	char		d_name[13];	/* name (up to MAXNAMLEN + 1) */
};

#ifndef	_POSIX_SOURCE
/*
 * It's unlikely to change, but make sure that sizeof d_name above is
 * at least MAXNAMLEN + 1 (more may be added for padding).
 */
#define	MAXNAMLEN	255
/*
 * The macro DIRSIZ(dp) gives the minimum amount of space required to represent
 * a directory entry.  For any directory entry dp->d_reclen >= DIRSIZ(dp).
 * Specific filesystem types may use this macro to construct the value
 * for d_reclen.
 */
#undef	DIRSIZ
#define	DIRSIZ(dp) \
	(((sizeof(struct dirent) - (MAXNAMLEN+1) + ((dp)->d_namlen+1)) +3) & ~3)

#endif	/* !_POSIX_SOURCE */
#endif	/* !__sys_dirent_h */
