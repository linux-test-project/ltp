/*
 * @(#)telldir.c	1.5	2003/12/30 Connectathon Testsuite
 */

/*
 * Ensure that telldir and seekdir cooperate.
 * This is done by creating lots of scratch files, walking through with
 * telldir, and using the resulting cookies with seekdir. 
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../tests.h"

static char *usage = "usage: telldir [-d] [-n nfiles]";

/*
 * Information kept for each file.
 */

typedef struct {
	int inuse;			/* is this entry initialized? */
	long cookie;			/* cookie to reach the file */
	int numfiles;			/* number of files after this one */
} file_info_t;

static int debug = 0;

/* 
 * Number of scratch files to create.
 */
static int numfiles = 200;

static char *tdirname = "telldir-test";	/* scratch directory to hold files */ 

static file_info_t *file_info;

static void alloc_file_info ARGS_((int));
static void check_file_info ARGS_((DIR *));
static void cleanup ARGS_((void));
static DIR *make_files ARGS_((void));
static void save_file_info ARGS_((int, long, int));
static void verify ARGS_((DIR *, int, long, int));
static void walk_dir ARGS_((DIR *));

/*ARGSUSED*/
int
main(argc, argv)
	int argc;
	char **argv;
{
	DIR *dp;
	int c;

	while ((c = getopt(argc, argv, "dn:")) != EOF)
		switch (c) {
		case 'd':
			debug = 1;
			break;
		case 'n':
			numfiles = atoi(optarg);
			break;
		case '?':
		default:
			fprintf(stderr, "%s\n", usage);
			exit(1);
			break;
		}

	alloc_file_info(numfiles);
	dp = make_files();
	walk_dir(dp);
	check_file_info(dp);
	cleanup();

	exit(0);
}

/*
 * Create the test directory and scratch files.  Exits if there is a
 * problem.  Returns a pointer to the opened test directory.
 */

static DIR *
make_files()
{
	DIR *dp;
	char filename[MAXPATHLEN];
	int i;

	if (mkdir(tdirname, 0777) < 0) {
		if (errno != EEXIST) {
			fprintf(stderr, "can't create %s: %s\n",
				tdirname, strerror(errno));
			exit(1);
		}
	}

	for (i = 0; i < numfiles; i++) {
		int fd;

		sprintf(filename, "%s/%d", tdirname, i);
		fd = creat(filename, 0666);
		if (fd < 0) {
			fprintf(stderr, "can't create %s: %s\n",
				filename, strerror(errno));
			exit(1);
		}
		close(fd);
	}

	dp = opendir(tdirname);
	if (dp == NULL) {
		fprintf(stderr, "can't open %s: %s\n",
			tdirname, strerror(errno));
		exit(1);
	}

	return (dp);
}

/*
 * Walk through the test directory and make sure we find all the expected
 * files.  For each file, save the telldir information. 
 */

static void
walk_dir(dp)
	DIR *dp;
{
	int files_left;
	struct dirent *entry;
	long cookie;

	files_left = numfiles;
	while (files_left > 0) {
		int filenum;
		char *filename;

		cookie = telldir(dp);
		if (cookie == -1) {
			fprintf(stderr, 
				"warning: cookie = -1, errno=%d (%s)\n",
				errno, strerror(errno));
		}
		errno = 0;
		entry = readdir(dp);
		if (entry == NULL) {
			fprintf(stderr, "error reading %s: %s\n",
				tdirname,
				errno != 0 ? strerror(errno) :
					"premature EOF");
			exit(1);
		}
		filename = entry->d_name;
		if (strcmp(filename, ".") == 0 ||
		    strcmp(filename, "..") == 0)
			continue;
		filenum = atoi(entry->d_name);
		if (filenum < 0 || filenum >= numfiles) {
			fprintf(stderr, "Warning: unexpected filename: %s\n",
				filename);
			continue;
		}
		save_file_info(filenum, cookie, files_left);
		files_left--;
	}
}

/*
 * Check that a seekdir to each saved cookie behaves as expected.
 * 1. we see the expected number of files starting at that cookie.
 * 2. we see the expected first file at that cookie.
 */

static void
check_file_info(dp)
	DIR *dp;
{
	int file;

	for (file = 0; file < numfiles; file++) {
		file_info_t *ip;

		ip = file_info + file;
		if (! ip->inuse) {
			fprintf(stderr, "no information for file %d\n", file);
			exit(1);
		}
		seekdir(dp, ip->cookie);
		verify(dp, file, ip->cookie, ip->numfiles);
	}
}

/*
 * The given directory should be positioned so that file "file" is next,
 * and there should be "files_left" files left in the directory (including
 * "file").  Verify that this is true; complain and exit if it isn't.
 */

static void
verify(dp, file, cookie, files_left)
	DIR *dp;
	int file;
	long cookie;
	int files_left;
{
	struct dirent *entry;
	int first_file = 1;
	int files_found;

	for (files_found = 0; files_found < files_left; files_found++) {
		errno = 0;
		entry = readdir(dp);
		if (entry == NULL) {
			char *errmsg = errno != 0 ?
				strerror(errno) : NULL;

			fprintf(stderr, "entry for %d (cookie %ld):\n",
				file, cookie);
			fprintf(stderr,
			    "expected to find %d entries, only found %d\n",
				files_left, files_found);
			if (errmsg)
				fprintf(stderr, "error: %s\n", errmsg);
			exit(1);
		}
		if (first_file) {
			int file_read = atoi(entry->d_name);

			if (file_read != file) {
				fprintf(stderr,
					"expected file %d at cookie %ld, ",
					file, cookie);
				fprintf(stderr, "found %s\n",
					entry->d_name);
				exit(1);
			}
		}
		first_file = 0;
	}
}

/*
 * Remove the test directory and its contents.
 */

static void
cleanup()
{
	char command[2 * MAXPATHLEN];

	sprintf(command, "rm -rf %s", tdirname);
	system(command);
}

/*
 * Create and initialize the file info array.
 */

static void
alloc_file_info(numfiles)
	int numfiles;
{
	file_info = calloc(numfiles, sizeof (file_info_t));
	if (file_info == NULL) {
		fprintf(stderr, "can't allocate file info array: %s\n",
			strerror(errno));
		exit(1);
	}
	/* inuse fields zeroed by calloc */
}

/*
 * Save the given information in file_info[filenum].
 */

static void
save_file_info(filenum, cookie, files_left)
	int filenum;
	long cookie;
	int files_left;
{
	if (debug) {
		printf("%d 0x%lx %d\n", filenum, cookie, files_left);
	}

	file_info[filenum].inuse = 1;
	file_info[filenum].cookie = cookie;
	file_info[filenum].numfiles = files_left;
}
