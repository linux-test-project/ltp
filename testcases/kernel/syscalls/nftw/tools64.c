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

/* 10/31/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001   Port to Linux   nsharoff@us.ibm.com */

/*
 * NAME
 *	tools64.c - Supporting functions for nftw64.c
 */

#include "nftw64.h"

extern pathdata pathdat[];
extern struct list mnem[];
extern char ebuf[ERR_BUF_SIZ];
extern int npathdats, ngoods, nbads, nmnem;
void fail_exit(void);
extern FILE *temp;
/*
 * Function: void cleanup_function(void)
 *
 * Description:
 *	Cleans the residues$
 *
 * Returns :
 *	Nothing
 */

void cleanup_function(void)
{
	chmod("./tmp/data/d333", (mode_t) S_IRWXU | S_IRWXG | S_IRWXO);
	chmod("./tmp/data/d666", (mode_t) S_IRWXU | S_IRWXG | S_IRWXO);
	chmod("./tmp/data/dirg/dir_right.1", (mode_t) S_IRWXU | S_IRWXG |
	      S_IRWXO);
	system("rm -rf ./tmp");
	wait(NULL);
}

/*
 * Function: void setup_path(void)
 *
 * Description:
 *	Setup the environment to run the nftw64.c
 *
 * Returns :
 *	Nothing
 */

void setup_path(void)
{
	int i, fd;
	temp = stderr;

	if (mkdir("./tmp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
		sprintf(ebuf, "Can't mkdir ./tmp");
		perror(ebuf);
		fprintf(temp, "ERROR: setup_path function failed\n");
		fail_exit();
	}
	for (i = 0; i < npathdats; i++) {
		if (pathdat[i].type == DIR) {
			if (mkdir(pathdat[i].name, pathdat[i].mode) == -1) {
				sprintf(ebuf, "Can't mkdir %s %d",
					pathdat[i].name, i);
				perror(ebuf);
				fprintf(temp,
					"ERROR: setup_path function failed\n");
				cleanup_function();
				fail_exit();
			}
		} else if (pathdat[i].type == SYM) {
			if (symlink(pathdat[i].contents, pathdat[i].name) == -1) {
				sprintf(ebuf, "Can't symlink %s ",
					pathdat[i].name);
				perror(ebuf);
				fprintf(temp,
					"ERROR: setup_path function failed\n");
				cleanup_function();
				fail_exit();
			}
		} else {
			if ((fd = open(pathdat[i].name, O_WRONLY | O_CREAT,
				       pathdat[i].mode)) == -1) {
				sprintf(ebuf, "Can't open %s", pathdat[i].name);
				perror(ebuf);
				fprintf(temp,
					"ERROR: setup_path function failed\n");
				cleanup_function();
				fail_exit();
			}
			if (write(fd, pathdat[i].contents,
				  strlen(pathdat[i].contents)) == -1) {
				perror("Can't write");
				close(fd);
				fprintf(temp,
					"ERROR: setup_path function failed\n");
				cleanup_function();
				fail_exit();
			}
			close(fd);
		}
	}

	if (chmod("./tmp/data/d333", (mode_t) S_IWUSR | S_IXUSR | S_IWGRP |
		  S_IXGRP | S_IWOTH | S_IXOTH) == -1) {
		sprintf(ebuf, "Can't chmod %s ", "./tmp/data/d333");
		perror(ebuf);
		fprintf(temp, "ERROR: setup_path function failed\n");
		cleanup_function();
		fail_exit();
	}
	if (chmod("./tmp/data/d666", (mode_t) S_IRUSR | S_IWUSR | S_IRGRP |
		  S_IWGRP | S_IROTH | S_IWOTH) == -1) {
		sprintf(ebuf, "Can't chmod %s ", "./tmp/data/d666");
		perror(ebuf);
		fprintf(temp, "ERROR: setup_path function failed\n");
		cleanup_function();
		fail_exit();
	}
	if (chmod("./tmp/data/dirg/dir_right.1", (mode_t) S_IWUSR | S_IXUSR |
		  S_IWGRP | S_IXGRP | S_IWOTH | S_IXOTH) == -1) {
		sprintf(ebuf, "Can't chmod %s ", "./tmp/data/dirg/dir_right.1");
		perror(ebuf);
		fprintf(temp, "ERROR: setup_path function failed\n");
		cleanup_function();
		fail_exit();
	}
}

/*
 * Function: int nftw64_fn(const char, const struct stat64, int, struct FTW )
 *
 * Description:
 *      Dummy function for errno tests
 *
 * Returns :
 *	0
 */

int
nftw64_fn(const char *path, const struct stat64 *st, int ival, struct FTW *FTWS)
{
	return (0);
}

/*
 * Function: int callback(char *)
 *
 * Description:
 *      Dummy function for errno tests
 *
 * Returns :
 *	nftw64()
 */

int callback(const char *path)
{
	return nftw64(path, nftw64_fn, 10, FTW_MOUNT);
}

/*
 * Function: char * ftw_mnemonic(int)
 *
 * Description:
 *	Conversion function for printing
 *
 * Returns:
 *	String for printing
 */

char *ftw_mnemonic(int x)
{
	static char s[STRLEN];
	int i;

	for (i = 0; i < nmnem; i++)
		if (x == mnem[i].i)
			return (mnem[i].s);

	sprintf(s, "Unknown value for third argument to fn(): %d\n", x);
	return (s);
}

/*
 * Function: int getbase(char *)
 *
 * Description:
 *	Find basename
 *
 * Returns:
 *	Position of filename in path
 */

int getbase(const char *s)
{
	int i, last = 0;

	for (i = 0; *s != '\0'; s++, i++)
		if (*s == '/')
			last = i;
	return (last ? last + 1 : 0);
}

/*
 * Function: int getlev( char *);
 *
 * Description:
 *	Find level
 *
 * Returns:
 *	Number of /'s in path
 */

int getlev(const char *s)
{
	int i;
	for (i = 0; *s != '\0'; s++)
		if (*s == '/')
			i++;
	return (i);
}

/*
 * Function: void doinfo(char *);
 *
 * Description:
 *	print the file being visited
 *
 * Returns:
 *	Nothing
 */

void do_info(const char *path_name)
{
#ifdef DEBUG
	temp = stderr;
	fprintf(temp, "INFO: Call to fn() at %s\n", path_name);
#endif
}

/** LTP Port **/

/*
 * fail_exit()
 *
 * Exit on failure
 */
void fail_exit(void)
{
	tst_brkm(TFAIL, NULL, "Test failed");
}

/**************/
