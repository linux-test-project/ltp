/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */
/*
 * This program will grow a list of files.
 * Each file will grow by grow_incr before the same
 * file grows twice.  Each file is open and closed before next file is opened.
 *
 * To just verify file contents: growfiles -g 0 -c 1 filename
 *
 * See help and prt_examples functions below.
 *
 * Basic code layout
 *  process cmdline
 *  print debug message about options used
 *  setup signal handlers
 *  return control to user (if wanted - default action)
 *  fork number of desired childern (if wanted)
 *  re-exec self (if wanted)
 *  Determine number of files
 *  malloc space or i/o buffer
 *  Loop until stop is set
 *    Determine if hit iteration, time, max errors or num bytes reached
 *    Loop through each file
 *	open file
 *	fstat file - to determine if file if a fifo
 *	prealloc file space (if wanted)
 *      growfile
 *	check last write
 *	check whole file
 *	shrink file
 *	close file
 *	delay (if wanted)
 *    End loop
 *  End loop
 *  remove all files (if wanted)
 *
 * Author: Richard Logan
 *
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <sys/file.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/statfs.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include "dataascii.h"
#include "random_range.h"
#include "databin.h"
#include "open_flags.h"
#include "forker.h"
#include "file_lock.h"

#ifdef CRAY
#include <sys/panic.h>
#include <sys/category.h>
#endif

#include "test.h"

int set_sig(void);
void sig_handler(int sig);
static void notify_others(void);
int handle_error(void);
int cleanup(void);
void usage(void);
void help(void);
void prt_examples(FILE * stream);
int growfile(int fd, char *file, int grow_incr, char *buf,
	     unsigned long *curr_size_ptr);
int shrinkfile(int fd, char *filename, int trunc_incr,
	       int trunc_inter, int just_trunc);
int check_write(int fd, int cf_inter, char *filename, int mode);
int check_file(int fd, int cf_inter, char *filename, int no_file_check);
int file_size(int fd);
int lkfile(int fd, int operation, int lklevel);

#ifndef linux
int pre_alloc(int fd, long size);
#endif /* !linux */

extern int datapidgen(int, char *, int, int);
extern int datapidchk(int, char *, int, int, char **);

/* LTP status reporting */
char *TCID = "growfiles";	/* Default test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

/* To avoid extensive modifications to the code, use this bodge */
#define exit(x) myexit(x)
void myexit(int x)
{
	if (x)
		tst_resm(TFAIL, "Test failed");
	else
		tst_resm(TPASS, "Test passed");
	tst_exit();
}

#define NEWIO	1		/* Use the tlibio.c functions */

#ifndef NEWIO
#define NEWIO	0		/* specifies to use original iowrite.c */
			/* functions instead of tlibio.c functions */
			/* Once it is proven tlibio.c functions work properly, */
			/* only tlibio.c functions will be used */
#else
#include "tlibio.h"
#endif

#ifndef PATH_MAX
#define PATH_MAX	1023
#endif

#define DEF_DIR		"."
#define DEF_FILE	"gf"

char *Progname;
int Debug = 0;

int Pid = 0;

int io_type = 0;		/* I/O type -sync */

#ifdef O_LARGEFILE
int open_flags = O_RDWR | O_CREAT | O_LARGEFILE;	/* open flags */
#else
#warning O_LARGEFILE is not defined!
int open_flags = O_RDWR | O_CREAT;	/* open flags */
#endif

#define MAX_FC_READ	196608	/* 4096 * 48 - 48 blocks */

#define PATTERN_ASCII	1	/* repeating alphabet letter pattern */
				/* allows multiple writers and to be checked */
#define PATTERN_PID	2	/* <pid><words byte offset><pid> */
				/* Assumes 64 bit word. Only allows single */
				/* process to write and check */
/*
 *	1234567890123456789012345678901234567890123456789012345678901234
 *	________________________________________________________________
 *	<    pid       >< offset in file of this word  ><    pid       >
 */

#define PATTERN_OFFSET	3	/* Like PATTERN_PID but has a fixed number */
				/* (STATIC_NUM) instead of pid. */
				/* Allows multiple processes to write/read */
#define PATTERN_ALT	4	/* alternating bit pattern (i.e. 0x5555555...) */
#define PATTERN_CHKER	5	/* checkerboard pattern (i.e. 0xff00ff00ff00...) */
#define PATTERN_CNTING  6	/* counting pattern (i.e. 0 - 07, 0 - 07, ...) */
#define PATTERN_ONES	7	/* all bits set (i.e. 0xffffffffffffff...) */
#define PATTERN_ZEROS	8	/* all bits cleared (i.e. 0x000000000...) */
#define PATTERN_RANDOM	9	/* random integers - can not be checked */
#define STATIC_NUM	221849	/* used instead of pid when PATTERN_OFFSET */

#define MODE_RAND_SIZE	1	/* random write and trunc */
#define MODE_RAND_LSEEK	2	/* random lseek before write */
#define MODE_GROW_BY_LSEEK 4	/* lseek beyond end of file then write a byte */
#define RANDOM_OPEN	999876	/* if Open_flags set to this value, open flags */
				/* will be randomly choosen from Open_flags[] */
#define MODE_FIFO	S_IFIFO	/* defined in stat.h  0010000 */

int num_files = 0;		/* num_auto_files + cmd line files */
char *filenames;		/* pointer to space containing filenames */
int remove_files = 0;		/* if set, cleanup default is not to cleanup */
int bytes_consumed = 0;		/* total bytes consumed, all files */
int bytes_to_consume = 0;	/* non-zero if -B was specified, total bytes */
int Maxerrs = 100;		/* Max number errors before forced exit */
int Errors = 0;			/* number of encountered errors */
int Upanic_on_error = 0;	/* call upanic if error and this variable set */

/* The *_size variables are only used when random iosize option (-r) is used */
int max_size = 5000;
int min_size = 1;		/* also set in option parsing */
int mult_size = 1;		/* when random iosz, iosz must be mult of mult_size */
/* the *_lseek variables are only used when radon lseek option (-R) is used */
int min_lseek = 0;		/* also set in option parsing */
int max_lseek = -1;		/* -1 means size of file */
#ifdef CRAY
int Pattern = PATTERN_OFFSET;	/* This pattern is 64 bit word based */
#else
int Pattern = PATTERN_ASCII;
#endif
int Seed = -1;			/* random number seed, < 0 == uninitialized  */
int Nseeds = 0;			/* Number of seed specified by the user */
int *Seeds;			/* malloc'ed arrary of ints holding user spec seeds */

int using_random = 0;		/* flag indicating randomization is being used */
float delaysecs = 0.0;		/* delay between iterations (in seconds) */
int delaytime;			/* delay between iterations in clocks/uses */
int lockfile = 0;		/* if set, do file locking */
				/* 1 = do file locking around write, trunc */
				/* and reads. */
				/* 2 = write lock around all file operations */

off_t Woffset = 0;		/* offset before last write */
int Grow_incr = 4096;		/* sz of last write */
int Mode = 0;			/* bitmask of write/trunc mode */
				/* also knows if dealing with fifo */
char *Buffer = NULL;		/* buffer used by write and write check */
int Alignment = 0;		/* if non word multiple, io will not be word aligned */
int Opid = 0;			/* original pid */

int Sync_with_others = 0;	/* Flag indicating to stop other if we stop before DONE */
int Iter_cnt = 0;		/* contains current iteration count value */
char TagName[40];		/* name of this growfiles (see Monster)     */

struct fileinfo_t {
	char *filename;
	int fd;
	int openflags;
	int mode;
} Fileinfo;

/*
 * Define open flags that will be used when '-o random' option is used.
 * Note: If there is more than one growfiles doing its thing to the same
 * file, O_TRUNC will cause data mismatches.  How you ask?
 * timing of events, example:
 *   Process one		Process two
 *   ---------------		-------------
 *   get write lock
 *   fstat file
 *   lseek
 *   generate pattern
 *				open with O_TRUNC
 *   write with wrong pattern
 *	because offset is wrong
 *
 *  The second process truncated the file after the pattern was
 *  determined, thus the pattern is wrong for the file location.
 *
 * There can also be a timing problem with open flag O_APPEND if
 * file locks are not being used (-l option).  Things could happen
 * between the fstat and the write. Thus, writing the wrong pattern.
 * If all processes observe the file locks, O_APPEND should be ok
 * to use.
 */
int Open_flags[] = {
#ifdef CRAY
	O_RDWR | O_CREAT,
	O_RDWR | O_CREAT | O_RAW,
	O_RDWR | O_CREAT | O_BIG,
	O_RDWR | O_CREAT | O_APPEND,
	O_RDWR | O_CREAT | O_NDELAY,
	O_RDWR | O_CREAT | O_PLACE,
	O_RDWR | O_CREAT | O_SYNC,
	O_RDWR | O_CREAT | O_RAW | O_SYNC,
	O_RDWR | O_CREAT | O_NDELAY | O_SYNC,
	O_RDWR | O_CREAT | O_NDELAY | O_SYNC | O_BIG,
	O_RDWR | O_CREAT | O_RAW,
	O_RDWR | O_CREAT | O_RAW | O_APPEND,
	O_RDWR | O_CREAT | O_RAW | O_BIG,
	O_RDWR | O_CREAT | O_RAW | O_APPEND | O_BIG,
/***
 * O_WELLFORMED makes -o random require well formed i/o
 ***/
#if ALLOW_O_WELLFORMED
#if O_PARALLEL
	O_RDWR | O_CREAT | O_PARALLEL | O_WELLFORMED | O_RAW,
	O_RDWR | O_CREAT | O_PARALLEL | O_WELLFORMED | O_RAW | O_TRUNC,
#endif /* O_PARALLEL */
#endif

#else /* CRAY */
	O_RDWR | O_CREAT,
	O_RDWR | O_CREAT | O_APPEND,
	O_RDWR | O_CREAT | O_NDELAY,
	O_RDWR | O_CREAT | O_SYNC,
	O_RDWR | O_CREAT | O_SYNC | O_NDELAY,
	O_RDWR | O_CREAT | O_APPEND | O_NDELAY,

#endif /* CRAY */
};

#define REXEC_INIT	0	/* don't do re-exec of childern */
#define REXEC_DOIT	1	/* Do re-exec of childern */
#define REXEC_DONE	2	/* We've already been re-exec'ed */

#ifndef BSIZE
#ifdef CRAY
#define BSIZE	1024
#else
#define BSIZE	512
#endif /* CRAY */
#endif /* BSIZE */

#define USECS_PER_SEC	1000000	/* microseconds per second */

/*
 * Define macros used when dealing with file locks.
 */
#define LKLVL0		1	/* file lock around write/read/trunc */
#define LKLVL1		2	/* file lock after open to before close */

/*
 * Define special max lseek values
 */
#define LSK_EOF       	    -1	/* set fptr up to EOF */
#define LSK_EOFPLUSGROW	    -2	/* set fptr up to EOF + grow - leave whole */
#define LSK_EOFMINUSGROW    -3	/* set fptr up to EOF-grow - no grow */

/***********************************************************************
 * MAIN
 ***********************************************************************/
int main(int argc, char **argv)
{
	extern char *optarg;	/* used by getopt */
	extern int optind;

	int ind;
	int first_file_ind = 0;
	int num_auto_files = 0;	/* files created by tool */
	int seq_auto_files = 0;	/* auto files created by tool created by tool */
	char *auto_dir = DEF_DIR;
	char *auto_file = DEF_FILE;
	int grow_incr = 4096;
	int trunc_incr = 4096;
	int trunc_inter = 0;	/* 0 means none, */
	int unlink_inter = 0;	/* 0 means none, 1 means always unlink */
	int unlink_inter_ran = -1;	/* -1 -use unlink_inter, otherwise randomly choose */
	/* between unlink_inter and unlink_inter_ran */
	int file_check_inter = 0;	/* 0 means never, 1 means always */
	int write_check_inter = 1;	/* 0 means never, 1 means always */
	int iterations = 1;	/* number of increments to be added */
	int no_file_check = 0;	/* if set, no whole file checking will be done */
	int num;
	int fd;			/* file descriptor */
	int stop = 0;		/* loop stopper if set */

	unsigned long curr_size = 0;	/* BUG:14136 (keep track of file size) */
	unsigned long fs_limit = 2147483647; /* BUG:14136 (filesystem size limit is 2G by default) */
	struct statfs fsbuf;

	int tmp;
	char chr;
	int ret;
	int pre_alloc_space = 0;
#ifndef linux
	long total_grow_value;	/* used in pre-allocations */
#endif
	int backgrnd = 1;	/* return control to user */
	struct stat statbuf;
	int time_iterval = -1;
	time_t start_time = 0;
	char reason[128];	/* reason for loop termination */
	int num_procs = 1;
	int forker_mode = 0;
	int reexec = REXEC_INIT;	/* reexec info */
	char *exec_path = NULL;

/*char *strrchr();*/

	char *filename;		/* name of file specified by user */
	char *cptr;		/* temp char pointer */
	extern int Forker_npids;	/* num of forked pid, defined in forker.c */
	struct timeval tv1;

	if (argv[0][0] == '-')
		reexec = REXEC_DONE;
	/*
	 * Determine name of file used to invoke this program
	 */
	if ((Progname = strrchr(argv[0], '/')) != NULL)
		Progname++;
	else
		Progname = argv[0];

	TagName[0] = '\0';

	/*
	 * Process options
	 */
	while ((ind = getopt(argc, argv,
			     "hB:C:c:bd:D:e:Ef:g:H:I:i:lL:n:N:O:o:pP:q:wt:r:R:s:S:T:uU:W:xy"))
	       != EOF) {
		switch (ind) {

		case 'h':
			help();
			tst_exit();

		case 'B':
			switch (sscanf(optarg, "%i%c", &bytes_to_consume, &chr)) {
			case 1:	/* noop */
				break;

			case 2:
				if (chr == 'b') {
					bytes_to_consume *= BSIZE;
				} else {
					fprintf(stderr,
						"%s%s:  --B option arg invalid\n",
						Progname, TagName);
					usage();
					exit(1);
				}
				break;

			default:
				fprintf(stderr,
					"%s%s: --B option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
				break;
			}

			break;

		case 'E':
			prt_examples(stdout);
			exit(0);

		case 'b':	/* batch */
			backgrnd = 0;
			break;

		case 'C':
			if (sscanf(optarg, "%i", &write_check_inter) != 1) {
				fprintf(stderr,
					"%s%s: --c option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 'c':
			if (sscanf(optarg, "%i", &file_check_inter) != 1) {
				fprintf(stderr,
					"%s%s: --c option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 'd':
			auto_dir = optarg;
#ifdef CRAY
			unsetenv("TMPDIR");	/* force the use of auto_dir */
#endif
			if (stat(auto_dir, &statbuf) == -1) {
				if (mkdir(auto_dir, 0777) == -1) {
					if (errno != EEXIST) {
						fprintf(stderr,
							"%s%s: Unable to make dir %s\n",
							Progname, TagName,
							auto_dir);
						exit(1);
					}
				}
			} else {
				if (!(statbuf.st_mode & S_IFDIR)) {
					fprintf(stderr,
						"%s%s: %s already exists and is not a directory\n",
						Progname, TagName, auto_dir);
					exit(1);
				}
			}
			break;

		case 'D':
			if (sscanf(optarg, "%i", &Debug) != 1) {
				fprintf(stderr,
					"%s%s: --D option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 'e':
			if (sscanf(optarg, "%i", &Maxerrs) != 1) {
				fprintf(stderr,
					"%s%s: --e option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 'f':
			auto_file = optarg;
			break;

		case 'g':
			if ((ret = sscanf(optarg, "%i%c", &grow_incr, &chr)) < 1
			    || grow_incr < 0) {

				fprintf(stderr,
					"%s%s: --g option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			if (ret == 2) {
				if (chr == 'b' || chr == 'B')
					grow_incr *= 4096;
				else {
					fprintf(stderr,
						"%s%s: --g option arg invalid\n",
						Progname, TagName);
					usage();
					exit(1);
				}
			}
			break;

		case 'H':
			if (sscanf(optarg, "%f", &delaysecs) != 1
			    || delaysecs < 0) {

				fprintf(stderr,
					"%s%s: --H option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 'i':
			if (sscanf(optarg, "%i", &iterations) != 1 ||
			    iterations < 0) {

				fprintf(stderr,
					"%s%s: --i option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 'I':
#if NEWIO
			if ((io_type = lio_parse_io_arg1(optarg)) == -1) {
				fprintf(stderr,
					"%s%s: --I arg is invalid, must be s, p, f, a, l, L or r.\n",
					Progname, TagName);
				exit(1);
			}
			if (io_type & LIO_RANDOM)
				using_random++;
#else
			if ((io_type = parse_io_arg(optarg)) == -1) {
				fprintf(stderr,
					"%s%s: --I arg is invalid, must be s, p, f, a, l, L or r.\n",
					Progname, TagName);
				exit(1);
			}
			if (io_type == 99)	/* hold-over until tlibio.h */
				using_random++;
#endif
			break;

		case 'l':
			lockfile++;
			if (lockfile > 2)
				lockfile = 2;	/* lockfile can only be 1 or 2 */
			break;

		case 'L':
			if (sscanf(optarg, "%i", &time_iterval) != 1 ||
			    time_iterval < 0) {
				fprintf(stderr,
					"%s%s: --L option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 'n':
			if (sscanf(optarg, "%i:%i", &num_procs, &forker_mode) <
			    1 || num_procs < 0) {

				fprintf(stderr,
					"%s%s: --n option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}

			break;

		case 'N':
			if (sscanf(optarg, "%i", &num_auto_files) != 1 ||
			    num_auto_files < 0) {

				fprintf(stderr,
					"%s%s: --N option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 'O':
			if (sscanf(optarg, "%i", &Alignment) != 1 ||
			    Alignment < 0) {

				fprintf(stderr,
					"%s%s: --O option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 'o':
			if (strcmp(optarg, "random") == 0) {
				open_flags = RANDOM_OPEN;
				using_random++;

			} else if ((open_flags = parse_open_flags(optarg, NULL))
				   == -1) {
				fprintf(stderr,
					"%s%s: --o arg contains invalid flag\n",
					Progname, TagName);
				exit(1);
			}
			break;

		case 'p':	/* pre allocate space */
#ifdef linux
			printf("%s%s: --p is illegal option on linux system\n",
			       Progname, TagName);
			exit(1);
#else
			pre_alloc_space++;
#endif
			break;

		case 'P':
#ifdef CRAY
			if (strcmp(optarg, "PANIC") != 0) {
				fprintf(stderr, "%s%s: --P arg must be PANIC\n",
					Progname, TagName);
				exit(1);
			}
			Upanic_on_error++;
			printf("%s%s: Will call upanic after writes\n", Progname, TagName);
#else
			printf
			    ("%s%s: --P is illegal option on non-cray system\n",
			     Progname, TagName);
			exit(1);
#endif
			break;

		case 'q':	/* file content or pattern */
			switch (optarg[0]) {
			case 'A':
				Pattern = PATTERN_ALT;
				break;
			case 'a':
				Pattern = PATTERN_ASCII;
				break;
			case 'p':
				Pattern = PATTERN_PID;
				break;
			case 'o':
				Pattern = PATTERN_OFFSET;
				break;
			case 'c':
				Pattern = PATTERN_CHKER;
				break;
			case 'C':
				Pattern = PATTERN_CNTING;
				break;
			case 'r':
				Pattern = PATTERN_RANDOM;
				using_random++;
				break;
			case 'z':
				Pattern = PATTERN_ZEROS;
				break;
			case 'O':
				Pattern = PATTERN_ONES;
				break;
			default:
				fprintf(stderr,
					"%s%s: --C option arg invalid, A, a, p, o, c, C, r, z, or 0\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 'R':	/* random lseek before write arg: [min-]max */
			if (sscanf(optarg, "%i-%i", &min_lseek, &max_lseek) !=
			    2) {
				min_lseek = 1;	/* same as default in define */
				if (sscanf(optarg, "%i%c", &max_lseek, &chr) !=
				    1) {
					fprintf(stderr,
						"%s%s: --R option arg invalid: [min-]max\n",
						Progname, TagName);
					exit(1);
				}
			}
			if (max_lseek < LSK_EOFMINUSGROW) {
				fprintf(stderr,
					"%s%s: --R option, max_lseek is invalid\n",
					Progname, TagName);
				exit(1);
			}
			Mode |= MODE_RAND_LSEEK;
			using_random++;
			break;

		case 'r':	/* random io size arg: [min-]max[:mult] */

			/* min-max:mult format */
			if (sscanf(optarg, "%i-%i:%i%c", &min_size, &max_size,
				   &mult_size, &chr) != 3) {
				min_size = 1;
				/* max:mult format */
				if (sscanf(optarg, "%i:%i%c", &max_size,
					   &mult_size, &chr) != 2) {
					/* min-max format */
					if (sscanf(optarg, "%i-%i%c", &min_size,
						   &max_size, &chr) != 2) {
						min_size = 1;
						if (sscanf
						    (optarg, "%i%c", &max_size,
						     &chr) != 1) {
							fprintf(stderr,
								"%s%s: --r option arg invalid: [min-]max[:mult]\n",
								Progname,
								TagName);
							exit(1);
						}
					}
				}
			}

			if (max_size < 0) {
				fprintf(stderr,
					"%s%s: --r option, max_size is invalid\n",
					Progname, TagName);
				exit(1);
			}
			/*
			 * If min and max are the same, no randomness
			 */
			if (min_size != max_size) {
				Mode |= MODE_RAND_SIZE;
				using_random++;
			}
			break;

		case 'S':
			if (sscanf(optarg, "%i", &seq_auto_files) != 1 ||
			    seq_auto_files < 0) {

				fprintf(stderr,
					"%s%s: --S option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 's':	/* format: seed[,seed...] */

			/* count the number of seeds */
			cptr = optarg;
			for (Nseeds = 1; *cptr; Nseeds++) {
				if ((filename = strchr(cptr, ',')) == NULL)
					break;
				cptr = filename;
				cptr++;
			}
			Seeds = malloc(Nseeds * sizeof(int));

			/*
			 * check that each seed is valid and put them in
			 * the newly malloc'ed Seeds arrary.
			 */
			filename = cptr = optarg;
			for (Nseeds = 0; *cptr; Nseeds++) {
				if ((filename = strchr(cptr, ',')) == NULL) {
					if (sscanf(cptr, "%i", &Seeds[Nseeds]) <
					    1) {
						fprintf(stderr,
							"%s%s: --s option arg %s invalid\n",
							Progname, TagName,
							cptr);
						usage();
						exit(1);
					}
					Nseeds++;
					break;
				}

				*filename = '\0';
				if (sscanf(cptr, "%i", &Seeds[Nseeds]) < 1) {
					fprintf(stderr,
						"%s%s: --s option arg %s invalid\n",
						Progname, TagName, cptr);
					usage();
					exit(1);
				}
				*filename = ',';	/* restore string */
				cptr = filename;
				cptr++;
			}
			break;

		case 't':
			if ((ret =
			     sscanf(optarg, "%i%c", &trunc_incr, &chr)) < 1
			    || trunc_incr < 0) {

				fprintf(stderr,
					"%s%s: --t option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			if (ret == 2) {
				if (chr == 'b' || chr == 'B')
					trunc_incr *= 4096;
				else {
					fprintf(stderr,
						"%s%s: --t option arg invalid\n",
						Progname, TagName);
					usage();
					exit(1);
				}
			}
			break;

		case 'T':	/* truncate interval */
			if (sscanf(optarg, "%i%c", &trunc_inter, &chr) != 1 ||
			    trunc_inter < 0) {

				fprintf(stderr,
					"%s%s: --T option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 'u':
			remove_files++;
			break;

		case 'U':	/* how often to unlink file */
			/*
			 * formats:
			 *      A-B  - randomly pick interval between A and B
			 *      X    - unlink file every X iteration
			 */
			if (sscanf(optarg, "%i-%i", &unlink_inter,
				   &unlink_inter_ran) == 2) {

				if (unlink_inter < 0 || unlink_inter_ran < 0) {
					fprintf(stderr,
						"%s%s: --U option arg invalid\n",
						Progname, TagName);
					usage();
					exit(1);
				}
				/* ensure unlink_inter contains smaller value */
				if (unlink_inter > unlink_inter_ran) {
					tmp = unlink_inter_ran;
					unlink_inter_ran = unlink_inter;
					unlink_inter = tmp;
				}
				using_random++;

			} else if (sscanf(optarg, "%i%c", &unlink_inter, &chr)
				   != 1 || unlink_inter < 0) {

				fprintf(stderr,
					"%s%s: --U option arg invalid\n",
					Progname, TagName);
				usage();
				exit(1);
			}
			break;

		case 'x':
			if (reexec != REXEC_DONE)
				reexec = REXEC_DOIT;
			break;

		case 'w':
			Mode |= MODE_GROW_BY_LSEEK;
			break;

		case 'W':
			TCID = optarg;
			sprintf(TagName, "(%.39s)", optarg);
			break;

		case 'y':
			Sync_with_others = 1;
			break;

		case '?':
			usage();
			exit(1);
			break;
		}
	}

	if (Debug == 1) {
		cptr = getenv("TOUTPUT");
		if ((cptr != NULL) && (strcmp(cptr, "NOPASS") == 0)) {
			Debug = 0;
		}
	}

	if (Pattern == PATTERN_RANDOM) {
		no_file_check = 1;
		if (write_check_inter || file_check_inter)
			printf
			    ("%s%s: %d Using random pattern - no data checking will be performed!\n",
			     Progname, TagName, getpid());
	} else if (max_lseek == LSK_EOFPLUSGROW || Mode & MODE_GROW_BY_LSEEK) {
		no_file_check = 1;

		if (file_check_inter)
			printf("%s%s: %d Using random lseek beyond EOF or lseek grow,\n\
no whole file checking will be performed!\n", Progname, TagName,
			       getpid());

	}

	if (Mode & MODE_RAND_SIZE)
		grow_incr = max_size;

	set_sig();

	Opid = getpid();
	Pid = Opid;

	if (backgrnd) {
		if (Debug > 1)
			printf
			    ("%s: %d DEBUG2 forking, returning control to the user\n",
			     Progname, Opid);
		background(Progname);	/* give user their prompt back */
	}
#if CRAY
	if (Sync_with_others)
		setpgrp();
#endif

	if (Debug > 3) {
#if NEWIO
		lio_set_debug(Debug - 3);
#else
		set_iowrite_debug(Debug - 3);
#endif
	}

	/*
	 * Print some program information here if debug is turned on to
	 * level 3 or higher.
	 */

	if (Debug > 2) {

		if (Mode & MODE_GROW_BY_LSEEK)
			printf
			    ("%s: %d DEBUG lseeking past end of file, writting a \"w\"\n",
			     Progname, Pid);
		else if (Pattern == PATTERN_OFFSET)
			printf
			    ("%s: %d DEBUG3 %d<byteoffset>%d per word pattern multi-writers.\n",
			     Progname, Pid, STATIC_NUM, STATIC_NUM);
		else if (Pattern == PATTERN_PID)
			printf
			    ("%s: %d DEBUG3 <pid><byteoffset><pid> per word pattern - 1 writer\n",
			     Progname, Pid);
		else if (Pattern == PATTERN_ASCII)
			printf
			    ("%s: %d DEBUG3 ascii pattern (vi'able)- allows multiple writers\n",
			     Progname, Pid);
		else if (Pattern == PATTERN_ALT)
			printf
			    ("%s: %d DEBUG3 alt bit pattern - allows multiple writers\n",
			     Progname, Pid);
		else if (Pattern == PATTERN_CHKER)
			printf
			    ("%s: %d DEBUG3 checkerboard pattern - allows multiple writers\n",
			     Progname, Pid);
		else if (Pattern == PATTERN_CNTING)
			printf
			    ("%s: %d DEBUG3 counting pattern - allows multiple writers\n",
			     Progname, Pid);
		else if (Pattern == PATTERN_RANDOM)
			printf
			    ("%s: %d DEBUG3 random integer pattern - no write/file checking\n",
			     Progname, Pid);
		else if (Pattern == PATTERN_ONES)
			printf
			    ("%s: %d DEBUG3 all ones pattern - allows multiple writers\n",
			     Progname, Pid);
		else if (Pattern == PATTERN_ZEROS)
			printf
			    ("%s: %d DEBUG3 all zeros pattern - allows multiple writers\n",
			     Progname, Pid);

		else
			printf("%s: %d DEBUG3 unknown pattern\n",
			       Progname, Pid);
		if (bytes_to_consume)
			printf("%s: %d DEBUG3 bytes_to_consume = %d\n",
			       Progname, Pid, bytes_to_consume);
		printf
		    ("%s: %d DEBUG3 Maxerrs = %d, pre_alloc_space = %d, filelocking = %d\n",
		     Progname, Pid, Maxerrs, pre_alloc_space, lockfile);

		printf
		    ("%s: %d DEBUG3 Debug = %d, remove files in cleanup : %d\n",
		     Progname, Pid, Debug, remove_files);

		printf("%s: %d DEBUG3 Mode = %#o\n", Progname, Pid, Mode);

		if (open_flags == RANDOM_OPEN)
			printf
			    ("%s: %d DEBUG3 open_flags = (random), io_type = %#o\n",
			     Progname, Pid, io_type);
		else
			printf
			    ("%s: %d DEBUG3 open_flags = %#o, io_type = %#o\n",
			     Progname, Pid, open_flags, io_type);

		if (Mode & MODE_RAND_SIZE) {
			printf
			    ("%s: %d DEBUG3 random write/trunc:  min=%d, max=%d, mult = %d\n",
			     Progname, Pid, min_size, max_size, mult_size);
		} else {
			printf("%s: %d DEBUG3 grow_incr = %d\n",
			       Progname, Pid, grow_incr);
		}
		if (Mode & MODE_RAND_LSEEK) {
			if (max_lseek == LSK_EOF)
				printf
				    ("%s: %d DEBUG3 random lseek:  min=%d, max=<endoffile>\n",
				     Progname, Pid, min_lseek);
			else if (max_lseek == LSK_EOFPLUSGROW)
				printf
				    ("%s: %d DEBUG3 random lseek:  min=%d, max=<endoffile+iosize>\n",
				     Progname, Pid, min_lseek);
			else if (max_lseek == LSK_EOFMINUSGROW)
				printf
				    ("%s: %d DEBUG3 random lseek:  min=%d, max=<endoffile-iosize>\n",
				     Progname, Pid, min_lseek);
			else
				printf
				    ("%s: %d DEBUG3 random lseek:  min=%d, max=%d\n",
				     Progname, Pid, min_lseek, max_lseek);
		}

		printf
		    ("%s: %d DEBUG3 check write interval = %d, check file interval = %d\n",
		     Progname, Pid, write_check_inter, file_check_inter);

		printf("%s: %d DEBUG3 trunc interval = %d, trunc_incr = %d\n",
		       Progname, Pid, trunc_inter, trunc_incr);

		if (no_file_check)
			printf
			    ("%s: %d DEBUG3 no whole file checking will be done\n",
			     Progname, Pid);

		if (unlink_inter_ran == -1) {
			printf("%s: %d DEBUG3 unlink_inter = %d\n",
			       Progname, Pid, unlink_inter);
		} else {
			printf
			    ("%s: %d DEBUG3 unlink_inter = %d, unlink_inter_ran = %d\n",
			     Progname, Pid, unlink_inter, unlink_inter_ran);
		}

		if (Debug > 8) {
			num = sizeof(Open_flags) / sizeof(int);
			printf("%s: %d DEBUG9 random open flags values:\n",
			       Progname, Pid);
			for (ind = 0; ind < num; ind++) {
				printf("\t%#o\n", Open_flags[ind]);
			}
		}
	}
	/* end of DEBUG > 2 */
	if (Debug > 1 && num_procs > 1) {
		printf("%s: %d DEBUG2 about to fork %d more copies\n", Progname,
		       Opid, num_procs - 1);
	}

	fflush(stdout);		/* ensure pending i/o is flushed before forking */
	fflush(stderr);

	forker(num_procs, forker_mode, Progname);

	Pid = getpid();		/* reset after the forks */
	/*
	 * If user specified random seed(s), get that random seed value.
	 * get random seed if it was not specified by the user.
	 * This is done after the forks, because pid is used to get the seed.
	 */
	if (Nseeds == 1) {
		/*
		 * If only one seed specified, all processes will get that seed.
		 */
		Seed = Seeds[0];
	} else if (Nseeds > 1) {
		/*
		 * More than one seed was specified.
		 * The original process gets the first seed.  Each
		 * process will be get the next seed in the specified list.
		 */
		if (Opid == Pid) {
			Seed = Seeds[0];
		} else {
			/*
			 * If user didn't specify enough seeds, use default method.
			 */
			if (Forker_npids >= Nseeds) {
				struct timeval ts;
				gettimeofday(&ts, NULL);
				Seed = ts.tv_sec + Pid;	/* default random seed */
			} else {
				Seed = Seeds[Forker_npids];
			}
		}
	} else {
		/*
		 * Generate a random seed based on time and pid.
		 * It has a good chance of being unique for each pid.
		 */
		struct timeval ts;
		gettimeofday(&ts, NULL);
		Seed = ts.tv_sec + Pid;	/* default random seed */
		//Seed=time(0) + Pid;  /* default random seed */

	}

	random_range_seed(Seed);

	if (using_random && Debug > 0)
		printf("%s%s: %d DEBUG1 Using random seed of %d\n",
		       Progname, TagName, Pid, Seed);

	if (unlink_inter_ran > 0) {
		/*
		 * Find unlinking file interval.  This must be done after
		 * the seed was set.   This allows multiple copies to
		 * get different intervals.
		 */
		tmp = unlink_inter;
		unlink_inter =
		    (int)random_range(tmp, unlink_inter_ran, 1, NULL);

		if (Debug > 2)
			printf
			    ("%s: %d DEBUG3 Unlink interval is %d (random %d - %d)\n",
			     Progname, Pid, unlink_inter, tmp,
			     unlink_inter_ran);
	}

	/*
	 * re-exec all childern if reexec is set to REXEC_DOIT.
	 * This is useful on MPP systems to get the
	 * child process on another PE.
	 */
	if (reexec == REXEC_DOIT && Opid != Pid) {
		if (exec_path == NULL) {
			exec_path = argv[0];
			/* Get space for cmd (2 extra, 1 for - and 1 fro NULL */
			argv[0] = malloc(strlen(exec_path) + 2);
			sprintf(argv[0], "-%s", exec_path);
		}

		if (Debug > 2)
			printf("%s: %d DEBUG3 %s/%d: execvp(%s, argv)\n",
			       Progname, Pid, __FILE__, __LINE__, argv[0]);

		execvp(argv[0], argv);
	}

	/*** begin filename stuff here *****/
	/*
	 * Determine the number of files to be dealt with
	 */
	if (optind == argc) {
		/*
		 * no cmd line files, therfore, set
		 * the default number of auto created files
		 */
		if (!num_auto_files && !seq_auto_files)
			num_auto_files = 1;
	} else {
		first_file_ind = optind;
		num_files += argc - optind;
	}

	if (num_auto_files) {
		num_files += num_auto_files;
	}

	if (seq_auto_files) {
		num_files += seq_auto_files;
	}

	/*
	 * get space for file names
	 */
	if ((filenames = malloc(num_files * PATH_MAX)) == NULL) {
		fprintf(stderr, "%s%s: %d %s/%d: malloc(%d) failed: %s\n",
			Progname, TagName, Pid, __FILE__, __LINE__,
			num_files * PATH_MAX, strerror(errno));
		exit(1);
	}

	/*
	 * fill in filename cmd files then auto files.
	 */

	num = 0;
	if (first_file_ind) {
		for (ind = first_file_ind; ind < argc; ind++, num++) {
			strcpy((char *)filenames + (num * PATH_MAX), argv[ind]);
		}
	}

	/*
	 * construct auto filename and insert them into filenames space
	 */

	for (ind = 0; ind < num_auto_files; ind++, num++) {
		gettimeofday(&tv1, NULL);
		sprintf((char *)filenames + (num * PATH_MAX),
			"%s/%s%ld%ld%d.%d", auto_dir, auto_file,
			(long)tv1.tv_sec, (long)tv1.tv_usec, rand(), ind);
	}

	/*
	 * construct auto seq filenames
	 */
	for (ind = 1; ind <= seq_auto_files; ind++, num++) {
		sprintf((char *)filenames + (num * PATH_MAX), "%s/%s%d",
			auto_dir, auto_file, ind);
	}

/**** end filename stuff ****/

	if (time_iterval > 0) {
		struct timeval ts;
		gettimeofday(&ts, NULL);
		start_time = ts.tv_sec;
		//start_time=time(0);
	}

	/*
	 * get space for I/O buffer
	 */
	if (grow_incr) {
		if ((Buffer = malloc(grow_incr + Alignment)) == NULL) {
			fprintf(stderr,
				"%s%s: %d %s/%d: malloc(%d) failed: %s\n",
				Progname, TagName, Pid, __FILE__, __LINE__,
				grow_incr, strerror(errno));
			exit(1);
		}
		if (Alignment)
			Buffer = Buffer + Alignment;

	}

	if (Debug > 2) {
		printf("%s: %d DEBUG3 num_files = %d\n",
		       Progname, Pid, num_files);
	}
#ifndef linux
	if (pre_alloc_space) {
		if (iterations == 0) {
			fprintf(stderr,
				"%s%s: %d %s/%d: can NOT pre-alloc and grow forever\n",
				Progname, TagName, Pid, __FILE__, __LINE__);
			exit(1);
		}
		if (Mode & MODE_RAND_SIZE) {
			fprintf(stderr,
				"%s%s: %d %s/%d: can NOT pre-alloc and do random io size\n",
				Progname, TagName, Pid, __FILE__, __LINE__);
			exit(1);
		}

		total_grow_value = grow_incr * iterations;

		/*
		 * attempt to limit
		 */
		if (bytes_to_consume && bytes_to_consume < total_grow_value) {
			total_grow_value = bytes_to_consume;
		}
	}
#endif

	/*
	 * If delaying between iterations, get amount time to
	 * delaysecs in clocks or usecs.
	 * If on the CRAY, delaytime is in clocks since
	 * _rtc() will be used, which does not have the overhead
	 * of gettimeofday(2).
	 */
	if (delaysecs) {
#if CRAY
		int hz;
		hz = sysconf(_SC_CLK_TCK);
		delaytime = (int)((float)hz * delaysecs);
#else
		delaytime = (int)((float)USECS_PER_SEC * delaysecs);
#endif
	}

	if (statfs(auto_dir, &fsbuf) == -1) {
		fprintf(stderr, "%s%s: Unable to get the info of mounted "
			"filesystem that includes dir %s\n",
			Progname, TagName, auto_dir);
		exit(1);
	}

	/* Compare two values and use the smaller one as limit */
	fs_limit = MIN(fsbuf.f_bsize * fsbuf.f_bavail / num_files, fs_limit);

	/*
	 * This is the main iteration loop.
	 * Each iteration, all files can  be opened, written to,
	 * read to check the write, check the whole file,
	 * truncated, and closed.
	 */
	for (Iter_cnt = 1; !stop; Iter_cnt++) {
		struct timeval ts;
		if (iterations && (Iter_cnt >= iterations + 1)) {
			strcpy(reason, "Hit iteration value");
			stop = 1;
			continue;
		}
		gettimeofday(&ts, NULL);
		if ((time_iterval > 0)
		    && (start_time + time_iterval < ts.tv_sec)) {

			sprintf(reason, "Hit time value of %d", time_iterval);
			stop = 1;
			continue;
		}

		if (bytes_to_consume && bytes_consumed >= bytes_to_consume) {
			sprintf(reason, "Hit bytes consumed value of %d",
				bytes_to_consume);
			stop = 1;
			continue;
		}

		/*
		 * This loop will loop through all files.
		 * Each iteration, a single file can  be opened, written to,
		 * read to check the write, check the whole file,
		 * truncated, and closed.
		 */
		for (ind = 0; ind < num_files; ind++) {

			fflush(stdout);
			fflush(stderr);

			filename = (char *)filenames + (ind * PATH_MAX);
			Fileinfo.filename =
			    (char *)filenames + (ind * PATH_MAX);

			if (open_flags == RANDOM_OPEN) {
				ret =
				    Open_flags[random_range
					       (0,
						sizeof(Open_flags) /
						sizeof(int) - 1, 1, NULL)];
			}

			else
				ret = open_flags;

			Fileinfo.openflags = ret;

			if (Debug > 3) {
				printf
				    ("%s: %d DEBUG3 %s/%d: %d Open filename = %s, open flags = %#o %s\n",
				     Progname, Pid, __FILE__, __LINE__,
				     Iter_cnt, filename, ret,
				     openflags2symbols(ret, ",", 0));
			} else if (Debug > 2) {
				printf
				    ("%s: %d DEBUG3 %s/%d: %d filename = %s, open flags = %#o\n",
				     Progname, Pid, __FILE__, __LINE__,
				     Iter_cnt, filename, ret);
			}

			/*
			 * open file with desired flags.
			 */
			if ((fd = open(filename, ret, 0777)) == -1) {
				fprintf(stderr,
					"%s%s: %d %s/%d: open(%s, %#o, 0777) returned -1, errno:%d %s\n",
					Progname, TagName, Pid, __FILE__,
					__LINE__, filename, ret, errno,
					strerror(errno));
				handle_error();
				continue;
			}

			Fileinfo.fd = fd;

			lkfile(fd, LOCK_EX, LKLVL1);	/* lock if lockfile is LKLVL1 */

#ifndef linux
			/*
			 * preallocation is only done once, if specified.
			 */
			if (pre_alloc_space) {
				if (pre_alloc(fd, total_grow_value) != 0) {
					cleanup();
					exit(2);
				}
				if (Debug > 1) {
					printf
					    ("%s: %d DEBUG2 %s/%d: pre_allocated %ld for file %s\n",
					     Progname, Pid, __FILE__, __LINE__,
					     total_grow_value, filename);
				}
				lkfile(fd, LOCK_UN, LKLVL1);	/* release lock */
				close(fd);
				Iter_cnt = 0;	/* reset outside loop to restart from one */
				continue;
			}
#endif

			/*
			 * grow file by desired amount.
			 * growfile() will set the Grow_incr variable and
			 * possiblly update the Mode variable indicating
			 * if we are dealing with a FIFO file.
			 */

			/* BUG:14136 (don't go past filesystem size limit) */
			curr_size = file_size(fd);
			if (curr_size + grow_incr >= fs_limit) {
				lkfile(fd, LOCK_UN, LKLVL1);	/* release lock */
				close(fd);
				sprintf(reason,
					"Reached %ld filesize which is almost %ld limit.",
					curr_size, fs_limit);
				stop = 1;
				continue;
			}

			if (growfile(fd, filename, grow_incr, Buffer, &curr_size) != 0) {	/* BUG:14136 */
				handle_error();
				lkfile(fd, LOCK_UN, LKLVL1);	/* release lock */
				close(fd);
				continue;
			}

			/*
			 * check if last write is not corrupted
			 */
			if (check_write(fd, write_check_inter, filename,
					Mode) != 0) {
				handle_error();
			}

			/*
			 * Check that whole file is not corrupted.
			 */
			if (check_file(fd, file_check_inter, filename,
				       no_file_check) != 0) {
				handle_error();
			}

			/*
			 * shrink file by desired amount if it is time
			 */

			if (shrinkfile
			    (fd, filename, trunc_incr, trunc_inter,
			     Mode) != 0) {
				handle_error();
			}

			lkfile(fd, LOCK_UN, LKLVL1);	/* release lock */

			if (Debug > 4)
				printf
				    ("%s: %d DEBUG5 %s/%d: %d Closing file %s fd:%d \n",
				     Progname, Pid, __FILE__, __LINE__,
				     Iter_cnt, filename, fd);
			close(fd);

			/*
			 * Unlink the file if that is desired
			 */
			if (unlink_inter && (Iter_cnt % unlink_inter == 0)) {

				if (Debug > 4)
					printf
					    ("%s: %d DEBUG5 %s/%d: %d Unlinking file %s\n",
					     Progname, Pid, __FILE__, __LINE__,
					     Iter_cnt, filename);

				unlink(filename);
			}

			/*
			 * delay while staying active for "delaysecs" seconds.
			 */
			if (delaytime) {

				int ct, end;
#ifdef CRAY
				ct = _rtc();
				end = ct + delaytime;
				while (ct < end) {
					ct = _rtc();
				}
#else
				struct timeval curtime;
				gettimeofday(&curtime, NULL);
				ct = curtime.tv_sec * USECS_PER_SEC +
				    curtime.tv_usec;
				end = ct + delaytime;
				while (ct < end) {

					gettimeofday(&curtime, NULL);
					ct = curtime.tv_sec * USECS_PER_SEC +
					    curtime.tv_usec;
				}
#endif
			}
		}
#ifndef linux
		/*
		 * if Iter_cnt == 0, then we pre allocated space to all files
		 * and we are starting outside loop over.  Set pre_alloc_space
		 * to zero otherwise we get in infinite loop
		 */
		if (Iter_cnt == 0) {
			pre_alloc_space = 0;
		}
#endif

	}			/* end iteration for loop */

	if (Debug) {
		printf("%s%s: %d %s/%d: DONE %d iterations to %d files. %s\n",
		       Progname, TagName, Pid, __FILE__, __LINE__, Iter_cnt,
		       num_files, reason);
	}
	fflush(stdout);
	fflush(stderr);

	cleanup();

	if (Errors) {
		if (Debug > 2) {
			printf("%s%s: %d DEBUG3 %d error(s) encountered\n",
			       Progname, TagName, Pid, Errors);
			printf
			    ("%s%s: %d DEBUG3 %s/%d: exiting with value of 1\n",
			     Progname, TagName, Pid, __FILE__, __LINE__);
		}
		exit(1);
	}
	if (Debug > 2) {
		printf
		    ("%s%s: %d DEBUG3 %s/%d: no errors, exiting with value of 0\n",
		     Progname, TagName, Pid, __FILE__, __LINE__);
	}

	exit(0);
	tst_exit();		/* to keep compiler happy */
}

/***********************************************************************
 *
 ***********************************************************************/
int set_sig(void)
{
	int sig;

	/*
	 * now loop through all signals and set the handlers
	 */

	for (sig = 1; sig < NSIG; sig++) {
		switch (sig) {
		case SIGKILL:
		case SIGSTOP:
		case SIGCONT:
#ifdef CRAY
		case SIGINFO:
		case SIGRECOVERY:
#endif /* CRAY */
#ifdef SIGCKPT
		case SIGCKPT:
#endif /* SIGCKPT */
#ifdef SIGRESTART
		case SIGRESTART:
#endif /* SIGRESTART */
		case SIGCHLD:
			break;

		default:
#ifdef sgi
			sigset(sig, sig_handler);
#else
/* linux and cray */
			signal(sig, sig_handler);
#endif
			break;
		}
	}			/* endfor */

	return 0;
}

/***********************************************************************
 *
 ***********************************************************************/
void sig_handler(int sig)
{
	int exit_stat = 2;

	if (sig == SIGUSR2) {
		fprintf(stdout,
			"%s%s: %d %s/%d: received SIGUSR2 (%d) - stopping.\n",
			Progname, TagName, Pid, __FILE__, __LINE__, sig);
#ifndef sgi
		signal(sig, sig_handler);	/* allow us to get this signal more than once */
#endif

	} else if (sig == SIGINT) {
		/* The user has told us to cleanup, don't pretend it's an error. */
		exit_stat = 0;
		if (Debug != 0) {
			fprintf(stderr,
				"%s%s: %d %s/%d: received unexpected signal: %d\n",
				Progname, TagName, Pid, __FILE__, __LINE__,
				sig);
		}
	} else {
		fprintf(stderr,
			"%s%s: %d %s/%d: received unexpected signal: %d\n",
			Progname, TagName, Pid, __FILE__, __LINE__, sig);
	}

	notify_others();
	cleanup();
	if (Debug > 2) {
		printf("%s%s: %d DEBUG3 %s/%d: Exiting with a value of %d\n",
		       Progname, TagName, Pid, __FILE__, __LINE__, exit_stat);
	}
	exit(exit_stat);
}

/***********************************************************************
 * this function attempts to send SIGUSR2 to other growfiles processes
 * telling them to stop.
 *
 ***********************************************************************/
static void notify_others(void)
{
	static int send_signals = 0;
	int ind;

	if (Sync_with_others && send_signals == 0) {

#if CRAY
		send_signals = 1;	/* only send signals once */
		if (Debug > 1)
			printf
			    ("%s%s: %d DEBUG2 %s/%d: Sending SIGUSR2 to pgrp\n",
			     Progname, TagName, Pid, __FILE__, __LINE__);
		killm(C_PGRP, getpgrp(), SIGUSR2);
#else
		send_signals = 1;	/* only send signals once */

		for (ind = 0; ind < Forker_npids; ind++) {
			if (Forker_pids[ind] != Pid)
				if (Debug > 1)
					printf
					    ("%s%s: %d DEBUG2 %s/%d: Sending SIGUSR2 to pid %d\n",
					     Progname, TagName, Pid, __FILE__,
					     __LINE__, Forker_pids[ind]);
			kill(Forker_pids[ind], SIGUSR2);
		}
#endif
	}

}

/***********************************************************************
 * this function will count the number of errors encountered.
 * This function will call upanic if wanted or cleanup and
 * and exit is Maxerrs were encountered.
 ***********************************************************************/
int handle_error(void)
{
	Errors++;

#ifdef CRAY
	if (Errors & Upanic_on_error) {
		upanic(PA_PANIC);
	}
#endif

	if (Maxerrs && Errors >= Maxerrs) {
		printf("%s%s: %d %s/%d: %d Hit max errors value of %d\n",
		       Progname, TagName, Pid, __FILE__, __LINE__, Iter_cnt,
		       Maxerrs);
		notify_others();
		cleanup();

		if (Debug > 2) {
			printf("%s%s: %d DEBUG3 %d error(s) encountered\n",
			       Progname, TagName, Pid, Errors);
			printf
			    ("%s%s: %d DEBUG3 %s/%d: exiting with value of 1\n",
			     Progname, TagName, Pid, __FILE__, __LINE__);
		}

		exit(1);
	}

	return 0;
}

/***********************************************************************
 *
 ***********************************************************************/
int cleanup(void)
{
	int ind;

	if (remove_files) {
		if (Debug > 2)
			printf("%s: %d DEBUG3 Removing all %d files\n",
			       Progname, Pid, num_files);
		for (ind = 0; ind <= num_files; ind++) {
			unlink(filenames + (ind * PATH_MAX));
		}
	}
	if (using_random && Debug > 1)
		printf("%s%s: %d DEBUG2 Used random seed: %d\n",
		       Progname, TagName, Pid, Seed);
	return 0;
}

/***********************************************************************
 *
 ***********************************************************************/
void usage(void)
{
	fprintf(stderr,
		"Usage: %s%s [-bhEluy][[-g grow_incr][-i num][-t trunc_incr][-T trunc_inter]\n",
		Progname, TagName);
	fprintf(stderr,
		"[-d auto_dir][-e maxerrs][-f auto_file][-N num_files][-w][-c chk_inter][-D debug]\n");
	fprintf(stderr,
		"[-s seed][-S seq_auto_files][-p][-P PANIC][-I io_type][-o open_flags][-B maxbytes]\n");
	fprintf(stderr,
		"[-r iosizes][-R lseeks][-U unlk_inter][-W tagname] [files]\n");

	return;

}				/* end of usage */

/***********************************************************************
 *
 ***********************************************************************/
void help(void)
{
	usage();

	fprintf(stdout, "\
  -h             Specfied to print this help and exit.\n\
  -b             Specfied to execute in sync mode.(def async mode)\n\
  -B maxbytes    Max bytes to consume by all files.  growfiles exits when more\n\
                 than maxbytes have been consumed. (def no chk)  If maxbytes ends\n\
                 with the letter 'b', maxbytes is multiplied by BSIZE\n\
  -C write_chk   Specifies how often to check the last write (default 1)\n\
  -c file_chk    Specifies how often to check whole file (default 0)\n\
  -d auto_dir    Specifies the directory to auto created files. (default .)\n\
  -D debug_lvl   Specifies the debug level (default 1)\n\
  -E             Print examples and exit\n\
  -e errs        The number errors that will terminate this program (def 100)\n\
  -f auto_file   Specifies the base filename files created. (default \"gf\")\n\
  -g grow_incr   Specfied to grow by incr for each num. (default 4096)\n\
                 grow_incr may end in b for blocks\n\
		 If -r option is used, this option is ignored and size is random\n\
  -H delay       Amount of time to delay between each file (default 0.0)\n\
  -I io_type Specifies io type: s - sync, p - polled async, a - async (def s)\n\
		 l - listio sync, L - listio async, r - random\n\
  -i iteration   Specfied to grow each file num times. 0 means forever (default 1)\n\
  -l             Specfied to do file locking around write/read/trunc\n\
		 If specified twice, file locking after open to just before close\n\
  -L time        Specfied to exit after time secs, must be used with -i.\n\
  -N num_files   Specifies the number of files to be created.\n\
                 The default is zero if cmd line files.\n\
                 The default is one if no cmd line files.\n\
  -n num_procs   Specifies the number of copies of this cmd.\n\
  -o op_type     Specifies open flages: (def O_RDWR,O_CREAT) op_type can be 'random'\n\
  -O offset      adjust i/o buffer alignment by offset bytes\n\
  -P PANIC       Specifies to call upanic on error.\n\
  -p             Specifies to pre-allocate space\n\
  -q pattern     pattern can be a - ascii, p - pid with boff, o boff (def)\n\
		 A - Alternating bits, r - random, O - all ones, z - all zeros,\n\
		 c - checkboard, C - counting\n\
  -R [min-]max   random lseek before write and trunc, max of -1 means filesz,\n\
		 -2 means filesz+grow, -3 filesz-grow. (min def is 0)\n\
  -r [min-]max   random io write size (min def is 1)\n\
  -S seq_auto_files Specifies the number of seqental auto files (default 0)\n\
  -s seed[,seed...] Specifies the random number seed (default time(0)+pid)\n\
  -t trunc_incr  Specfied the amount to shrink file. (default 4096)\n\
                 trunc_inter may end in b for blocks\n\
		 If -R option is used, this option is ignored and trunc is random\n\
  -T trunc_inter Specfied the how many grows happen before shrink. (default 0)\n\
  -u             unlink files before exit\n\
  -U ui[-ui2]    Unlink files each ui iteration (def 0)\n\
  -w             Specfied to grow via lseek instead of writes.\n\
  -W tag-name	 Who-am-i.  My Monster tag name.  (used by Monster).\n\
  -x		 Re-exec children before continuing - useful on MPP systems\n\
  -y             Attempt to sync copies - if one fails it will send sigusr2 to others\n\
  Action to each file every iteration is open, write, write check\n\
  file check, trunc and closed.\n");

	return;
}

/***********************************************************************
 *
 ***********************************************************************/
void prt_examples(FILE * stream)
{
	/* This example creates 200 files in directory dir1.  It writes */
	/* 4090 bytes 100 times then truncates 408990 bytes off the file */
	/* The file contents are checked every 1000 grow. */
	fprintf(stream,
		"# run forever: writes of 4090 bytes then on every 100 iterval\n\
# truncate file by 408990 bytes.  Done to 200 files in dir1.\n\
%s -i 0 -g 4090 -T 100 -t 408990 -l -C 10 -c 1000 -d dir1 -S 200\n\n",
		Progname);

	/* same as above with 5000 byte grow and a 499990 byte tuncate */
	fprintf(stream,
		"# same as above with writes of 5000 bytes and truncs of 499990\n\
%s -i 0 -g 5000 -T 100 -t 499990 -l -C 10 -c 1000 -d dir2 -S 200\n\n",
		Progname);

	/* This example beats on opens and closes */
	fprintf(stream,
		"# runs forever: beats on opens and closes of file ocfile - no io\n\
%s -i 0 -g 0 -c 0 -C 0 ocfile\n\n",
		Progname);

	fprintf(stream, "# writes 4096 to files until 50 blocks are written\n\
%s -i 0 -g 4096 -B 50b file1 file2\n\n", Progname);

	fprintf(stream,
		"# write one byte to 750 files in gdir then unlinks them\n\
%s -g 1 -C 0 -d gdir -u -S 750\n\n", Progname);

	fprintf(stream, "# run 30 secs: random iosize, random lseek up to eof\n\
%s -r 1-5000 -R 0--1 -i 0 -L 30 -C 1 g_rand1 g_rand2\n\n", Progname);

	fprintf(stream,
		"# run 30 secs: grow by lseek then write single byte, trunc every 10 itervals\n\
%s -g 5000 -wlu -i 0 -L 30 -C 1 -T 10  g_sleek1 g_lseek2\n\n",
		Progname);

	fprintf(stream,
		"# run forever: 5 copies of random iosize, random lseek to beyond eof,\n\
# rand io types doing a trunc every 5 iterations, with unlinks.\n\
%s -i0 -r 1-50000 -R 0--2 -I r -C1 -l -n5 -u -U 100-200 gf_rana gf_ranb\n\n",
		Progname);

	fprintf(stream,
		"# run forever: 5 copies of random iosize, random lseek to beyond eof,\n\
# random open flags, rand io types doing a trunc every 10 iterations.\n\
%s -i0 -r 1-50000 -R 0--2 -o random -I r -C0 -l -T 20 -uU100-200 -n 5 gf_rand1 gf_rand2\n",
		Progname);

	return;
}

/***********************************************************************
 *
 * The file descriptor current offset is assumed to be the end of the
 * file.
 * Woffset will be set to the offset before the write.
 * Grow_incr will be set to the size of the write or lseek write.
 ***********************************************************************/
int /* BUG:14136 */ growfile(int fd, char *file, int grow_incr, char *buf,
			     unsigned long *curr_size_ptr)
{
	off_t noffset;
	int ret;
	int cur_offset;
	char *errmsg;
	off_t fsize;		/* current size of file */
	int size_grew;		/* size the file grew */
	struct stat stbuf;
	off_t off_tmp = 0;

	/*
	 * Do a stat on the open file.
	 * If the file is a fifo, set the bit in Mode variable.
	 * This fifo check must be done prior to growfile() returning.
	 * Also get the current size of the file.
	 */
	if (fstat(fd, &stbuf) != -1) {
		if (S_ISFIFO(stbuf.st_mode)) {
			Fileinfo.mode |= MODE_FIFO;
			Mode |= MODE_FIFO;
			if (Debug > 3)
				printf
				    ("%s: %d DEBUG4 %s/%d: file is a fifo - no lseek or truncs,\n",
				     Progname, Pid, __FILE__, __LINE__);
		}
		fsize = stbuf.st_size;

	} else {
		fprintf(stderr,
			"%s%s: %d %s/%d: Unable to fstat(%d, &buf), errno:%d %s\n",
			Progname, TagName, Pid, __FILE__, __LINE__, fd, errno,
			strerror(errno));

		return -1;
	}

	if (grow_incr <= 0) {	/* don't attempt i/o if grow_incr <= 0 */

		Grow_incr = grow_incr;
		if (Debug > 2)
			printf
			    ("%s: %d DEBUG3 %s/%d: Not attempting to grow, growsize == %d\n",
			     Progname, Pid, __FILE__, __LINE__, grow_incr);
		return grow_incr;
	}

	if (Mode & MODE_RAND_SIZE) {
		grow_incr =
		    random_range(min_size, max_size, mult_size, &errmsg);
		if (errmsg != NULL) {
			fprintf(stderr,
				"%s%s: %d %s/%d: random_range() failed - %s\n",
				Progname, TagName, Pid, __FILE__, __LINE__,
				errmsg);
			return -1;
		}
		Grow_incr = grow_incr;
	} else
		Grow_incr = grow_incr;

	if (!(Mode & MODE_FIFO)) {
		if ((cur_offset = lseek(fd, 0, SEEK_CUR)) == -1) {
			fprintf(stderr, "%s%s: %d %s/%d: tell failed: %s\n",
				Progname, TagName, Pid, __FILE__, __LINE__,
				strerror(errno));
			return -1;
		}
	}

	if (Mode & MODE_GROW_BY_LSEEK) {
		Woffset = fsize;
		if (Debug > 2) {
			printf
			    ("%s: %d DEBUG3 %s/%d: Current size of file is %ld\n",
			     Progname, Pid, __FILE__, __LINE__, (long)Woffset);
			printf
			    ("%s: %d DEBUG3 %s/%d: lseeking to %d byte with SEEK_END\n",
			     Progname, Pid, __FILE__, __LINE__, grow_incr - 1);
		}

		if ((noffset = lseek(fd, grow_incr - 1, SEEK_END)) == -1) {
			fprintf(stderr,
				"%s%s: %s/%d: lseek(fd, %d, SEEK_END) failed: %s\n",
				Progname, TagName, __FILE__, __LINE__,
				grow_incr - 1, strerror(errno));
			return -1;
		}

		lkfile(fd, LOCK_EX, LKLVL0);	/* get exclusive lock */

#if NEWIO
		ret =
		    lio_write_buffer(fd, io_type, "w", 1, SIGUSR1, &errmsg, 0);
#else
		ret = write_buffer(fd, io_type, "w", 1, 0, &errmsg);
#endif

		if (ret != 1) {
			fprintf(stderr, "%s%s: %d %s/%d: %d %s\n",
				Progname, TagName, Pid, __FILE__, __LINE__,
				Iter_cnt, errmsg);
			if (ret == -ENOSPC) {
				cleanup();
				exit(2);
			}
		}
/***
		write(fd, "w", 1);
****/

		lkfile(fd, LOCK_UN, LKLVL0);

		if (Debug > 2)
			printf("%s: %d DEBUG3 %s/%d: %d wrote 1 byte to file\n",
			       Progname, Pid, __FILE__, __LINE__, Iter_cnt);

	} else {		/* end of grow by lseek */

		if (Fileinfo.openflags & O_APPEND) {
			/*
			 * Deal with special case of the open flag containing O_APPEND.
			 * If it does, the current offset does not matter since the write
			 * will be done end of the file.
			 */
			if (Debug > 4)
				printf
				    ("%s: %d DEBUG5 %s/%d: dealing with O_APPEND condition\n",
				     Progname, Pid, __FILE__, __LINE__);
			lkfile(fd, LOCK_EX, LKLVL0);	/* get exclusive lock */

			/*
			 * do fstat again to get size of the file.
			 * This is done inside a file lock (if locks are being used).
			 */
			if (fstat(fd, &stbuf) != -1) {
				Woffset = stbuf.st_size;
			} else {
				fprintf(stderr,
					"%s%s: %d %s/%d: Unable to fstat(%d, &buf), errno:%d %s\n",
					Progname, TagName, Pid, __FILE__,
					__LINE__, fd, errno, strerror(errno));

				lkfile(fd, LOCK_UN, LKLVL0);	/* release lock */
				return -1;
			}
			if (Debug > 2)
				printf
				    ("%s: %d DEBUG3 %s/%d: dealing with O_APPEND condition (offset:fsz:%d)\n",
				     Progname, Pid, __FILE__, __LINE__,
				     (int)stbuf.st_size);

		} else if (Mode & MODE_RAND_LSEEK) {
			if (max_lseek == LSK_EOF) {	/* within file size */
				noffset =
				    random_range(min_lseek, fsize, 1, NULL);
			} else if (max_lseek == LSK_EOFPLUSGROW) {
				/* max to beyond file size */
				noffset =
				    random_range(min_lseek, fsize + grow_incr,
						 1, NULL);
			} else if (max_lseek == LSK_EOFMINUSGROW) {
				/*
				 * Attempt to not grow the file.
				 * If the i/o will fit from min_lseek to EOF,
				 * pick offset to allow it to fit.
				 * Otherwise, pick the min_lseek offset and grow
				 * file by smallest amount.
				 * If min_lseek is != 0, there will be a problem
				 * with whole file checking if file is ever smaller
				 * than min_lseek.
				 */
				if (fsize <= min_lseek + grow_incr)
					noffset = min_lseek;	/* file will still grow */
				else
					noffset =
					    random_range(min_lseek,
							 fsize - grow_incr, 1,
							 NULL);
			} else {
				noffset =
				    random_range(min_lseek, max_lseek, 1, NULL);
			}

			if ((Woffset = lseek(fd, noffset, SEEK_SET)) == -1) {
				fprintf(stderr,
					"%s%s: %d %s/%d: lseek(%d, %ld, "
					"SEEK_SET) l2 failed: %s\n", Progname,
					TagName, Pid, __FILE__, __LINE__, fd,
					(long)noffset, strerror(errno));
				return -1;
			} else if (Debug > 2)
				printf("%s: %d DEBUG3 %s/%d: lseeked to "
				       "random offset %ld (fsz:%d)\n",
				       Progname, Pid, __FILE__, __LINE__,
				       (long)Woffset, (int)stbuf.st_size);

		}

		/*
		 * lseek to end of file only if not fifo
		 */
		else if (!(Mode & MODE_FIFO)) {
			if ((Woffset = lseek(fd, 0, SEEK_END)) == -1) {
				fprintf(stderr,
					"%s%s: %d %s/%d: lseek(fd, 0, SEEK_END) failed: %s\n",
					Progname, TagName, Pid, __FILE__,
					__LINE__, strerror(errno));
				return -1;
			} else if (Debug > 2)
				printf("%s: %d DEBUG3 %s/%d: lseeked to "
				       "end of file, offset %ld\n",
				       Progname, Pid, __FILE__, __LINE__,
				       (long)Woffset);
		}

		if (Pattern == PATTERN_OFFSET)
			datapidgen(STATIC_NUM, buf, grow_incr, Woffset);
		else if (Pattern == PATTERN_PID)
			datapidgen(Pid, buf, grow_incr, Woffset);
		else if (Pattern == PATTERN_ASCII)
			dataasciigen(NULL, buf, grow_incr, Woffset);
		else if (Pattern == PATTERN_RANDOM)
			databingen('r', buf, grow_incr, Woffset);
		else if (Pattern == PATTERN_ALT)
			databingen('a', buf, grow_incr, Woffset);
		else if (Pattern == PATTERN_CHKER)
			databingen('c', buf, grow_incr, Woffset);
		else if (Pattern == PATTERN_CNTING)
			databingen('C', buf, grow_incr, Woffset);
		else if (Pattern == PATTERN_ZEROS)
			databingen('z', buf, grow_incr, Woffset);
		else if (Pattern == PATTERN_ONES)
			databingen('o', buf, grow_incr, Woffset);
		else
			dataasciigen(NULL, buf, grow_incr, Woffset);

		if (Debug > 2)
			printf
			    ("%s: %d DEBUG3 %s/%d: attempting to write %d bytes\n",
			     Progname, Pid, __FILE__, __LINE__, grow_incr);

		lkfile(fd, LOCK_EX, LKLVL0);	/* get exclusive lock */

/*****
		ret=write(fd, buf, grow_incr);

		off_tmp = tell(fd);

		lkfile(fd, LOCK_UN, LKLVL0);

		if (ret != grow_incr) {
			fprintf(stderr, "%s: %s/%d: write failed: %s\n",
				Progname, __FILE__, __LINE__, strerror(errno));
			return -1;
		}
*****/

#if NEWIO
		ret = lio_write_buffer(fd, io_type, buf, grow_incr,
				       SIGUSR1, &errmsg, 0);
#else
		ret = write_buffer(fd, io_type, buf, grow_incr, 0, &errmsg);
#endif

		if (Mode & MODE_FIFO) {
			/* If it is a fifo then just pretend the file
			 * offset is where we think it should be.
			 */
			off_tmp = Woffset + grow_incr;
		} else {
			if ((off_tmp = lseek(fd, 0, SEEK_CUR)) < 0) {	/* get offset after the write */
				fprintf(stderr,
					"%s%s: %s/%d: tell(2) failed: %d  %s\n",
					Progname, TagName, __FILE__, __LINE__,
					errno, strerror(errno));
				return -1;
			}
#if NEWIO
#if defined(sgi) || defined(__linux__)
			/* If this is POSIX I/O and it is via aio_{read,write}
			 * or lio_listio then after completion of the I/O the
			 * value of the file offset for the file is
			 * unspecified--which means we cannot trust what
			 * tell() told us.  Fudge it here.
			 */
			if ((io_type & LIO_IO_ASYNC_TYPES)
			    || (io_type & LIO_RANDOM)) {
				if (off_tmp != Woffset + grow_incr) {
					if (Debug > 5) {
						printf
						    ("%s: %d DEBUG6 %s/%d: posix fudge, forcing tmp (%"
						     PRId64
						     ") to match Woffset+grow_incr (%"
						     PRId64 ")\n", Progname,
						     Pid, __FILE__, __LINE__,
						     (int64_t) off_tmp,
						     (int64_t) Woffset +
						     grow_incr);
					}
					off_tmp = Woffset + grow_incr;
				}
			}
#endif /* sgi __linux__ */
#endif
		}
		*curr_size_ptr = off_tmp;	/* BUG:14136 */

		lkfile(fd, LOCK_UN, LKLVL0);

		if (ret != grow_incr) {
			fprintf(stderr, "%s%s: %d %s/%d: %d %s\n",
				Progname, TagName, Pid, __FILE__, __LINE__,
				Iter_cnt, errmsg);
			if (ret == -ENOSPC) {
				cleanup();
				exit(2);
			}
			return -1;
		}

		/*
		 * Check for a condition where the file was truncated just before
		 * the write.
		 */
		if (off_tmp != Woffset + grow_incr) {
			/*
			 * The offset after the write was not as expected.
			 * This could be caused by the following:
			 *  - file truncated after the lseek and before the write.
			 *  - the file was written to after fstat and before the write
			 *    and the file was opened with O_APPEND.
			 *
			 * The pattern written to the file will be considered corrupted.
			 */
			if (Debug > 0 && lockfile) {
				printf("%s%s: %d DEBUG1 %s/%d: offset after "
				       "write(%ld) not as exp(%ld+%d=%ld)\n",
				       Progname, TagName, Pid, __FILE__,
				       __LINE__, (long)off_tmp, (long)Woffset,
				       grow_incr, (long)(Woffset + grow_incr));
				printf
				    ("%s%s: %d DEBUG1 %s/%d: %d Assuming file "
				     "changed by another process, resetting "
				     "offset:%ld (expect pattern mismatch)\n",
				     Progname, TagName, Pid, __FILE__, __LINE__,
				     Iter_cnt, (long)(off_tmp - grow_incr));
			}
			if (Debug > 4) {
				printf
				    ("%s: %d DEBUG5 %s/%d: about to chop Woffset.  "
				     "tmp=%ld, grow_incr=%d, Woffset was %ld\n",
				     Progname, Pid, __FILE__, __LINE__,
				     (long)off_tmp, grow_incr, (long)Woffset);
			}
			Woffset = off_tmp - grow_incr;
			if (Woffset < 0)
				Woffset = 0;
		}

	}			/* end of grow by write */

	/*
	 * Woffset - holds start of grow (start of write expect in grow by lseek)
	 * Grow_incr - holds size of grow (write).
	 * fsize - holds size of file before write
	 */
	size_grew = (Woffset + Grow_incr) - fsize;
	if (Debug > 1) {
		if (Mode & MODE_FIFO) {
			printf
			    ("%s: %d DEBUG2 %s/%d: file is fifo, %d wrote %d bytes\n",
			     Progname, Pid, __FILE__, __LINE__, Grow_incr,
			     Iter_cnt);
		}

		else if (size_grew > 0)
			printf
			    ("%s: %d DEBUG2 %s/%d: %d wrote %d bytes(off:%ld), "
			     "grew file by %d bytes\n", Progname, Pid, __FILE__,
			     __LINE__, Iter_cnt, Grow_incr, (long)Woffset,
			     size_grew);
		else
			printf
			    ("%s: %d DEBUG2 %s/%d: %d wrote %d bytes(off:%ld), "
			     "did not grow file\n", Progname, Pid, __FILE__,
			     __LINE__, Iter_cnt, Grow_incr, (long)Woffset);
	}

	bytes_consumed += size_grew;
	return 0;

}				/* end of growfile */

/***********************************************************************
 * shrinkfile file by trunc_incr.  file can not be made smaller than
 * size zero.  Therefore, if trunc_incr is larger than file size,
 * file will be truncated to zero.
 * The file descriptor current offset is assumed to be the end of the
 * file.
 *
 ***********************************************************************/
int
shrinkfile(int fd, char *filename, int trunc_incr, int trunc_inter,
	   int just_trunc)
{
	static int shrink_cnt = 0;
	int cur_offset;
	int new_offset;
	int ret;
#ifdef CRAY
	int offset;
#endif

	shrink_cnt++;

	if (trunc_inter == 0 || (shrink_cnt % trunc_inter != 0)) {
		if (Debug > 3)
			printf
			    ("%s: %d DEBUG4 %s/%d: Not shrinking file - not time, iter=%d, cnt=%d\n",
			     Progname, Pid, __FILE__, __LINE__, trunc_inter,
			     shrink_cnt);
		return 0;	/* not this time */
	}

	if (Mode & MODE_FIFO) {
		if (Debug > 5)
			printf
			    ("%s: %d DEBUG5 %s/%d: Not attempting to shrink a FIFO\n",
			     Progname, Pid, __FILE__, __LINE__);
		return 0;	/* can not truncate fifo */
	}

	lkfile(fd, LOCK_EX, LKLVL0);

	if ((cur_offset = lseek(fd, 0, SEEK_CUR)) == -1) {
		fprintf(stderr, "%s%s: %d %s/%d: tell(%d) failed: %s\n",
			Progname, TagName, Pid, __FILE__, __LINE__, fd,
			strerror(errno));
		lkfile(fd, LOCK_UN, LKLVL0);
		return -1;
	}

	if (Mode & MODE_RAND_LSEEK) {
		if (max_lseek <= -1) {
			if ((new_offset = file_size(fd)) == -1) {
				lkfile(fd, LOCK_UN, LKLVL0);
				return -1;
			}

			if (new_offset < min_lseek)
				new_offset = min_lseek;
			else
				new_offset =
				    random_range(min_lseek, new_offset, 1,
						 NULL);
		} else {
			new_offset =
			    random_range(min_lseek, max_lseek, 1, NULL);
		}

#ifdef CRAY
		if ((offset = lseek(fd, new_offset, SEEK_SET)) == -1) {
			fprintf(stderr,
				"%s%s: %d %s/%d: lseek(%d, %d, SEEK_SET) l3 failed: %s\n",
				Progname, TagName, Pid, __FILE__, __LINE__, fd,
				new_offset, strerror(errno));
			lkfile(fd, LOCK_UN, LKLVL0);
			return -1;
		} else if (Debug > 3)
			printf
			    ("%s: %d DEBUG4 %s/%d: lseeked to random offset %d\n",
			     Progname, Pid, __FILE__, __LINE__, offset);

#endif
	}

	else {			/* remove trunc_incr from file */

		new_offset = cur_offset - trunc_incr;

		if (new_offset < 0)
			new_offset = 0;

#ifdef CRAY
		if (lseek(fd, new_offset, SEEK_SET) == -1) {
			fprintf(stderr,
				"%s%s: %d %s/%d: lseek(fd, %d, SEEK_SET) l4 failed: %s\n",
				Progname, TagName, Pid, __FILE__, __LINE__,
				new_offset, strerror(errno));
			lkfile(fd, LOCK_UN, LKLVL0);
			return -1;
		} else if (Debug > 3)
			printf
			    ("%s: %d DEBUG4 %s/%d: lseeked to offset %d, %d bytes from end\n",
			     Progname, Pid, __FILE__, __LINE__, new_offset,
			     trunc_incr);
#endif
	}

#ifdef CRAY
	ret = trunc(fd);
#else
	ret = ftruncate(fd, new_offset);
	if (ret == 0 && Debug > 3) {
		printf
		    ("%s: %d DEBUG4 %s/%d: ftruncated to offset %d, %d bytes from end\n",
		     Progname, Pid, __FILE__, __LINE__, new_offset, trunc_incr);
	}
#endif

	lkfile(fd, LOCK_UN, LKLVL0);

	if (ret == -1) {
#ifdef CRAY
		fprintf(stderr, "%s%s: %d %s/%d: trunc failed: %s\n",
			Progname, TagName, Pid, __FILE__, __LINE__,
			strerror(errno));
#else
		fprintf(stderr, "%s%s: %d %s/%d: ftruncate failed: %s\n",
			Progname, TagName, Pid, __FILE__, __LINE__,
			strerror(errno));
#endif
		return -1;
	}

	if (Debug > 2) {
		printf
		    ("%s: %d DEBUG2 %s/%d: trunc file by %d bytes, to size of = %d bytes\n",
		     Progname, Pid, __FILE__, __LINE__, cur_offset - new_offset,
		     new_offset);
	}

	bytes_consumed -= (cur_offset - new_offset);
	return 0;

}				/* end of shrinkfile */

/***********************************************************************
 *
 ***********************************************************************/
int check_write(int fd, int cf_inter, char *filename, int mode)
{
	int fsize;
	static int cf_count = 0;
	int ret = 0;
	int tmp;
	char *errmsg;
	char *ptr;

	cf_count++;

	if (cf_inter == 0 || (cf_count % cf_inter != 0)) {
		if (Debug > 4)
			printf
			    ("%s: %d DEBUG5 %s/%d: no write check, not time iter=%d, cnt=%d\n",
			     Progname, Pid, __FILE__, __LINE__, cf_inter,
			     cf_count);
		return 0;	/* no check done */
	}

	if (Grow_incr <= 0) {
		if (Debug > 3)
			printf("%s: %d DEBUG4 %s/%d: No write validation,  "
			       "Grow_incr = %d, offset = %ld\n",
			       Progname, Pid, __FILE__, __LINE__, Grow_incr,
			       (long)Woffset);
		return 0;	/* no check */
	}

	/*
	 * Get the shared file lock.  We need to hold the lock from before
	 * we do the stat until after the read.
	 */
	lkfile(fd, LOCK_SH, LKLVL0);

	if ((fsize = file_size(fd)) == -1) {
		lkfile(fd, LOCK_UN, LKLVL0);
		return -1;

	} else if (fsize <= Woffset) {
		/*
		 * The file was truncated between write and now.
		 * The contents of our last write is totally gone, no check.
		 */
		if (Debug > 1)
			printf
			    ("%s%s: %d DEBUG2 %s/%d: %d File size (%d) smaller than "
			     "where last wrote (%ld)- no write validation\n",
			     Progname, TagName, Pid, __FILE__, __LINE__,
			     Iter_cnt, fsize, (long)Woffset);
		lkfile(fd, LOCK_UN, LKLVL0);
		return 0;	/* no validation, but not an error */

	} else if (fsize < (Woffset + Grow_incr)) {
		/*
		 * The file was truncated between write and now.
		 * Part of our last write has been truncated, adjust our Grow_incr
		 * to reflect this.
		 */

		tmp = Grow_incr;
		Grow_incr = fsize - Woffset;

		if (Debug > 1) {

			printf("%s%s: %d DEBUG2 %s/%d: %d fsz:%d, lost(%d)of "
			       "wrt(off:%ld, sz:%d), adj=%d\n", Progname,
			       TagName, Pid, __FILE__, __LINE__, Iter_cnt,
			       fsize, tmp - Grow_incr, (long)Woffset, tmp,
			       Grow_incr);
		}

	}

	if (Debug > 2)
		printf("%s: %d DEBUG3 %s/%d: about to do write validation, "
		       "offset = %ld, size = %d\n",
		       Progname, Pid, __FILE__, __LINE__, (long)Woffset,
		       Grow_incr);

	if (!(mode & MODE_FIFO)) {

		if (lseek(fd, Woffset, 0) == -1) {
			fprintf(stderr,
				"%s%s: %d %s/%d: lseek(fd, %ld, 0) failed: %s\n",
				Progname, TagName, Pid, __FILE__, __LINE__,
				(long)Woffset, strerror(errno));
		}
		if (Debug > 3)
			printf("%s: %d DEBUG4 %s/%d: lseeked to offset:%ld\n",
			       Progname, Pid, __FILE__, __LINE__,
			       (long)Woffset);
	}

	/*
	 * Read last writes data
	 */
#if NEWIO
	ret =
	    lio_read_buffer(fd, io_type, Buffer, Grow_incr, SIGUSR1, &errmsg,
			    0);
#else
	ret = read_buffer(fd, io_type, Buffer, Grow_incr, 0, &errmsg);
#endif

	/*
	 * report the error and debug information before releasing
	 * the file lock
	 */
	if (ret != Grow_incr) {
		fprintf(stderr, "%s%s: %d %s/%d: %d CW %s\n", Progname, TagName,
			Pid, __FILE__, __LINE__, Iter_cnt, errmsg);
		{
			struct stat stbuf;
			fstat(fd, &stbuf);
			if (Debug > 2)
				printf("%s%s: %d DEBUG3 %s/%d: fd:%d, offset:%d, fsize:%d, openflags:%#o\n", Progname, TagName, Pid, __FILE__, __LINE__, fd, (int)lseek(fd, SEEK_CUR, 0),	/* FIXME: 64bit/LFS ? */
				       (int)stbuf.st_size, Fileinfo.openflags);
		}

		lkfile(fd, LOCK_UN, LKLVL0);
		return 1;
	}

	lkfile(fd, LOCK_UN, LKLVL0);

	if (Mode & MODE_GROW_BY_LSEEK) {
		/* check that all zeros upto last character */
		for (ptr = Buffer; ptr < (Buffer + Grow_incr - 1); ptr++) {
			if (*ptr != '\0') {
				fprintf(stderr,
					"%s%s: %d %s/%d: data mismatch at offset %d, exp:%#o(zerofilled), act:%#o in file %s\n",
					Progname, TagName, Pid, __FILE__,
					__LINE__,
					(int)(Woffset +
					      (Grow_incr - (Buffer - ptr))), 0,
					*ptr, filename);
				fflush(stderr);
				return 1;
			}
		}
		/* check that the last char is a 'w' */
		if (*ptr != 'w') {
			fprintf(stderr,
				"%s%s: %d %s/%d: data mismatch at offset %d, exp:%#o(zerofilled), act:%#o in file %s\n",
				Progname, TagName, Pid, __FILE__, __LINE__,
				(int)(Woffset + (Grow_incr - (Buffer - ptr))),
				'w', *ptr, filename);
			fflush(stderr);
			return 1;
		}
		return 0;	/* all is well */

	} else if (Pattern == PATTERN_OFFSET)
		ret =
		    datapidchk(STATIC_NUM, Buffer, Grow_incr, Woffset, &errmsg);
	else if (Pattern == PATTERN_PID)
		ret = datapidchk(Pid, Buffer, Grow_incr, Woffset, &errmsg);
	else if (Pattern == PATTERN_ASCII)
		ret = dataasciichk(NULL, Buffer, Grow_incr, Woffset, &errmsg);
	else if (Pattern == PATTERN_RANDOM) ;	/* no check for random */
	else if (Pattern == PATTERN_ALT)
		ret = databinchk('a', Buffer, Grow_incr, Woffset, &errmsg);
	else if (Pattern == PATTERN_CHKER)
		ret = databinchk('c', Buffer, Grow_incr, Woffset, &errmsg);
	else if (Pattern == PATTERN_CNTING)
		ret = databinchk('C', Buffer, Grow_incr, Woffset, &errmsg);
	else if (Pattern == PATTERN_ZEROS)
		ret = databinchk('z', Buffer, Grow_incr, Woffset, &errmsg);
	else if (Pattern == PATTERN_ONES)
		ret = databinchk('o', Buffer, Grow_incr, Woffset, &errmsg);
	else
		ret = dataasciichk(NULL, Buffer, Grow_incr, Woffset, &errmsg);

	if (ret >= 0) {
		fprintf(stderr, "%s%s: %d %s/%d: %d CW %s in file %s\n",
			Progname, TagName, Pid, __FILE__, __LINE__, Iter_cnt,
			errmsg, filename);

		if (Debug > 0)
			printf("%s%s: %d DEBUG1 %s/%d: **fd:%d, lk:%d, "
			       "offset:%ld, sz:%d open flags:%#o %s\n",
			       Progname, TagName, Pid, __FILE__, __LINE__, fd,
			       lockfile, (long)Woffset, Grow_incr,
			       Fileinfo.openflags,
			       openflags2symbols(Fileinfo.openflags, ",", 0));

		fflush(stderr);
		return 1;
	}

	if (Debug > 6)
		printf("%s: %d DEBUG7 %s/%d: No corruption detected on "
		       "write validation , offset = %ld, size = %d\n",
		       Progname, Pid, __FILE__, __LINE__, (long)Woffset,
		       Grow_incr);

	return 0;		/* all is well */
}

/***********************************************************************
 *
 ***********************************************************************/
int check_file(int fd, int cf_inter, char *filename, int no_file_check)
{
	int fsize;
	static int cf_count = 0;
	char *buf;
	int ret;
	int ret_val = 0;
	int rd_cnt;
	int rd_size;
	char *errmsg;

	cf_count++;

	if (cf_inter == 0 || (cf_count % cf_inter != 0)) {
		if (Debug > 4)
			printf
			    ("%s: %d DEBUG5 %s/%d: No file check - not time, iter=%d, cnt=%d\n",
			     Progname, Pid, __FILE__, __LINE__, cf_inter,
			     cf_count);
		return 0;	/* no check done */
	}

	/*
	 * if we can't determine file content, don't bother checking
	 */
	if (no_file_check) {
		if (Debug > 4)
			printf
			    ("%s: %d DEBUG5 %s/%d: No file check, lseek grow or random lseeks\n",
			     Progname, Pid, __FILE__, __LINE__);
		return 0;
	}

	/*
	 * Lock the file.  We need to have the file lock before
	 * the stat and until after the last read to prevent
	 * a trunc/truncate from "corrupting" our data.
	 */
	lkfile(fd, LOCK_SH, LKLVL0);

	if ((fsize = file_size(fd)) == -1) {
		lkfile(fd, LOCK_UN, LKLVL0);
		return -1;
	}

	if (fsize == 0) {
		if (Debug > 2)
			printf
			    ("%s: %d DEBUG3 %s/%d: No file validation, file size == 0\n",
			     Progname, Pid, __FILE__, __LINE__);

		lkfile(fd, LOCK_UN, LKLVL0);
		return 0;
	}

	if (Debug > 2)
		printf("%s: %d DEBUG3 %s/%d: about to do file validation\n",
		       Progname, Pid, __FILE__, __LINE__);

	if (fsize > MAX_FC_READ) {
		/*
		 * read the file in MAX_FC_READ chuncks.
		 */

		if ((buf = malloc(MAX_FC_READ)) == NULL) {
			fprintf(stderr, "%s%s: %s/%d: malloc(%d) failed: %s\n",
				Progname, TagName, __FILE__, __LINE__,
				MAX_FC_READ, strerror(errno));
			lkfile(fd, LOCK_UN, LKLVL0);
			return -1;
		}

		lseek(fd, 0, SEEK_SET);

		lkfile(fd, LOCK_SH, LKLVL0);	/* get lock on file before getting file size */

		rd_cnt = 0;
		while (rd_cnt < fsize) {
			rd_size = MIN(MAX_FC_READ, fsize - rd_cnt);

#if NEWIO
			ret = lio_read_buffer(fd, io_type, buf, rd_size,
					      SIGUSR1, &errmsg, 0);
#else
			ret =
			    read_buffer(fd, io_type, buf, rd_size, 0, &errmsg);
#endif

			if (ret != rd_size) {
				fprintf(stderr, "%s%s: %d %s/%d: %d CFa %s\n",
					Progname, TagName, Pid, __FILE__,
					__LINE__, Iter_cnt, errmsg);
				free(buf);
				lkfile(fd, LOCK_UN, LKLVL0);
				return -1;
			}
/**
	        read(fd, buf, rd_size);
***/

			if (Pattern == PATTERN_OFFSET)
				ret =
				    datapidchk(STATIC_NUM, buf, rd_size, rd_cnt,
					       &errmsg);
			else if (Pattern == PATTERN_PID)
				ret =
				    datapidchk(Pid, buf, rd_size, rd_cnt,
					       &errmsg);
			else if (Pattern == PATTERN_ASCII)
				ret =
				    dataasciichk(NULL, buf, rd_size, rd_cnt,
						 &errmsg);
			else if (Pattern == PATTERN_RANDOM) ;	/* no checks for random */
			else if (Pattern == PATTERN_ALT)
				ret =
				    databinchk('a', buf, rd_size, rd_cnt,
					       &errmsg);
			else if (Pattern == PATTERN_CHKER)
				ret =
				    databinchk('c', buf, rd_size, rd_cnt,
					       &errmsg);
			else if (Pattern == PATTERN_CNTING)
				ret =
				    databinchk('C', buf, rd_size, rd_cnt,
					       &errmsg);
			else if (Pattern == PATTERN_ZEROS)
				ret =
				    databinchk('z', buf, rd_size, rd_cnt,
					       &errmsg);
			else if (Pattern == PATTERN_ONES)
				ret =
				    databinchk('o', buf, rd_size, rd_cnt,
					       &errmsg);
			else
				ret =
				    dataasciichk(NULL, buf, rd_size, rd_cnt,
						 &errmsg);

			if (ret >= 0) {
				fprintf(stderr,
					"%s%s: %d %s/%d: %d CFp %s in file %s\n",
					Progname, TagName, Pid, __FILE__,
					__LINE__, Iter_cnt, errmsg, filename);
				fflush(stderr);
				ret_val = 1;
				lkfile(fd, LOCK_UN, LKLVL0);
				break;
			}
			rd_cnt += rd_size;
		}

		lkfile(fd, LOCK_UN, LKLVL0);

		free(buf);

	} else {
		/*
		 * Read the whole file in a single read
		 */
		if ((buf = malloc(fsize)) == NULL) {
			fprintf(stderr, "%s%s: %s/%d: malloc(%d) failed: %s\n",
				Progname, TagName, __FILE__, __LINE__, fsize,
				strerror(errno));
			fflush(stderr);
			return -1;
		}

		lseek(fd, 0, SEEK_SET);

/****
	    read(fd, buf, fsize);
****/
#if NEWIO
		ret =
		    lio_read_buffer(fd, io_type, buf, fsize, SIGUSR1, &errmsg,
				    0);
#else
		ret = read_buffer(fd, io_type, buf, fsize, 0, &errmsg);
#endif

		/* unlock the file as soon as we can */
		lkfile(fd, LOCK_UN, LKLVL0);

		if (ret != fsize) {
			fprintf(stderr, "%s%s: %d %s/%d: %d CFw %s\n",
				Progname, TagName, Pid, __FILE__, __LINE__,
				Iter_cnt, errmsg);
			ret_val = 1;
		} else {
			if (Pattern == PATTERN_OFFSET)
				ret =
				    datapidchk(STATIC_NUM, buf, fsize, 0,
					       &errmsg);
			else if (Pattern == PATTERN_PID)
				ret = datapidchk(Pid, buf, fsize, 0, &errmsg);
			else if (Pattern == PATTERN_ASCII)
				ret =
				    dataasciichk(NULL, buf, fsize, 0, &errmsg);
			else if (Pattern == PATTERN_RANDOM) ;	/* no check for random */
			else if (Pattern == PATTERN_ALT)
				ret = databinchk('a', buf, fsize, 0, &errmsg);
			else if (Pattern == PATTERN_CHKER)
				ret = databinchk('c', buf, fsize, 0, &errmsg);
			else if (Pattern == PATTERN_CNTING)
				ret = databinchk('C', buf, fsize, 0, &errmsg);
			else if (Pattern == PATTERN_ZEROS)
				ret = databinchk('z', buf, fsize, 0, &errmsg);
			else if (Pattern == PATTERN_ONES)
				ret = databinchk('o', buf, fsize, 0, &errmsg);
			else
				ret =
				    dataasciichk(NULL, buf, fsize, 0, &errmsg);

			if (ret >= 0) {
				fprintf(stderr,
					"%s%s: %d %s/%d: %d CFw %s in file %s\n",
					Progname, TagName, Pid, __FILE__,
					__LINE__, Iter_cnt, errmsg, filename);
				fflush(stderr);
				ret_val = 1;
			}
		}
		free(buf);
	}

	return ret_val;

}				/* end of check_file */

/***********************************************************************
 *
 ***********************************************************************/
int file_size(int fd)
{
	struct stat sb;

	if (fstat(fd, &sb) < 0) {
		fprintf(stderr,
			"%s%s: %d %s/%d: Unable to fstat(%d, &buf), errno:%d %s\n",
			Progname, TagName, Pid, __FILE__, __LINE__, fd, errno,
			strerror(errno));
		return -1;

	}

	return sb.st_size;
}

/***********************************************************************
 *  do file lock/unlock action.
 ***********************************************************************/
int lkfile(int fd, int operation, int lklevel)
{
	char *errmsg;

	if (lockfile == lklevel) {

		if (Debug > 5) {
			switch (operation) {
			case LOCK_UN:
				printf
				    ("%s: %d DEBUG6 %s/%d: Attempting to release lock on fd %d\n",
				     Progname, Pid, __FILE__, __LINE__, fd);
				break;

			case LOCK_SH:
				printf
				    ("%s: %d DEBUG6 %s/%d: Attempting to get read/shared lock on fd %d\n",
				     Progname, Pid, __FILE__, __LINE__, fd);
				break;

			case LOCK_EX:
				printf
				    ("%s: %d DEBUG6 %s/%d: Attempting to get write/exclusive lock on fd %d\n",
				     Progname, Pid, __FILE__, __LINE__, fd);
				break;
			}
		}

		/*
		 * Attempt to get/release desired lock.
		 * file_lock will attempt to do action over and over again until
		 * either an unretryable error or the action is completed.
		 */

		if (file_lock(fd, operation, &errmsg) != 0) {
			printf
			    ("%s%s: %d %s/%d: Unable to perform lock operation. %s\n",
			     Progname, TagName, Pid, __FILE__, __LINE__,
			     errmsg);

			/* do we count this as an error? handle_error();  */
			return -1;
		}

		if (Debug > 2) {
			switch (operation) {
			case LOCK_UN:
				printf
				    ("%s: %d DEBUG3 %s/%d: Released lock on fd %d\n",
				     Progname, Pid, __FILE__, __LINE__, fd);
				break;

			case LOCK_SH:
				printf
				    ("%s: %d DEBUG3 %s/%d: Got read/shared lock on fd %d\n",
				     Progname, Pid, __FILE__, __LINE__, fd);
				break;

			case LOCK_EX:
				printf
				    ("%s: %d DEBUG3 %s/%d: Got write/exclusive lock on fd %d\n",
				     Progname, Pid, __FILE__, __LINE__, fd);
				break;

			default:
				printf
				    ("%s: %d DEBUG3 %s/%d: Completed action %d on fd %d\n",
				     Progname, Pid, __FILE__, __LINE__,
				     operation, fd);
				break;
			}
		}
	}

	return 0;
}

#ifndef linux
/***********************************************************************
 *
 ***********************************************************************/
int pre_alloc(int fd, long size)
{

#ifdef CRAY
	long avl;

	if (ialloc(fd, size, IA_CONT, &avl) == -1) {
		fprintf(stderr,
			"%s%s %s/%d: Unable to pre-alloc space: ialloc failed: %d  %s\n",
			Progname, TagName, __FILE__, __LINE__, errno,
			strerror(errno));
		return -1;
	}
#endif

#ifdef sgi
	struct flock f;

	f.l_whence = 0;
	f.l_start = 0;
	f.l_len = size;

	/* non-zeroing reservation */
	if (fcntl(fd, F_RESVSP, &f) == -1) {
		fprintf(stderr,
			"%s%s %s/%d: Unable to pre-alloc space: fcntl(F_RESVSP) failed: %d  %s\n",
			Progname, TagName, __FILE__, __LINE__, errno,
			strerror(errno));
		return -1;
	}
#endif

	return 0;
}
#endif
