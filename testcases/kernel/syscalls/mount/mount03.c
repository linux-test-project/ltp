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

#include <sys/types.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

static void help(void);
static void setup(void);
static void cleanup(void);
static int test_rwflag(int, int);
static int setup_uid(void);

char *TCID = "mount03";		/* Test program identifier.    */
int TST_TOTAL = 6;		/* Total number of test cases. */
extern char **environ;		/* pointer to this processes env */

#define DEFAULT_FSTYPE	"ext2"
#define TEMP_FILE	"temp_file"
#define FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#define SUID_MODE	(S_ISUID|S_IRUSR|S_IXUSR|S_IXGRP|S_IXOTH)

static char *Fstype;

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

long rwflags[] = {
	MS_RDONLY,
	MS_NODEV,
	MS_NOEXEC,
	MS_SYNCHRONOUS,
	MS_RDONLY,
	MS_NOSUID,
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
	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/* Check for mandatory option of the testcase */
	if (!Dflag) {
		tst_brkm(TBROK, NULL,
		    "you must specify the device used for mounting with -D "
		    "option");
	}

	if (Tflag) {
		Fstype = (char *)malloc(strlen(fstype)+1);
		if (Fstype == NULL) {
			tst_brkm(TBROK|TERRNO, NULL, "malloc failed");
		}
		Fstype[strlen(fstype)] = '\0';
		strncpy(Fstype, fstype, strlen(fstype));
	} else {
		Fstype = (char *)malloc(strlen(DEFAULT_FSTYPE)+1);
		if (Fstype == NULL) {
			tst_brkm(TBROK|TERRNO, NULL, "malloc failed");
		}
		strncpy(Fstype, DEFAULT_FSTYPE, strlen(DEFAULT_FSTYPE));
		Fstype[strlen(DEFAULT_FSTYPE)] = '\0';
	}

	if (STD_COPIES != 1) {
		tst_resm(TINFO, "-c option has no effect for this testcase - "
			 "%s doesn't allow running more than one instance "
			 "at a time", TCID);
		STD_COPIES = 1;
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			/* Call mount(2) */
			TEST(mount(device, mntpoint, Fstype, rwflags[i], NULL));

			/* check return code */
			if (TEST_RETURN != 0) {
				tst_resm(TFAIL|TTERRNO, "mount(2) failed");
				continue;
			}

			/* Validate the rwflag */
			if (test_rwflag(i, lc) == 1) {
				tst_resm(TFAIL, "mount(2) failed while"
				    " validating %ld", rwflags[i]);
			} else {
				tst_resm(TPASS, "mount(2) passed with "
				    "rwflag = %ld", rwflags[i]);
			}
			TEST(umount(mntpoint));
			if (TEST_RETURN != 0) {
				tst_brkm(TBROK|TTERRNO, cleanup,
				    "umount(2) failed for %s", mntpoint);
			}
		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();
}

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
				tst_resm(TWARN|TERRNO,
				    "open didn't fail with EROFS");
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
					tst_resm(TWARN|TERRNO,
					    "open didn't fail with EACCES");
					return 1;
				}
			}
			close(fd);
		} else {
			tst_resm(TWARN|TERRNO, "mknod(2) failed to create %s",
			    file);
			return 1;
		}
		return 1;
	case 2:
		/* Validate MS_NOEXEC flag of mount call */

		snprintf(file, PATH_MAX, "%stmp1", Path_name);
		if ((fd = open(file, O_CREAT | O_RDWR, S_IRWXU)) == -1) {
			tst_resm(TWARN|TERRNO, "opening %s failed", file);
		} else {
			close(fd);
			execlp(file, basename(file), NULL);
		}
		return 1;
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
			tst_resm(TWARN|TERRNO,
			    "open(%s, O_RDWR|O_CREAT, %#o) failed",
			    file, FILE_MODE);
			return 1;
		}

		/* Write the buffer data into file */
		if (write(fildes, write_buffer, strlen(write_buffer)) !=
		    strlen(write_buffer)) {
			tst_resm(TWARN|TERRNO, "writing to %s failed", file);
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
				tst_resm(TWARN, "Data read from %s and written "
				    "mismatch", file);
				close(fildes);
				return 1;
			} else {
				close(fildes);
				return 0;
			}
		} else {
			tst_resm(TWARN|TERRNO, "read() Fails on %s", file);
			close(fildes);
			return 1;
		}

	case 4:
		/* Validate MS_REMOUNT flag of mount call */

		TEST(mount(device, mntpoint, Fstype, MS_REMOUNT, NULL));
		if (TEST_RETURN != 0) {
			tst_resm(TWARN|TTERRNO, "mount(2) failed to remount");
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

		if (setup_uid() != 0) {
			tst_resm(TBROK|TERRNO, "setup_uid failed");
			return 1;
		}
		switch (pid = fork()) {
		case -1:
			tst_resm(TBROK|TERRNO, "fork failed");
			return 1;
		case 0:
			snprintf(file, PATH_MAX, "%ssetuid_test", Path_name);
			if (chmod(file, SUID_MODE) != 0) {
				tst_resm(TWARN, "chmod(%s, %#o) failed",
				    file, SUID_MODE);
			}

			ltpuser = getpwnam(nobody_uid);
			if (setreuid(ltpuser->pw_uid, ltpuser->pw_uid) == -1) {
				tst_resm(TWARN|TERRNO,
				    "seteuid() failed to change euid to %d",
				    ltpuser->pw_uid);
			}
			execlp(file, basename(file), NULL);
			exit(1);
		default:
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
int setup_uid()
{
	int pid, status;
	char command[PATH_MAX];

	switch (pid = fork()) {
	case -1:
		tst_resm(TWARN|TERRNO, "fork failed");
		return 1;
	case 0:
		Cmd_buffer[0] = cmd;
		Cmd_buffer[1] = testhome_path;
		Cmd_buffer[2] = Path_name;

		/* Put command into string */
		sprintf(command, "%s %s %s", cmd, testhome_path, Path_name);

		/* Run command to cp file to right spot */
		if (system(command) == 0) {
			execlp(file, basename(file), NULL);
		} else {
			printf("call to %s failed\n", command);
		}
		exit(1);
	default:
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			return WEXITSTATUS(status);
		} else if (WIFSIGNALED(status)) {
			return WTERMSIG(status);
		} else {
			/* Should be 0. */
			assert (status == 0);
			return 0;
		}
	}
}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{
	char *test_home;	/* variable to hold TESTHOME env */
	struct stat setuid_test_stat;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Check whether we are root */
	if (geteuid() != 0) {
		free(Fstype);
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}

	/* Test home directory */
	test_home = get_current_dir_name();

	/* make a temp directory */
	tst_tmpdir();

	/* Unique mount point */
	(void)sprintf(mntpoint, "mnt_%d", getpid());

	if (mkdir(mntpoint, DIR_MODE)) {
		tst_brkm(TBROK|TERRNO, cleanup, "mkdir(%s, %#o) failed",
		    mntpoint, DIR_MODE);
	}
	/* Get the current working directory of the process */
	if (getcwd(Path_name, sizeof(Path_name)) == NULL) {
		tst_brkm(TBROK, cleanup, "getcwd failed");
	}
	if (chmod(Path_name, DIR_MODE) != 0) {
		tst_brkm(TBROK, cleanup, "chmod(%s, %#o) failed",
		    Path_name, DIR_MODE);
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
		if (setuid_test_stat.st_mode != SUID_MODE &&
		    chmod(file, SUID_MODE) < 0) {
			tst_brkm(TBROK, cleanup,
					"setuid for setuid_test failed");
		}
	}

	/*
	 * under temporary directory
	 */
	snprintf(Path_name, PATH_MAX, "%s/%s/", Path_name, mntpoint);

	strcpy(testhome_path, test_home);
	strcat(testhome_path, "/setuid_test");

	TEST_PAUSE;

}

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

	tst_rmdir();
}

/*
 * issue a help message
 */
void help()
{
	printf("-T type	  : specifies the type of filesystem to be mounted."
	       " Default ext2. \n");
	printf("-D device : device used for mounting \n");
}