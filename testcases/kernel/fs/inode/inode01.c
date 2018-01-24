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

/* 10/31/2002	Port to LTP	robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

			   /* inode1.c */
/*======================================================================
	=================== TESTPLAN SEGMENT ===================
CALLS:	mkdir, stat, open

	run using TERM mode

>KEYS:  < file system management I/O
>WHAT:  < Do the system's file system management and I/O functions work
	< correctly?
>HOW:   < Construct a directory tree, create files in it, and verify
	< that this was done as expected.
>BUGS:  <
======================================================================*/
/* modified by dale 25-Jul-84 */

/************************************************/
#define PATH_STRING_LENGTH  100
#define NAME_LENGTH  8
#define MAX_PATH_STRING_LENGTH  (PATH_STRING_LENGTH - NAME_LENGTH)
#define MAX_DEPTH   3
#define MAX_BREADTH 3
#define FILE_LENGTH 100
#define DIRECTORY_MODE  00777
#define FILE_MODE       00777

/* #define PRINT 		 define to get list while running */

#define TRUE  1
#define FALSE 0
#define READ  0
#define WRITE 1

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

/** LTP Port **/
#include "test.h"

void blexit(void);
void blenter(void);
void setup(void);
void fail_exit(void);
void anyfail(void);
void ok_exit(void);

#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
int block_number;
FILE *temp;

char *TCID = "inode01";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
/**************/

#ifdef LINUX
#include <string.h>
#endif

char name[NAME_LENGTH + 1];
char path_string[PATH_STRING_LENGTH + 1];
char read_string[PATH_STRING_LENGTH + 1];
char write_string[PATH_STRING_LENGTH + 1];
char rm_string[200];

FILE *list_stream = NULL;
int file_id;
int list_id;

int increment_name(), get_next_name(), mode(), escrivez();

int main(void)
{
	char root[16];		//as pids can get much longer
	int gen_ret_val, ch_ret_val, level;
	int ret_val;
	int generate(), check();
	char path_list_string[PATH_STRING_LENGTH + 1];
	int status;
	int len;
	int term();
	int snp_ret;

	strcpy(path_string, "inode");
	sprintf(root, "A%d", getpid());
	strcat(path_string, root);

	strcpy(rm_string, "rm -rf ");
	strcat(rm_string, path_string);

	setup();

	if (signal(SIGTERM, (void (*)())term) == SIG_ERR) {
		fprintf(temp, "\tSIGTERM signal set failed!, errno=%d\n",
			errno);
		fail_exit();
	}

	blenter();

	/********************************/
	/*                              */
	/*  make the root directory for */
	/*  the tree                    */
	/*                              */
	/********************************/

	ret_val = mkdir(path_string, DIRECTORY_MODE);

	if (ret_val == -1) {
		perror("mkdir error");
		fprintf(temp, "\tcreating directory '%s'\n", path_string);
		fprintf(temp, "\t\n%s Impossible to create directory %s\n",
			root, path_string);
		fail_exit();
	}
#ifdef PRINT
	printf("\n%s\n", path_string);
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
		fail_exit();
	}
	list_id = creat(path_list_string, FILE_MODE);
	if (list_id == -1) {
		fprintf(temp,
			"\t\n%s The path_list file cannot be created, errno=%d \n",
			root, errno);
		fail_exit();
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

#ifdef PRINT
	printf("\n\t%s\n\n", "GENERATING:");
#endif

	gen_ret_val = generate(path_string, level);

	if (gen_ret_val) {
		fprintf(temp,
			"Failure occured in generate routine, return value %d\n",
			gen_ret_val);
		local_flag = FAILED;
	}

	blexit();
	blenter();

	close(list_id);
	list_id = open(path_list_string, READ);
	if (list_id == -1) {
		fprintf(temp,
			"\t\n%s The path_list file cannot be opened for reading, errno=%d\n",
			root, errno);
		fail_exit();
	}
	list_stream = fdopen(list_id, "r");

	/****************************************/
	/*                                      */
	/*   check the directory-file tree      */
	/*      for correctness                 */
	/*                                      */
	/****************************************/

#ifdef PRINT
	printf("\n\t%s\n\n", "CHECKING:");
#endif

	ch_ret_val = check();

	if (ch_ret_val) {
		fprintf(temp,
			"Failure occured in check routine, return value %d\n",
			ch_ret_val);
		local_flag = FAILED;
	}

	status = fclose(list_stream);
	if (status != 0) {
		fprintf(temp,
			"Failed to close list_stream: ret=%d errno=%d (%s)\n",
			status, errno, strerror(errno));
		local_flag = FAILED;
	}

	blexit();

	/*
	 * Now fork and exec a system call to remove the directory.
	 */

#ifdef DEBUG
	fprintf(temp, "\nClean up:\trm string = %s\n", rm_string);
#endif
	fflush(stdout);
	fflush(temp);

	status = system(rm_string);

	if (status) {
		fprintf(temp, "Caution-``%s'' may have failed\n", rm_string);
		fprintf(temp, "rm command exit status = %d\n", status);
	}

	/****************************************/
	/*                                      */
	/*         .....and exit main           */
	/*                                      */
	/****************************************/

	anyfail();
	/***** NOT REACHED ******/
	tst_exit();
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
/*  string[]      	   */
/*  the directory path     */
/*  string below which a   */
/*  tree is generated      */
/*                         */
/***************************/

/***************************/
/* level                   */
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

		fprintf(temp, "\tMaximum path_name length reached.\n");
		return (-1);
	} else if (level < MAX_DEPTH) {
		for (i = 0; i <= MAX_BREADTH; i++) {
			get_next_name();
			snp_ret = snprintf(new_string, sizeof(new_string),
				"%s/%s", string, name);
			if (snp_ret < 0 || snp_ret >= sizeof(new_string)) {
				tst_resm(TBROK, "snprintf(new_string,..) "
					"returned %d", snp_ret);
				fail_exit();
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
						"\tImpossible to create file %s, errno=%d\n",
						new_string, errno);
					return (-2);
				}
#ifdef PRINT
				printf("%d  %s F\n", level, new_string);
#endif

				/****************************************/
				/*                                      */
				/*            write to it               */
				/*                                      */
				/****************************************/

				len = strlen(new_string);
				for (j = 1; j <= FILE_LENGTH; j++) {
					ret_len =
					    write(file_id, new_string, len);
					if (ret_len != len) {
						fprintf(temp,
							"\tUnsuccessful write to file %s, expected return of %d, got %d, errno=%d\n",
							new_string, len,
							ret_len, errno);
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
				/****************************************/

				ret_val = mkdir(new_string, DIRECTORY_MODE);

				if (ret_val != 0) {
					fprintf(temp,
						"\tImpossible to create directory %s, errno=%d\n",
						new_string, errno);
					return (-5);
				}
#ifdef PRINT
				printf("%d  %s D\n", level, new_string);
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

#ifdef PRINT
			printf("\nEnd of path_list file reached \n");
#endif

			return 0;
		}
#ifdef PRINT
		printf("%s\n", path_string);
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
					"\tImpossible to open file %s, errno=%d\n",
					path_string, errno);
				return (-1);
			}

			else {
				/********************************/
				/*                              */
				/*    check its contents        */
				/*                              */
				/********************************/

				len = strlen(path_string);
				for (j = 1; j <= FILE_LENGTH; j++) {
					ret_len =
					    read(file_id, read_string, len);
					if (len != ret_len) {
						fprintf(temp,
							"\tFile read error for file %s, expected return of %d, got %d, errno=%d\n",
							path_string, len,
							ret_len, errno);
						return (-3);
					}
					read_string[len] = '\0';
					val = strcmp(read_string, path_string);
					if (val != 0) {
						fprintf(temp,
							"\tContents of file %s are different than expected: %s\n",
							path_string,
							read_string);
						return (-4);
					}
				}
				close(file_id);
			}	/* else for */
			if (ret_len <= 0) {
				fprintf(temp, "\tImpossible to read file %s\n",
					path_string);
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
					"\tPreviously created directory path %s was not open\n",
					path_string);
				return (-4);
			}
			if ((040000 & path_mode) != 040000) {
				fprintf(temp,
					"\tPath %s was not recognized to be a directory\n",
					path_string);
				fprintf(temp, "\tIts mode is %o\n", path_mode);
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
				"\tERROR: There are no more available names\n");
			fail_exit();
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
			"\tA string of deviant length %d written to path_list, errno=%d\n",
			ret_len, errno);
		fail_exit();
	}
	return 0;
}

int term(void)
{
	int status;

	fprintf(temp, "\tterm - got SIGTERM, cleaning up.\n");

	if (list_stream != NULL)
		fclose(list_stream);
	close(list_id);
	close(file_id);

	status = system(rm_string);
	if (status) {
		fprintf(temp, "Caution - ``%s'' may have failed.\n", rm_string);
		fprintf(temp, "rm command exit status = %d\n", status);
	}

	ok_exit();
	/***NOT REACHED***/
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
 * Function: blexit()
 *
 * Description: This function will exit a block, a block may be a lo
gical unit
 *              of a test. It will report the status if the test ie
fail or
 *              pass.
 */
void blexit(void)
{
	(local_flag == PASSED) ? tst_resm(TPASS, "Test block %d", block_number)
	    : tst_resm(TFAIL, "Test block %d", block_number);
	block_number++;
	return;
}

/*
 * Function: blenter()
 *
 * Description: Print message on entering a new block
 */
void blenter(void)
{
	local_flag = PASSED;
	return;
}

/*
 * fail_exit()
 *
 * Exit on failure
 */
void fail_exit(void)
{
	tst_brkm(TFAIL, tst_rmdir, "Test failed");
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
