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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* 11/01/2002	Port to LTP	robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

			   /*inode02.c */
/*======================================================================
	=================== TESTPLAN SEGMENT ===================
CALLS:	mkdir, stat, open

	Run with TERM mode.

>KEYS:  < file system and I/O management, system resource constraints.
>WHAT:  < Can the system handle a heavy load on the file system I/O
	< functions?
>HOW:   < Create several identical process that call inode02.c.  This
	< will simulate the multi-user environment, and hopefully uncover
	< conflicts that might occur in "real life" use.
>BUGS:  <
======================================================================*/

#define PATH_STRING_LENGTH  1024
#define NAME_LENGTH  8
#define MAX_PATH_STRING_LENGTH  (PATH_STRING_LENGTH - NAME_LENGTH - 40)
#define DIRECTORY_MODE  00777
#define FILE_MODE       00777

#define MKDIR_STRING_LENGTH  (MAX_PATH_STRING_LENGTH + 7)

/* #define DEBUG	 you can watch the generation with this flag */

#define TRUE  1
#define FALSE 0
#define READ  0
#define WRITE 1

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#ifdef LINUX
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#endif

#define MAXCHILD	25
int allchild[MAXCHILD + 1];

char name[NAME_LENGTH + 1];
char path_string[PATH_STRING_LENGTH + 1];
char read_string[PATH_STRING_LENGTH + 1];
char write_string[PATH_STRING_LENGTH + 1];
char remove_string[PATH_STRING_LENGTH + 10];
int parent_pid;
int nchild;

FILE *list_stream = NULL;
int list_id;
int file_id;

int increment_name(), get_next_name(), mode(), escrivez(), massmurder();
int max_depth, max_breadth, file_length;
int bd_arg(char *);

#ifdef LINUX
void (*sigset(int, void (*)(int))) (int);
#endif

/** LTP Port **/
#include "test.h"

void setup(void);
void fail_exit(void);
void anyfail(void);
void ok_exit(void);
void forkfail(void);
void terror(char *);
int instress(void);

#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
FILE *temp;

char *TCID = "inode02";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
/**************/

int main(int argc, char *argv[])
{
	int pid, tree(), p, status;
	int count, child;
	register int i;
	int term();

	setup();

	parent_pid = getpid();

	if (sigset(SIGTERM, (void (*)())term) == SIG_ERR) {
		tst_resm(TBROK, "\tSIGTERM sigset set failed, errno=%d\n",
			 errno);
		exit(1);
	}

	/************************************************/
	/*                                              */
	/*  Input the parameters for the directory---   */
	/*  file trees which are to be generated        */
	/*                                              */
	/************************************************/

	if (argc < 2) {
		max_depth = 6;
		max_breadth = 5;
		file_length = 8;
		nchild = 5;
	} else if (argc < 5) {
		tst_resm(TCONF, "Bad argument count.\n");
		printf
		    ("\tinode02 max_depth max_breadth file_length #children\n\tdefault: inode02 6 5 8 5\n");
		exit(1);
	} else {
		i = 1;
		if (sscanf(argv[i++], "%d", &max_depth) != 1)
			bd_arg(argv[i - 1]);
		if (sscanf(argv[i++], "%d", &max_breadth) != 1)
			bd_arg(argv[i - 1]);
		if (sscanf(argv[i++], "%d", &file_length) != 1)
			bd_arg(argv[i - 1]);
		if (sscanf(argv[i++], "%d", &nchild) != 1)
			bd_arg(argv[i - 1]);
		if (nchild > MAXCHILD) {
			fprintf(temp, "too many children - max is %d\n",
				MAXCHILD);
			exit(1);
		}
	}

	/************************************************/
	/*                                              */
	/*  Generate and check nchild trees             */
	/*                                              */
	/************************************************/

	for (p = 0; p < nchild; p++) {
		pid = fork();
		if (pid == 0) {
			tree();
		} else {
			if (pid < 1) {
				terror
				    ("Fork failed (may be OK if under stress)");
				massmurder();
				if (instress()) {
					ok_exit();
				}
				forkfail();
			}
		}
	}

	count = 0;
	while ((child = wait(&status)) > 0) {
#ifdef DEBUG
		tst_resm(TINFO, "Test %d exited status = %d\n", child, status);
#endif
		if (status) {
			fprintf(temp, "Test %d failed - expected 0 exit.\n",
				child);
			local_flag = FAILED;
		}
		count++;
	}

	if (count != nchild) {
		tst_resm(TFAIL, "Wrong number of children waited on!\n");
		tst_resm(TFAIL, "Saw %d, expected %d\n", count, nchild);
		local_flag = FAILED;
	}

	/************************************************/
	/*                                              */
	/*  And report the results..........            */
	/*                                              */
	/************************************************/

	anyfail();
	/** NOT REACHED **/
	tst_exit();
}

int bd_arg(char *str)
{
	fprintf(temp,
		"Bad argument - %s - could not parse as number.\n\tinode02 [max_depth] [max_breadth] [file_length] [#children]\n\tdefault: inode02 6 5 8 5\n",
		str);
	exit(1);
}

int tree(void)

/************************************************/
/*						*/
/*  		      TREE			*/
/*						*/
/*   generate a tree of directories and files   */
/*   and save the path names in the path_list	*/
/*   file 					*/
/*						*/
/*   then, read the path names and attempt to   */
/*   access the corresponding directories and	*/
/*   files					*/
/*						*/
/************************************************/
{
	int gen_ret_val, ch_ret_val, exit_val, level;
	int ret_val;
	int generate(), check();
	char path_list_string[PATH_STRING_LENGTH + 10];
	int len;
	int status;
	int snp_ret;

	/********************************/
	/*                              */
	/*  make the root directory for */
	/*  the tree                    */
	/*                              */
	/********************************/

	sprintf(path_string, "inode02.%d", getpid());

	ret_val = mkdir(path_string, DIRECTORY_MODE);

	if (ret_val == -1) {
		tst_resm(TBROK,
			 "Reason: Impossible to create directory %s, errno=%d\n",
			 path_string, errno);
		exit(-5);
	}

	strcpy(remove_string, "rm -rf ");
	strcat(remove_string, path_string);

#ifdef DEBUG
	tst_resm(TINFO, "\n%s\n", path_string);
#endif

	/****************************************/
	/*                                      */
	/*  create the "path_list" file, in     */
	/*  which the list of generated paths   */
	/*  will be stored so that they later   */
	/*  may be checked                      */
	/*                                      */
	/****************************************/

	snp_ret = snprintf(path_list_string, sizeof(path_list_string),
		"%s/path_list",	path_string);
	if (snp_ret < 0 || snp_ret >= sizeof(path_list_string)) {
		tst_resm(TBROK, "snprintf(path_list_string,..) returned %d",
			snp_ret);
		exit(-1);
	}
	list_id = creat(path_list_string, FILE_MODE);
	if (list_id == -1) {
		fprintf(temp,
			"\nThe path_list file '%s' cannot be created, errno=%d\n",
			path_list_string, errno);
		exit(-7);
	}

	/****************************************/
	/*                                      */
	/*   and store its name in path_list    */
	/*                                      */
	/****************************************/

	strcpy(write_string, path_string);
	len = strlen(write_string);
	write_string[len++] = 'D';
	write_string[len] = '\0';
	escrivez(write_string);

	/****************************************/
	/*                                      */
	/*   generate the directory-file tree   */
	/*                                      */
	/****************************************/

	level = 0;

#ifdef DEBUG
	tst_resm(TINFO, "\n\t%s\n\n", "GENERATING:");
#endif

	gen_ret_val = generate(path_string, level);
	close(list_id);
	list_id = open(path_list_string, READ);
	if (list_id == -1) {
		fprintf(temp,
			"\nThe path_list file cannot be opened for reading, errno=%d\n",
			errno);
		exit(-8);
	}
	list_stream = fdopen(list_id, "r");

	/****************************************/
	/*                                      */
	/*   check the directory-file tree      */
	/*      for correctness                 */
	/*                                      */
	/****************************************/

#ifdef DEBUG
	tst_resm(TINFO, "\n\t%s\n\n", "CHECKING:");
#endif

	ch_ret_val = check();

	exit_val = MIN(ch_ret_val, gen_ret_val);

	status = fclose(list_stream);
	if (status != 0) {
		fprintf(temp,
			"Failed to close list_stream: ret=%d errno=%d (%s)\n",
			status, errno, strerror(errno));
		exit(-8);
	}

	/*
	 * Remove file.
	 */

	status = system(remove_string);
	if (status) {
		fprintf(temp, "Caution - `%s' failed.\n", remove_string);
		fprintf(temp, "Status returned %d.\n", status);
	}

	/****************************************/
	/*                                      */
	/*         .....and exit main           */
	/*                                      */
	/****************************************/

	exit(exit_val);
}

int generate(char *string, int level)

/****************************************/
/*					*/
/*   generate recursively a tree of	*/
/*   directories and files:  within   	*/
/*   created directory, an alternating	*/
/*   series of files and directories 	*/
/*   are constructed---until tree	*/
/*   breadth and depth limits are	*/
/*   reached or an error occurs		*/
/*					*/
/****************************************/
/***************************/
/*  string:                */
/*  the directory path     */
/*  string below which a   */
/*  tree is generated      */
/*                         */
/***************************/

/***************************/
/* level:                  */
/* the tree depth variable */
/*                         */
/***************************/
{
	int switch_flag;
	int ret_val = 0;
	int new_ret_val, len, ret_len;
	char new_string[PATH_STRING_LENGTH + 1];
	int new_level;
	int i, j;		/* iteration counters */
	int snp_ret;

	switch_flag = level & TRUE;
	if (strlen(string) >= MAX_PATH_STRING_LENGTH) {

		/********************************/
		/*                              */
		/*   Maximum path name length   */
		/*          reached             */
		/*                              */
		/********************************/

		fprintf(temp, "\nMaximum path_name length reached\n");
		return (-1);
	} else if (level < max_depth) {
		for (i = 0; i <= max_breadth; i++) {
			get_next_name();
			snp_ret = snprintf(new_string, sizeof(new_string),
				"%s/%s", string, name);
			if (snp_ret < 0 || snp_ret >= sizeof(new_string)) {
				tst_resm(TBROK, "snprintf(new_string,..) "
					"returned %d", snp_ret);
				exit(-1);
			}

			/****************************************/
			/*                                      */
			/*    switch between creating files     */
			/*    and making directories            */
			/*                                      */
			/****************************************/

			if (switch_flag) {
				switch_flag = FALSE;

				/****************************************/
				/*                                      */
				/*        create a new file             */
				/*                                      */
				/****************************************/

				file_id = creat(new_string, FILE_MODE);
				if (file_id == -1) {
					fprintf(temp,
						"\nImpossible to create file %s, errno=%d\n",
						new_string, errno);
					return (-2);
				}
#ifdef DEBUG
				tst_resm(TINFO, "%d  %s F\n", level,
					 new_string);
#endif

				/****************************************/
				/*                                      */
				/*            write to it               */
				/*                                      */
				/****************************************/

				len = strlen(new_string);
				for (j = 1; j <= file_length; j++) {
					ret_len =
					    write(file_id, new_string, len);
					if (ret_len != len) {
						fprintf(temp,
							"\nUnsuccessful write to file %s, errno=%d\n",
							new_string, errno);
						return (-3);
					}
				}
				close(file_id);

				/****************************************/
				/*                                      */
				/*   and store its name in path_list    */
				/*                                      */
				/****************************************/

				strcpy(write_string, new_string);
				len = strlen(write_string);
				write_string[len++] = 'F';
				write_string[len] = '\0';
				escrivez(write_string);
			} else {
				switch_flag = TRUE;

				/****************************************/
				/*                                      */
				/*       or make a directory            */
				/*                                      */
				/*  (mknod can only be called when in   */
				/*   super user mode)                   */
				/*                                      */
				/****************************************/

				ret_val = mkdir(new_string, DIRECTORY_MODE);

				if (ret_val != 0) {
					fprintf(temp,
						"\nImpossible to create directory %s, errno=%d\n",
						new_string, errno);
					return (-5);
				}
#ifdef DEBUG
				tst_resm(TINFO, "%d  %s D\n", level,
					 new_string);
#endif

				/****************************************/
				/*                                      */
				/*     store its name in path_list      */
				/*                                      */
				/****************************************/

				strcpy(write_string, new_string);
				len = strlen(write_string);
				write_string[len++] = 'D';
				write_string[len] = '\0';
				escrivez(write_string);

				/****************************************/
				/*                                      */
				/*      and generate a new level        */
				/*                                      */
				/****************************************/

				new_level = level + 1;
				new_ret_val = generate(new_string, new_level);
				if (new_ret_val < ret_val)
					ret_val = new_ret_val;
			}
		}

		/********************************/
		/*                              */
		/*    Maximum breadth reached   */
		/*                              */
		/********************************/

		return (ret_val);
	} else
		    /********************************/
		/*                             */
		/*    Maximum depth reached    */
		/*                             */
 /********************************/
		return 0;
}

int check(void)

/****************************************/
/*					*/
/*   check for file and directory	*/
/*   correctness by reading records	*/
/*   from the path_list and attempting	*/
/*   to determine if the corresponding	*/
/*   files or directories are as 	*/
/*   created 				*/
/*					*/
/****************************************/
{
	int len, path_mode, val, ret_len, j;

	for (;;) {

		/****************************************/
		/*                                      */
		/*  read a path string from path_list   */
		/*                                      */
		/****************************************/

		if (fscanf(list_stream, "%s", path_string) == EOF) {

#ifdef DEBUG
			tst_resm(TINFO, "\nEnd of path_list file reached \n");
#endif

			return 0;
		}
#ifdef DEBUG
		tst_resm(TINFO, "%s\n", path_string);
#endif

		len = strlen(path_string);
		len--;
		if (path_string[len] == 'F') {

		/********************************/
			/*                              */
			/*    this should be a file     */
			/*                              */
		/********************************/

			path_string[len] = '\0';
			file_id = open(path_string, READ);
			if (file_id <= 0) {
				fprintf(temp,
					"\nImpossible to open file %s, errno=%d\n",
					path_string, errno);
				return (-1);
			}

			else {
				/********************************/
				/*                              */
				/*    check its contents        */
				/*                              */
				/********************************/

				ret_len = 0;
				len = strlen(path_string);
				for (j = 1; j <= file_length; j++) {
					ret_len =
					    read(file_id, read_string, len);
					if (len != ret_len) {
						fprintf(temp,
							"\nFile read error for file %s, errno=%d\n",
							path_string, errno);
						return (-3);
					}
					read_string[len] = '\0';
					val = strcmp(read_string, path_string);
					if (val != 0) {
						fprintf(temp,
							"\nContents of file %s are different than expected: %s\n",
							path_string,
							read_string);
						return (-4);
					}
				}
				close(file_id);
			}	/* else for */
			if (ret_len <= 0) {
				fprintf(temp,
					"\nImpossible to read file %s, errno=%d\n",
					path_string, errno);
				return (-2);
			}
		} else {

		/********************************/
			/*                              */
			/*  otherwise..........         */
			/*  it should be a directory    */
			/*                              */
		/********************************/

			path_string[len] = '\0';
			path_mode = mode(path_string);
			if (path_mode == -1) {
				fprintf(temp,
					"\nPreviously created directory path %s was not open\n",
					path_string);
				return (-4);
			}
			if ((040000 & path_mode) != 040000) {
				fprintf(temp,
					"\nPath %s was not recognized to be a directory\n",
					path_string);
				fprintf(temp, "Its mode is %o\n", path_mode);
				return (-5);
			}
		}
	}			/* while */
}

int get_next_name(void)

/****************************************/
/*					*/
/*   get the next---in a dictionary	*/
/*   sense---file or directory name	*/
/*					*/
/****************************************/
{
	static int k;
	int i;
	int last_position;

	last_position = NAME_LENGTH - 1;
	if (k == 0) {

		/************************/
		/*                      */
		/*   initialize name    */
		/*                      */
		/************************/

		for (i = 0; i < NAME_LENGTH; i++)
			name[i] = 'a';
		name[NAME_LENGTH] = '\0';
		k++;
	}
					    /********************************/
	/*                              */
	else
		increment_name(last_position);	/* i.e., beginning at the last  */
	/* position                     */
	/*                              */
					    /********************************/
	return 0;
}

int increment_name(int position)

/****************************************/
/*					*/
/*  recursively revise the letters in 	*/
/*  a name to get the lexiographically	*/
/*  next name				*/
/*					*/
/****************************************/
{
	int next_position;

	if (name[position] == 'z')
		if (position == 0) {
			fprintf(temp,
				"ERROR: There are no more available names\n");
			exit(-1);
		} else {
			name[position] = 'a';	       /**********************/
			next_position = --position;	/*                    */
			increment_name(next_position);	/*  increment the     */
			/*  previous letter   */
			/*                    */
						       /**********************/
		}
				  /*********************************/
	/*                               */
	else
		name[position]++;	/* otherwise, increment this one */
	return 0;		/*                               */
				  /*********************************/
}

int mode(char *path_string)

/****************************************/
/*					*/
/*   determine and return the mode of	*/
/*   the file named by path_string 	*/
/*					*/
/****************************************/
{
	struct stat buf;
	int ret_val, mod;

	ret_val = stat(path_string, &buf);
	if (ret_val == -1)
		return (-1);
	else {
		mod = buf.st_mode;
		return (mod);
	}
}

int escrivez(char *string)
{
	char write_string[PATH_STRING_LENGTH + 1];
	int len, ret_len;

	strcpy(write_string, string);
	len = strlen(write_string);
	write_string[len] = '\n';
	len++;
	ret_len = write(list_id, write_string, len);
	if (len != ret_len) {
		fprintf(temp,
			"A string of deviant length %d written to path_list, errno=%d\n",
			ret_len, errno);
		exit(-2);
	}
	return 0;
}

int term(void)
{
	int status;

	fflush(temp);
	if (parent_pid == getpid()) {
		massmurder();	/* kill kids */
		fprintf(temp, "\term1 - SIGTERM received by parent.\n");
		fflush(temp);
	} else {
		fprintf(temp, "\tchild - got SIGTERM signal.\n");
		if (list_stream != NULL)
			fclose(list_stream);
		close(list_id);
		close(file_id);
		status = system(remove_string);
		if (status) {
			fprintf(temp, "Caution - ``%s'' returned status %d\n",
				remove_string, status);
		}
		exit(0);
	}
	return 0;
}

int massmurder(void)
{
	int i;
	for (i = 0; i < MAXCHILD; i++) {
		if (allchild[i]) {
			kill(allchild[i], SIGTERM);
		}
	}
	return 0;
}

/** LTP Port **/
/*
 * setup
 *
 * Do set up - here its a dummy function
 */
void setup(void)
{
	tst_tmpdir();
	temp = stderr;
}

/*
 * fail_exit()
 *
 * Exit on failure
 */
void fail_exit(void)
{
	tst_brkm(TFAIL, tst_rmdir, "Test failed\n");
}

/*
 *
 * Function: anyfail()
 *
 * Description: Exit a test.
 */
void anyfail(void)
{
	(local_flag == FAILED) ? tst_resm(TFAIL, "Test failed")
	    : tst_resm(TPASS, "Test passed");
	tst_rmdir();
	tst_exit();
}

/*
 * ok_exit
 *
 * Calling block passed the test
 */
void ok_exit(void)
{
	local_flag = PASSED;
	return;
}

/*
 * forkfail()
 *
 * exit on failure
 */
void forkfail(void)
{
	tst_brkm(TBROK, tst_rmdir, "Reason: %s\n", strerror(errno));
}

/*
 * Function: terror
 *
 * Description: prints error message this may not be because some part of the
 *              test case failed, for example fork() failed. We will log this
 *              failure as TBROK instead of TFAIL.
 */
void terror(char *message)
{
	tst_resm(TBROK, "Reason: %s:%s\n", message, strerror(errno));
	return;
}

/*
 * instress
 *
 * Assume that we are always running under stress, so this function will
 * return > 0 value always.
 */
int instress(void)
{
	tst_resm(TINFO, "System resource may be too low, fork() malloc()"
		 " etc are likely to fail.\n");
	return 1;
}
