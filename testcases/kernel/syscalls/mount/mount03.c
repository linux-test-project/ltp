/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**************************************************************************
 *
 *    TEST IDENTIFIER	: mount03
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Test for checking mount(2) flags
 *
 *    TEST CASE TOTAL	: 6
 *
 *    AUTHOR		: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Check for basic mount(2) system call flags.
 *
 *	Verify that mount(2) syscall passes for each flag setting and validate
 *	the flags
 *	1) MS_RDONLY - mount read-only.
 *	2) MS_NODEV - disallow access to device special files.
 *	3) MS_NOEXEC - disallow program execution.
 *	4) MS_SYNCHRONOUS - writes are synced at once.
 *	5) MS_REMOUNT - alter flags of a mounted FS.
 *	6) MS_NOSUID - ignore suid and sgid bits.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Create a mount point.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	  Execute mount system call for each flag
 *	  Validate each flag setting. if validation fails
 *		Delete the mount point.
 *		Log the errno and Issue a FAIL message.
 *	  Delete the mount point.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *	  Delete the temporary directory(s)/file(s) created.
 *
 * USAGE:  <for command-line>
 *  mount03 [-T type] -D device [-e] [-i n] [-I x] [-p x] [-t]
 *			where,  -T type : specifies the type of filesystem to
 *					  be mounted. Default ext2.
 *				-D device : device to be mounted.
 *				-e   : Turn on errno logging.
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 * RESTRICTIONS
 *	test must run with the -D option
 *	test doesn't support -c option to run it in parallel, as mount
 *	syscall is not supposed to run in parallel.
 *****************************************************************************/

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

static void help(void);
static void setup(void);
static void cleanup(void);
static int test_rwflag(int, int);
static void setup_uid(void);

char *TCID = "mount03";		/* Test program identifier.    */
int TST_TOTAL = 6;		/* Total number of test cases. */
extern int Tst_count;		/* TestCase counter for tst_* routine */
extern char **environ;		/* pointer to this processes env */

#define DEFAULT_FSTYPE	"ext2"
#define TEMP_FILE	"temp_file"
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define DIR_MODE	S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH
#define SUID_MODE	S_ISUID|S_IRUSR|S_IXUSR|S_IXGRP|S_IXOTH

static char *Fstype;

static unsigned long Flag;

static char mntpoint[20];
static char *fstype;
static char *device;
static int Tflag = 0;
static int Dflag = 0;

static char write_buffer[BUFSIZ];	/* buffer used to write data to file */
static char read_buffer[BUFSIZ];	/* buffer used to read data from file */
static int fildes;		/* file descriptor for temporary file */
static char *Cmd_buffer[3];	/* Buffer to hold command string */
static char Path_name[PATH_MAX];	/* Buffer to hold command string */
static char testhome_path[PATH_MAX];	/* Test home Path                */
static char file[PATH_MAX];	/* Temporary file                */
static char cmd[] = "cp";

static struct test_case_t {
	char *rwflag_desc;	/* error description            */
	unsigned long rwflag;	/* Expected error no            */
	char *rwdesc;		/* rwflag                       */
} testcases[] = {
	{
	"mount read-only", MS_RDONLY, "MS_RDONLY"}, {
	"disallow access to device special files", MS_NODEV, "MS_NODEV"}, {
	"disallow program execution", MS_NOEXEC, "MS_NOEXEC"}, {
	"writes are synced at once", MS_SYNCHRONOUS, "MS_SYNCHRONOUS"}, {
	"alter flags of a mounted FS", MS_RDONLY, "MS_REMOUNT"}, {
	"ignore suid and sgid bits", MS_NOSUID, "MS_NOSUID"}
};

static option_t options[] = {	/* options supported by mount03 test */
	{"T:", &Tflag, &fstype},	/* -T type of filesystem        */
	{"D:", &Dflag, &device},	/* -D device used for mounting  */
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, options, &help)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}
	/* Check for mandatory option of the testcase */
	if (!Dflag) {
		tst_resm(TWARN, "You must specifiy the device used for "
			 " mounting with -D option, Run '%s  -h' for option "
			 " information.", TCID);
		tst_exit();
	}

	if (Tflag) {
		Fstype = (char *)malloc(strlen(fstype)+1);
		if (Fstype == NULL) {
			tst_brkm(TBROK, NULL, "malloc failed to alloc %d errno "
				 " %d ", strlen(fstype)+1, errno);
		}
		memset(Fstype, 0, strlen(fstype)+1);
		strncpy(Fstype, fstype, strlen(fstype));
	} else {
		Fstype = (char *)malloc(strlen(DEFAULT_FSTYPE)+1);
		if (Fstype == NULL) {
			tst_brkm(TBROK, NULL, "malloc failed to alloc %d errno "
				 " %d ", strlen(fstype)+1, errno);
		}
		memset(Fstype, 0, strlen(DEFAULT_FSTYPE)+1);
		strncpy(Fstype, DEFAULT_FSTYPE, strlen(DEFAULT_FSTYPE));
	}

	if (STD_COPIES != 1) {
		tst_resm(TINFO, "-c option has no effect for this testcase - "
			 "%s doesn't allow running more than one instance "
			 "at a time", TCID);
		STD_COPIES = 1;
	}

	/* perform global setup for test */
	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			Flag = testcases[i].rwflag;

			/* Call mount(2) */
			TEST(mount(device, mntpoint, Fstype, Flag, NULL));

			/* check return code */
			if (TEST_RETURN != 0) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL, "mount(2) failed errno = %d : "
					 "%s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
				continue;
			}

			/* Validate the rwflag */
			if (test_rwflag(i, lc) == 1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL, "mount(2) Failed while"
					 " validating %s", testcases[i].rwdesc);
			} else {
				tst_resm(TPASS, "mount(2) Passed for rwflag"
					 " %s - %s", testcases[i].rwdesc,
					 testcases[i].rwflag_desc);
			}
			TEST(umount(mntpoint));
			if (TEST_RETURN != 0) {
				tst_brkm(TBROK, cleanup, "umount(2) failed to "
					 "to umount %s errno = %d : %s",
					 mntpoint, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			}
		}		/* End of TEST CASE LOOPING. */
	}			/* End for TEST_LOOPING */

	/* cleanup and exit */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/*
 * test_rwflag(int i, int cnt)
 * Validate the mount system call for rwflags.
 */

int test_rwflag(int i, int cnt)
{
	int fd, pid, status;
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;

	switch (i) {
	case 0:
		/* Validate MS_RDONLY flag of mount call */

		snprintf(file, PATH_MAX, "%stmp", Path_name);
		if ((fd = open(file, O_CREAT | O_RDWR, S_IRWXU)) == -1) {
			if (errno == EROFS) {
				return 0;
			} else {
				tst_resm(TWARN, "open(%s) failed with "
					 " error %d instead EACCES",
					 file, errno);
				return 1;
			}
		}
		close(fd);
		return 1;
	case 1:
		/* Validate MS_NODEV flag of mount call */

		snprintf(file, PATH_MAX, "%smynod_%d_%d", Path_name, getpid(),
			       cnt);
		if (mknod(file, S_IFBLK | 0777, 0) == 0) {
			if ((fd = open(file, O_RDWR, S_IRWXU)) == -1) {
				if (errno == EACCES) {
					return 0;
				} else {
					tst_resm(TWARN, "open(%s) "
						 "failed  with error %d instead "
						 "EACCES", file, errno);
					return 1;
				}
			}
			close(fd);
		} else {
			tst_resm(TWARN, "mknod(2) failed to creat "
				 "device %s errno = %d : %s", file,
				 errno, strerror(errno));
			return 1;
		}
		return 1;
	case 2:
		/* Validate MS_NOEXEC flag of mount call */

		snprintf(file, PATH_MAX, "%stmp1", Path_name);
		if ((fd = open(file, O_CREAT | O_RDWR, S_IRWXU)) == -1) {
			tst_resm(TWARN,
				 "open() of %s failed with error" " %d : %s",
				 file, errno, strerror(errno));
			return 1;
		} else {
			close(fd);
			if (execve(file, NULL, NULL) == -1) {
				return 0;
			} else {
				return 1;
			}
		}
	case 3:
		/*
		 * Validate MS_SYNCHRONOUS flag of mount call.
		 * Copy some data into data buffer.
		 */

		strcpy(write_buffer, "abcdefghijklmnopqrstuvwxyz");

		/* Creat a temporary file under above directory */
		snprintf(file, PATH_MAX, "%s%s", Path_name, TEMP_FILE);
		if ((fildes = open(file, O_RDWR | O_CREAT, FILE_MODE))
		    == -1) {
			tst_resm(TWARN, "open(%s, O_RDWR | O_CREAT,"
				 " %#o) Failed, errno=%d :%s",
				 file, FILE_MODE, errno, strerror(errno));
			return 1;
		}

		/* Write the buffer data into file */
		if (write(fildes, write_buffer, strlen(write_buffer)) !=
		    strlen(write_buffer)) {
			tst_resm(TWARN, "write() failed to write buffer"
				 " data to %s", file);
			close(fildes);
			return 1;
		}

		/* Set the file ptr to b'nning of file */
		if (lseek(fildes, 0, SEEK_SET) < 0) {
			tst_resm(TWARN, "lseek() failed on %s, error="
				 " %d", file, errno);
			close(fildes);
			return 1;
		}

		/* Read the contents of file */
		if (read(fildes, read_buffer, sizeof(read_buffer)) > 0) {
			if (strcmp(read_buffer, write_buffer)) {
				tst_resm(TWARN, "Data read "
					 "from %s doesn't match with"
					 " written data", file);
				close(fildes);
				return 1;
			} else {
				close(fildes);
				return 0;
			}
		} else {
			tst_resm(TWARN, "read() Fails on %s, error=%d",
				 file, errno);
			close(fildes);
			return 1;
		}

	case 4:
		/* Validate MS_REMOUNT flag of mount call */

		TEST(mount(device, mntpoint, Fstype, MS_REMOUNT, NULL));
		if (TEST_RETURN != 0) {
			tst_resm(TWARN, "mount(2) Failed to remount "
				 "errno = %d : %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
			return 1;
		} else {
			snprintf(file, PATH_MAX, "%stmp2", Path_name);
			if ((fd = open(file, O_CREAT | O_RDWR, S_IRWXU))
			    == -1) {
				tst_resm(TWARN, "open(%s) on readonly "
					 "filesystem passed", file);
				return 1;
			} else {
				close(fd);
				return 0;
			}
		}
	case 5:
		/* Validate MS_NOSUID flag of mount call */

		setup_uid();
		if ((pid = fork()) == 0) {
			snprintf(file, PATH_MAX, "%ssetuid_test", Path_name);
			if (chmod(file, SUID_MODE) != 0) {
				tst_resm(TWARN, "chmod() failed to "
					 "change mode  %d errno = %d : %s",
					 4511, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			}

			ltpuser = getpwnam(nobody_uid);
			if (setreuid(ltpuser->pw_uid, ltpuser->pw_uid) == -1) {
				tst_resm(TWARN, "seteuid() failed to "
					 "change euid to %d errno = %d : %s",
					 ltpuser->pw_uid, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			}
			execve(file, NULL, NULL);
			/* NOT REACHEAD */
		} else {
			waitpid(pid, &status, 0);
			if (WIFEXITED(status)) {
				/* reset the setup_uid */
				if (status)
					return 0;
				else
					return 1;
			}
		}
	}
	return 0;
}

/* setup_uid() - performs setup for NOUID test */
void setup_uid()
{
	int pid, status;
	char command[PATH_MAX];

	switch (pid = fork()) {
	case -1:
		tst_resm(TWARN, "Unable to fork a child process"
			 " to exec over!  Errno:%d, :%s",
			 errno, strerror(errno));
		break;
	case 0:
		Cmd_buffer[0] = cmd;
		Cmd_buffer[1] = testhome_path;
		Cmd_buffer[2] = Path_name;

		/* Put command into string */
		sprintf(command, "%s %s %s", cmd, testhome_path, Path_name);

		/*Run command to cp file to right spot */
		system(command);

		/* Must kill child */
		execve(file, NULL, NULL);

		/* execve("/bin/cp", Cmd_buffer, environ); */
		exit(errno);
	default:
		waitpid(pid, &status, 0);
	}
	return;
}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{
	char *test_home;	/* variable to hold TESTHOME env */
	struct stat setuid_test_stat;

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Check whether we are root */
	if (geteuid() != 0) {
		free(Fstype);
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}

	/* Test home directory */
	test_home = get_current_dir_name();

	/* make a temp directory */
	tst_tmpdir();

	/* Unique mount point */
	(void)sprintf(mntpoint, "mnt_%d", getpid());

	if (mkdir(mntpoint, DIR_MODE)) {
		tst_brkm(TBROK, cleanup, "mkdir(%s, %#o) failed; "
			 "errno = %d: %s", mntpoint, DIR_MODE, errno,
			 strerror(errno));
	}
	/* Get the current working directory of the process */
	if (getcwd(Path_name, sizeof(Path_name)) == NULL) {
		tst_brkm(TBROK, cleanup,
			 "getcwd(3) fails to get working directory of process");
	}
	if (chmod(Path_name, DIR_MODE) != 0) {
		tst_brkm(TBROK, cleanup, "chmod() failed to change mode %#o "
			 "errno = %d : %s", DIR_MODE, TEST_ERRNO,
			 strerror(TEST_ERRNO));
	}
	snprintf(file, PATH_MAX, "%ssetuid_test", Path_name);
	if (stat(file, &setuid_test_stat) < 0) {
		tst_brkm(TBROK, cleanup, "stat for setuid_test failed");
	} else {
		if ((setuid_test_stat.st_uid || setuid_test_stat.st_gid) &&
		     chown(file, 0, 0) < 0) {
			tst_brkm(TBROK, cleanup,
					"chown for setuid_test failed");
		}
		if (setuid_test_stat.st_mode != 04511 &&
		    chmod(file, 04511) < 0) {
			tst_brkm(TBROK, cleanup,
					"setuid for setuid_test failed");
		}
	}

	/*
	 * Get the complete path of TESTDIR created
	 * under temporary directory
	 */
	snprintf(Path_name, PATH_MAX, "%s/%s/", Path_name, mntpoint);

	strcpy(testhome_path, test_home);

	strcat(testhome_path, "/setuid_test");

	/* Pause if that option was specified */
	TEST_PAUSE;

}				/* End setup() */

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{
	free(Fstype);

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove tmp dir and all files in it  */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */

/*
 * issue a help message
 */
void help()
{
	printf("-T type	  : specifies the type of filesystem to be mounted."
	       " Default ext2. \n");
	printf("-D device : device used for mounting \n");
}
