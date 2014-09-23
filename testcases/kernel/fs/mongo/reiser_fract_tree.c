/*
 * Copyright 2000 by Hans Reiser, licensing governed by reiserfs/README
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
char tdir[256];
char path[256];
long stats = 0;

void print_usage()
{
	printf("
This program creates files in a tree of random depth and branching. Files vary
in size randomly according to a distribution function which seems to model real
file systems.  This distribution function has a median size of median_file_size
(Median file size is hypothesized to be proportional to the average per file
space wastage. Notice how that implies that with a more efficient file system
file size usage patterns will in the long term move to a lower median file
size), and a maximum size of max_file_size.  Directories vary in size according
to the same distribution function but with separate parameters to control median
and maximum size for the number of files within them, and the number of
subdirectories within them.  This program prunes some empty subdirectories in a
way that causes parents of leaf directories to branch less than
median_dir_branching.

 To avoid having one large file distort the results such that you have
to benchmark many times set max_file_size to not more than
bytes_to_consume/10.  If maximum/median is a small integer, then
randomness is very poor.  This is a bug, Nikita, please find some
clever way to fix it.  If it is 0, then the program crashes....

For isolating performance consequences of design variations on
particular file or directory size ranges, try setting their median size and
max_size to both equal the max size of the file size range you want
to test.

To avoid having one large file distort the results set max_file_size to
not more than bytes_to_consume/10.  Using a distribution function for
the sizes of writes would be a natural next step in developing this program.\n\n");

	printf
	    ("Usage: reiser_fract_tree bytes_to_consume median_file_size max_file_size median_dir_nr_files max_directory_nr_files median_dir_branching max_dir_branching write_buffer_size /testfs_mount_point print_stats_flag\n\n");
}

/* #define DEBUG */

char *write_buffer;		/* buffer from which we write */
int write_buffer_size = 0;	/* gets reset to an argv  */
static int already_whined = 0;	/* keep out of disk space errors from being
				   endless by tracking whether we already
				   printed the message */
long bytes_to_consume = 0;	/* create files until their total number of
				   bytes exceeds this number, but not by more
				   than 1/10th */
long byte_total = 0;		/* bytes created so far */

/* statistics on sizes of files we attempted to create */
int fsz_0_100 = 0;
int fsz_100_1k = 0;
int fsz_1k_10k = 0;
int fsz_10k_100k = 0;
int fsz_100k_1m = 0;
int fsz_1m_10m = 0;
int fsz_10m_larger = 0;

void chngdir(char *name)
{
	int i;

	if (name[0] == '.' && name[1] == '.') {
		for (i = strlen(path); i > 0; i--) {
			if (path[i] == '/') {
				path[i] = 0;
				break;
			}
		}
	} else {
		strcat(path, "/");
		strcat(path, name);
	}
}

/* this is the core statistical distribution function, and it is used for file
   sizes, directory sizes, etc. */
int determine_size(double median_size,
		   double max_size /* The maximal value of size */ )
{
	/* when x is half of its random range (max_size/median_size), result is
	   median_size */
	int nr_random, granularity_reducer;
	double size, double_nr_random;

	/* it is a feature for us that this repeats identically every time it is run,
	   as otherwise meaningless variances would affect our results and require us
	   to use a higher number of benchmarks to achieve low noise results.  */
	nr_random = rand();
	median_size++;		/* avoids divide by zero errors */

	/* this code does poorly when max_size is not a lot more than median size,
	   and that needs fixing */

	/* THE NEXT 2 LINES ARE THE HEART OF THE PROGRAM */

	/* keep x below the value that when multiplied by median size on the next
	   line will equal max_size */
	/* the granularity_reducer is to handle the case where max_size is near
	   median_size, since '%' can only take ints, we need this complicated what
	   of handling that for small values of max_size/median_size by making
	   large ints out of small ints temporarily.  */
	if (max_size / median_size < 1024)
		granularity_reducer = 1024 * 1024;
	else
		granularity_reducer = 1;
	nr_random =
	    nr_random %
	    ((int)
	     (granularity_reducer *
	      (((double)max_size) / ((double)median_size))));
	double_nr_random = ((double)nr_random) / (granularity_reducer);
	size =
	    median_size * (1 /
			   (1 -
			    (double_nr_random) / (((double)max_size) /
						  ((double)median_size))) - 1);
	return ((int)size);
}

/* generate a unique filename */
void get_name_by_number(long this_files_number, char *str)
{
	sprintf(str, "%lu", this_files_number);
}

/* make a file of a specified size */
void make_file(int size)
{
	char string[128] = { 0 };
	char *str = string;
	char fname[256];
	int fd = 0;
	int error;
	static long this_files_number = 1;

	/* collect statistics about the size of files created, or more precisely, the
	   size of files that we will attempt to create. */
	if (size <= 100)
		fsz_0_100++;
	else if (size <= 1000)
		fsz_100_1k++;
	else if (size <= 10 * 1000)
		fsz_1k_10k++;
	else if (size <= 100 * 1000)
		fsz_10k_100k++;
	else if (size <= 1000 * 1000)
		fsz_100k_1m++;
	else if (size <= 10 * 1000 * 1000)
		fsz_1m_10m++;
	else
		fsz_10m_larger++;

	/* construct a name for the file */
	get_name_by_number(this_files_number++, str);
	strcpy(fname, path);
	strcat(fname, "/");
	strcat(fname, str);

	/* open the file, and deal with the various errors that can occur */

	if ((fd = open(fname, O_CREAT | O_EXCL | O_RDWR, 0777)) == -1) {
		if (errno == ENOSPC) {
			if (!already_whined) {
				printf
				    ("reiser-2021A: out of disk (or inodes) space, will keep trying\n");
				already_whined = 1;	/* we continue other file creation in out of
							   space conditions */
			}
			return;
		}
		/*  it is sometimes useful to be able to run this program more than once
		   inside the same directory, and that means skipping over filenames that
		   already exist.  Thus we ignore EEXIST, and pay attention to all
		   else. */
		if (errno == EEXIST) {	/* just skip existing file */
			return;
		}
		perror("open");
		exit(errno);
	}
	/* write to the file until it is the right size, handling the various error
	   conditions appropriately */

	while (size > 0) {
		size -= (error =
			 write(fd, write_buffer,
			       (size <
				write_buffer_size -
				1) ? size : (write_buffer_size - 1)));
		if (error == -1) {
			if (errno == ENOSPC) {
				if (!already_whined) {
					printf
					    ("reiser-2022: out of disk space, will keep trying\n");
					already_whined = 1;
				}
				close(fd);
				return;
			}
			perror("write() failed");
			exit(errno);
		}
	}

	/* close the file */
	if (close(fd)) {
		perror("close() failed");
		exit(errno);
	}
}

/* print the statistics on how many files were created of what size */

void print_stats()
{
	if (!stats)
		return;

	printf("\n");
	printf("File stats: Units are decimal (1k = 1000)\n");
	printf("files 0-100    : %i\n", fsz_0_100);
	printf("files 100-1K   : %i\n", fsz_100_1k);
	printf("files 1K-10K   : %i\n", fsz_1k_10k);
	printf("files 10K-100K : %i\n", fsz_10k_100k);
	printf("files 100K-1M  : %i\n", fsz_100k_1m);
	printf("files 1M-10M    : %i\n", fsz_1m_10m);
	printf("files 10M-larger    : %i\n", fsz_10m_larger);
	printf("total bytes written    : %lu\n", byte_total);

}

/* predict the number of files that will be created before max_bytes total
   length of files is reached */
long determine_nr_of_files(int median_file_size, double max_file_size,
			   long bytes_to_consume)
{
	long nr_of_files = 0, byte_total = 0;

	/* the next line is not necessary as 1 is the default, it is just cautious
	   coding */
	srand(1);
	while (byte_total < bytes_to_consume) {
		byte_total += determine_size(median_file_size, max_file_size);
		nr_of_files++;
	}
	/* reset the random number generator so that when we determine_size() of the
	   files later they will be created with the same "random" sequence used in
	   this calculation */
	srand(1);
#ifdef DEBUG
	printf("number of files is %d\n", (int)nr_of_files);
#endif /* DEBUG */
	fflush(NULL);
	return nr_of_files;
}

/* fill the current working directory with nr_files_this_directory number of
   files*/

void fill_this_directory(long nr_files_this_directory, long median_file_size,
			 long maximum_size)
{
	long size;

#ifdef DEBUG
	printf("filling with %lu files, ", nr_files_this_directory);
#endif
	while (nr_files_this_directory--) {
		size = determine_size(median_file_size, maximum_size);
		byte_total += size;
		make_file(size);
	}
}

/* this will unfortunately handle out of disk space by forever trying */
/* What we should do in out of space situaltion ? I think we must skip this
   directory and continue files/dirs creation process. Error value (!= 0)
   indicates that we can't go to this directory. -zam */
int make_directory(char *dirname)
{
	static long this_directory_number = 0;

	strcpy(tdir, path);
	strcat(tdir, "/");
	strcat(tdir, dirname);

	if (mkdir(tdir, 0755) == -1) {
		if (errno == ENOSPC) {
			if (!already_whined) {
				printf("reiser-2021: out of disk space, ");
				already_whined = 1;
			}
			return errno;
		}
		/*  it is sometimes useful to be able to run this program more than once
		   inside the same directory, and that means skipping over filenames that
		   already exist.  Thus we ignore EEXIST, and pay attention to all else. */
		if (errno != EEXIST) {
			perror("mkdir");
			exit(errno);
		}
	}
	sprintf(dirname, "d%lu", this_directory_number++);
	strcpy(tdir, path);
	strcat(tdir, "/");
	strcat(tdir, dirname);

	return 0;
}

/* assumes we are already chdir'd into a directory that the subtree is rooted
   at.  Fills the directory with files and subdirectories, cd's into those
   subdirectories, and recurses upon itself */

void do_subtree(
		       /* the start and end of the portion of the directory sizes
		          array which corresponds to the sizes of the directories
		          composing this subtree */
		       /* sizes_end minus sizes_start is equal to the number of
		          directories in this subtree */
		       long *sizes_start, long *sizes_end,
		       long median_file_size, long maximum_file_size,
		       long median_dir_branching, long max_dir_branching)
{
	long *p;
	long *sub_start;
	long *sub_end;
	int index_subdirectory_to_add_directory_to;
	long *dirs_in_subtrees;
	char *subtree_name;
	long *sizes_index = sizes_start;
	char subtree_name_array[128];
	long this_directory_branching;
	static long this_directorys_number;

	subtree_name = subtree_name_array;
	/* fill this directory with its number of files */
	fill_this_directory(*sizes_index, median_file_size, maximum_file_size);
	sizes_index++;
	/* ok, now randomly assign directories (and their number of files) among the
	   subdirectories that will be created if at least one directory is assigned
	   to it */

	/* this will cause the random number sequence to not match the one used in
	   determine_nr_files() I need to accumulate my values in an array
	   beforehand. I'll code that later.  */
	/* worry about whether 0 or 1 is a problem value */
	this_directory_branching =
	    determine_size(median_dir_branching, max_dir_branching) + 1;

	/* create an array holding the number of directories assigned to each
	   potential subdirectory */
	dirs_in_subtrees = calloc(this_directory_branching, sizeof(long));
	while (sizes_index <= sizes_end) {
		index_subdirectory_to_add_directory_to =
		    (rand() % this_directory_branching);
		(*
		 (dirs_in_subtrees + index_subdirectory_to_add_directory_to))++;
		sizes_index++;
	}
	/* the +1 is for the fill_directory() we did above */
	sizes_index = sizes_start + 1;

	/* go through each potential subdirectory, and if at least one directory has
	   been assigned to it, create it and recurse */
	for (p = dirs_in_subtrees;
	     p < (dirs_in_subtrees + this_directory_branching); p++) {
		if (*p) {
			int nocd;
			sprintf(subtree_name, "d%lu", this_directorys_number++);
			nocd = make_directory(subtree_name);
			/* if make_dir.. may fails (in out of space situation), we continue
			   creation process in same dir */
			if (!nocd)
				chngdir(subtree_name);
			sub_start = sizes_index;
			/* the minus one is because *p is the number of elements and arrays start at 0 */
			sub_end = (sizes_index + (*p - 1));

#ifdef DEBUG
			/* comment this back in if the array logic has you going cross-eyed */
			/*      printf ("sizes_start is %p, sizes_index is %p, sizes_index+p is %p, sizes_end is %p\n", sizes_start, sub_start, sub_end, sizes_end); */
#endif
			do_subtree(sub_start, sub_end, median_file_size,
				   maximum_file_size, median_dir_branching,
				   max_dir_branching);
			if (!nocd)
				chngdir("..");
		}
		sizes_index += *p;
	}
}

/* We have already determined that nr_files can fit in bytes_to_consume space.
   Fill the sizes array with the number of files to be in each directory, and
   then call do_subtree to fill the tree with files and directories.  */

void make_fractal_tree(long median_file_size, long maximum_file_size,
		       long median_dir_nr_files, long max_dir_nr_files,
		       long median_dir_branching, long max_dir_branching,
		       long nr_files)
{
	long *sizes_start;
	long *sizes_end;
	long *sizes_index;
	long remaining_files = nr_files;

	/* collect together array of directory sizes for whole filesystem.  This
	   cannot easily be done recursively without distorting the directory sizes
	   and making deeper directories smaller.  Send me the code if you
	   disagree.:-) */
	/* we almost certainly don't need this much space, but so what.... */
	sizes_index = sizes_start = malloc(nr_files * sizeof(long));
	for (; remaining_files > 0;) {
		*sizes_index =
		    determine_size(median_dir_nr_files, max_dir_nr_files);
		// we alloc space for nr_files, so we should avoid
		// number of files in directory = 0 -grev.
		if (*sizes_index == 0)
			*sizes_index = 1;
		*sizes_index =
		    (*sizes_index <
		     remaining_files) ? *sizes_index : remaining_files;

#ifdef DEBUG
		printf("*sizes_index == %lu, ", *sizes_index);
#endif
		remaining_files -= *sizes_index;
		sizes_index++;
	}
	/* don't decrement below sizes_start if nr_files is 0 */
	sizes_end = (sizes_index-- > sizes_start) ? sizes_index : sizes_start;

	sizes_index = sizes_start;
	srand(1);
	do_subtree(sizes_start, sizes_end, median_file_size, maximum_file_size,
		   median_dir_branching, max_dir_branching);

}

int main(int argc, char *argv[])
{
	/* initialized from argv[] */
	long median_file_size,
	    median_dir_branching,
	    median_dir_nr_files,
	    max_dir_nr_files, max_dir_branching, max_file_size;
	long nr_of_files = 0;	/* files to be created */

	if (argc != 11) {
		print_usage();
		exit(1);
	}

	write_buffer_size = atoi(argv[8]);
	write_buffer = malloc(write_buffer_size);
	memset(write_buffer, 'a', write_buffer_size);

	/* the number of bytes that we desire this tree to consume.  It will actually
	   consume more, because the last file will overshoot by a random amount, and
	   because the directories and metadata will consume space.  */
	bytes_to_consume = atol(argv[1]);
	max_file_size = atol(argv[3]);
	median_file_size = atol(argv[2]);
	/* Figure out how many random files will fit into bytes_to_consume bytes. We
	   depend on resetting rand() to get the same result later. */
	nr_of_files =
	    determine_nr_of_files(median_file_size, max_file_size,
				  bytes_to_consume);

	strcpy(path, argv[9]);
	mkdir(path, 0755);
	stats = atol(argv[10]);
	median_dir_branching = atol(argv[6]);
	max_dir_branching = atol(argv[7]);
	median_dir_nr_files = atol(argv[4]);
	max_dir_nr_files = atol(argv[5]);
	make_fractal_tree(median_file_size, max_file_size, median_dir_nr_files,
			  max_dir_nr_files, median_dir_branching,
			  max_dir_branching, nr_of_files);
	print_stats();
	if (stats)
		printf("\nreiser_fract_tree finished\n");

	return 0;
}
