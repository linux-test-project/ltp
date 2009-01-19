/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	ftest02.c -- test inode things (ported from SPIE section2, filesuite, by Airong Zhang)
 *
 * CALLS
 *	open, close,  read, write, lseek,
 *	unlink, chdir
 *	
 *
 * ALGORITHM
 *
 *
 *	ftest02 [-f tmpdirname] nchild iterations [partition]
 *
 *	This forks some child processes, they do some random operations
 *	which use lots of directory operations.
 *
 * RESTRICTIONS
 *	Runs a long time with default args - can take others on input
 *	line.  Use with "term mode".
 *	If run on vax the ftruncate will not be random - will always go to
 *	start of file.  NOTE: produces a very high load average!!
 *
 */


#include <stdio.h>		/* needed by testhead.h		*/
#include "test.h"	/* standard test header		*/
#include "usctest.h"
#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/mount.h>
#include <signal.h>		/* DEM - added SIGTERM support */
#include <unistd.h>

#define MAXCHILD	25	/* max number of children to allow */
#define K_1		1024
#define K_2		2048
#define K_4		4096


char *TCID = "ftest02";
int TST_TOTAL = 1;
extern int Tst_count;

#define PASSED 1
#define FAILED 0

void crfile(int, int);
void unlfile(int, int);
void fussdir(int, int);
int dotest(int, int);
void Warn(int, char*, char*);
int mkname(char*, int, int);
int term();
void cleanup(); 

/*--------------------------------------------------------------*/


#define	M	(1024*1024)
/* #define temp stderr */

int	iterations;			/* # total iterations */
int	nchild;
int	parent_pid;
int	pidlist[MAXCHILD];

char	homedir[MAXPATHLEN];
char	dirname[MAXPATHLEN];
char	tmpname[MAXPATHLEN];
char	*prog;
int	dirlen;
int 	mnt = 0;
char	startdir[MAXPATHLEN], mntpoint[MAXPATHLEN], newfsstring[90];
char	*partition;
char 	*cwd;
char 	*fstyp;
int 	local_flag;

/*--------------------------------------------------------------*/
int main (ac, av)
	int  ac;
	char *av[];
{
	register int k, j;
	int	pid;
	int	child;
	int	status;
	int	count;
	char	name[128];


	/*
	 * Default values for run conditions.
	 */

	iterations = 50;
	nchild = 5;

	if (signal(SIGTERM, (void (*)())term) == SIG_ERR) {
		tst_resm(TFAIL, "first signal failed");
		tst_exit();
	}

	/*
	 * Make a directory to do this in; ignore error if already exists.
	 */
	

	local_flag = PASSED;
	parent_pid = getpid();
	tst_tmpdir();

	if (!startdir[0]) {
		if (getcwd(startdir, MAXPATHLEN) == NULL) {
			tst_resm(TBROK,"getcwd failed");
			tst_exit();
		}
	}
	cwd = startdir;
	strcat(dirname, cwd);
	sprintf(tmpname, "/ftest02.%d", getpid());
	strcat(dirname, tmpname);
	strcat(homedir, cwd);
	sprintf(tmpname, "/ftest02h.%d", getpid());
	strcat(homedir, tmpname);

	mkdir(dirname, 0755);
	mkdir(homedir, 0755);
	if (chdir(dirname) < 0) {
		tst_resm(TBROK,"\tCan't chdir(%s), error %d.", dirname, errno);
		cleanup();
		tst_exit();
	}
	dirlen = strlen(dirname);
	if (chdir(homedir) < 0) {
		tst_resm(TBROK,"\tCan't chdir(%s), error %d.", homedir, errno);
		cleanup();
		tst_exit();
	}


	for(k = 0; k < nchild; k++) {
		if ((child = fork()) == 0) {		/* child */
			dotest(k, iterations);		/* do it! */
			exit(0);
		}
		if (child < 0) {
			tst_resm(TINFO, "System resource may be too low, fork() malloc()"
					   " etc are likely to fail.");
			tst_resm(TBROK, "Test broken due to inability of fork.");
			cleanup();
		}
		pidlist[k] = child;
	} /* end for */

		/*
		 * Wait for children to finish.
	 */

	count = 0;
	while((child = wait(&status)) > 0) {
		//tst_resm(TINFO,"Test{%d} exited status = 0x%x", child, status);
		//tst_resm(TINFO,"status is %d",status);
		if (status) {
			tst_resm(TFAIL,"Test{%d} failed, expected 0 exit.", child);
			local_flag = FAILED;
		}
		++count;
	}

	/*
	 * Should have collected all children.
	 */

	if (count != nchild) {
		tst_resm(TFAIL,"Wrong # children waited on, count = %d", count);
		local_flag = FAILED;
	}

	if (local_flag == FAILED) {
		tst_resm(TFAIL, "Test failed in fork-wait part.");
	} else {
		tst_resm(TPASS, "Test passed in fork-wait part.");
	}

	if (iterations > 26)
		iterations = 26;
	for (k=0; k < nchild; k++)
		for (j=0; j < iterations + 1; j++) {
			mkname(name, k, j);
			rmdir(name);
			unlink(name);
		}

	chdir(startdir);

	pid = fork();
	if (pid < 0) {
		tst_resm(TINFO, "System resource may be too low, fork() malloc()"
				    " etc are likely to fail.");
		tst_resm(TBROK, "Test broken due to inability of fork.");
		cleanup();
	}
	if (pid == 0) {
		execl("/bin/rm", "rm", "-rf", homedir, NULL);
		exit(1);
	} else
		wait(&status);
	if (status)
		tst_resm(TINFO,"CAUTION - ftest02, '%s' may not have been removed.",
		  homedir);

	pid = fork();
	if (pid < 0) {
		tst_resm(TINFO, "System resource may be too low, fork() malloc()"
	                        " etc are likely to fail.");
	        tst_resm(TBROK, "Test broken due to inability of fork.");
	        cleanup();
	}
	if (pid == 0) {
		execl("/bin/rm", "rm", "-rf", dirname, NULL);
		exit(1);
	} else
		wait(&status);
	if (status) {
		tst_resm(TINFO,"CAUTION - ftest02, '%s' may not have been removed.",
		  dirname);
	}

	sync();				/* safeness */

	cleanup();

	return(0);

}

/*--------------------------------------------------------------*/



#define	warn(val,m1,m2)	if ((val) < 0) Warn(me,m1,m2)

/*
 * crfile()
 *	Create a file and write something into it.
 */

char	crmsg[] = "Gee, let's write something in the file!\n";

void crfile(me, count)
{
	int	fd;
	int	val;
	char	fname[128];
	char	buf[128];

	mkname(fname, me, count);

	fd = open(fname, O_RDWR|O_CREAT|O_TRUNC, 0666);
	if (fd < 0 && errno == EISDIR) {
		val = rmdir(fname);
		warn(val, (char*)"rmdir", fname);
		fd = open(fname, O_RDWR|O_CREAT|O_TRUNC, 0666);
	}
	warn(fd, "creating", fname);

	val = lseek(fd, (rand() % M), 0);
	warn(val, "lseek", 0);

	val = write(fd, crmsg, sizeof(crmsg)-1);
	warn(val, "write", 0);

	val = lseek(fd, -(sizeof(crmsg)-1), 1);
	warn(val, "lseek", 0);

	val = read(fd, buf, sizeof(crmsg)-1);
	warn(val, "read", 0);

	if (strncmp(crmsg, buf, sizeof(crmsg)-1)) Warn(me, "compare", 0);

	val = close(fd);
	warn(val, "close", 0);
}

/*
 * unlfile()
 *	Unlink some of the files.
 */

void unlfile(me, count)
{
	int	i;
	int	val;
	char	fname[128];

	i = count - 10;
	if (i < 0)
		i = 0;
	for(; i < count; i++) {
		mkname(fname, me, i);
		val = rmdir(fname);
		if (val < 0 )
			val = unlink(fname);
		if (val == 0 || errno == ENOENT)
			continue;
		Warn(me, "unlink", fname);
	}
}

/*
 * fussdir()
 *	Make a directory, put stuff in it, remove it, and remove directory.
 *
 * Randomly leave the directory there.
 */

void fussdir(me, count)
{
	int	val;
	char	dir[128];
	char	fname[128];
	char	savedir[128];

	mkname(dir, me, count);
	rmdir(dir); unlink(dir);		/* insure not there */

	val = mkdir(dir, 0755);
	warn(val, "mkdir", dir);

	/*
	 * Arrange to create files in the directory.
	 */

	strcpy(savedir, dirname);
	strcpy(dirname, "");

	val = chdir(dir);
	warn(val, "chdir", dir);

	crfile(me, count);
	crfile(me, count+1);

	val = chdir("..");
	warn(val, "chdir", "..");

	val = rmdir(dir);
	if (val >= 0) {
		tst_resm(TFAIL,"Test[%d]: rmdir of non-empty %s succeeds!", me, dir);
		tst_exit();
	}

	val = chdir(dir);
	warn(val, "chdir", dir);

	mkname(fname, me, count);
	val = unlink(fname);
	warn(val, "unlink", fname);

	mkname(fname, me, count+1);
	val = unlink(fname);
	warn(val, "unlink", fname);

	val = chdir(homedir);
	warn(val, "chdir", homedir);

	if (rand() & 0x01) {
		val = rmdir(dir);
		warn(val, "rmdir", dir);
	}

	strcpy(dirname, savedir);
}


/*
 * dotest()
 *	Children execute this.
 *
 * Randomly do an inode thing; loop for # iterations.
 */


#define	THING(p)	{p, "p"}

struct	ino_thing {
	void	(*it_proc)();
	char	*it_name;
}	ino_thing[] = {
	THING(crfile),
	THING(unlfile),
	THING(fussdir),
	THING(sync),
};

#define	NTHING	(sizeof(ino_thing) / sizeof(ino_thing[0]))

int	thing_cnt[NTHING];
int	thing_last[NTHING];

int dotest(me, count)
	int	me;
	int	count;
{
	int	i;
	int	thing;

	//tst_resm(TINFO,"Test %d pid %d starting.", me, getpid());

	srand(getpid());
	for(i = 0; i < count; i++) {
		thing = (rand() >> 3) % NTHING;
		(*ino_thing[thing].it_proc)(me, i, ino_thing[thing].it_name);
		++thing_cnt[thing];
	}

	//tst_resm(TINFO,"Test %d pid %d exiting.", me, getpid());
	return(0);
}


void Warn(me, m1, m2)
	int	me;
	char	*m1;
	char	*m2;
{
	int	err = errno;

	tst_resm(TBROK,"Test[%d]: error %d on %s %s",
		me, err, m1, (m2 ? m2 : ""));
	tst_exit();
}

int mkname(name, me, idx)
	register char	*name;
{
	register int len;

	(void) strcpy(name, dirname);
	if (name[0]) {
		len = dirlen+1;
		name[len-1] = '/';
	} else
		len = 0;
	name[len+0] = 'A' + (me % 26);
	name[len+1] = 'a' + (idx % 26);
	name[len+2] = '\0';
	return(0);
}

/*--------------------------------------------------------------*/

/* term1()
 *
 *	Parent - this is called when a SIGTERM signal arrives.
 */

int term()
{
	register int i;

	//tst_resm(TINFO, "term -[%d]- got sig term.", getpid());
	
	if (parent_pid == getpid()) {
		for (i=0; i < nchild; i++)
			if (pidlist[i])		/* avoid embarassment */
				kill(pidlist[i], SIGTERM);
		return(0);
	}
	
	tst_resm(TBROK, "Child process exiting.");
	tst_exit();
	return(0);
}

void cleanup()
{
	char mount_buffer[1024];

	if (mnt == 1) {
		if (chdir(startdir) < 0) {
			tst_resm(TBROK,"Could not change to %s ", startdir);
		}
		if (!strcmp(fstyp, "cfs")) {
			sprintf(mount_buffer, "/etc/umount %s", partition);
			if (system(mount_buffer) != 0) {
				tst_resm(TBROK,"Unable to unmount %s from %s ", partition, mntpoint);
				if (umount(partition)) {
					tst_resm(TBROK,"Unable to unmount %s from %s ", partition, mntpoint);
				}
				else {
					tst_resm(TINFO, "Forced umount for %s, /etc/mnttab now dirty", partition );
				}
			}
		}
		else {
			if (umount(partition)) {
				tst_resm(TBROK,"Unable to unmount %s from %s ", partition, mntpoint);
			}
		}
		if (rmdir(mntpoint) != 0) {
			tst_resm(TBROK,"Unable to rmdir %s ", mntpoint);
		}
	}
	tst_rmdir();
	tst_exit();
}
