/*
 *   Copyright (C) Bull S.A. 1996
 *   Copyright (c) International Business Machines  Corp., 2001
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
/*---------------------------------------------------------------------+
|                           shmem_test_04                              |
| ==================================================================== |
|                                                                      |
| Description:  Test to verify mapping a file into memory (can also    |
|               be used as another anonymous shared memory test).      |
|                                                                      |
| Algorithm:    o  Create a file and map it into memory                |
|               o  Fork N processes                                    |
|               o  Each process writes ordered values to the memory    |
|                  segment and calculates a corresponding checksum     |
|               o  Each process then reads the values from the memory  |
|                  segment, checks the value and creates a comparison  |
|                  checksum                                            |
|               o  Verify checksums match                              |
|               o  Unmap file                                          |
|                                                                      |
| System calls: The following system calls are tested:                 |
|                                                                      |
|               mmap () - maps a file-system object into virtual       |
|                         memory                                       |
|               munmap () - unmaps a memory region                     |
|                                                                      |
| Usage:        shmem_test_04                                          |
|                                                                      |
| To Compile:   cc -O -o shmem_test_04 shmem_test_04.c                 |
|                                                                      |
| Last update:  Ver. 1.3, 1/30/94 00:40:27                            |
|                                                                      |
| Author:       Scott Porter (scott@austin.ibm.com)                    |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     070193  SLP   Initial version for AIX 3.2.5 VMM testing   |
|    0.2     011194  DJK   Modified for AIX 4.1 testing                |
|    1.2     020794  DJK   Moved to "prod" directory                   |
|                                                                      |
+---------------------------------------------------------------------*/

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <sys/mman.h>

#ifdef _LINUX_
typedef unsigned long ulong_t;
//#include <sys/except.h>
# ifndef PAGE_SIZE
#  define PAGE_SIZE 0x400
# endif
# ifndef SIGDANGER
#  define SIGDANGER 4
# endif
#else
# include <sys/m_except.h>
# include <sys/machine.h>
#endif

#ifdef _IA64
#include "dsi.h"
#endif



/* Defines
 *
 * USAGE: usage statement
 */
#define	TEMPDIR		"."
#define	TEMPNAME	"tmpfile"

#define MB		(1024*1024)
#define WPERMB		(MB/sizeof(int))

#define FILEOBJ		1
#define MEMOBJ		2

#define MAXPROCS	1000
#define USAGE	"\nUsage: %s [{-F | M}] [-l nloops] [-p nprocs] \n"	\
		"\t[{-m totmegs | -b totbytes}] [-v] [-W]\n\n"	\
		"\t-F          Map a file\n"	\
		"\t-M          Map annonymous memory\n"	\
		"\t-l nloops   Number of loops\n"	\
		"\t-p nprocs   Number of processes\n"	\
		"\t-m totmegs  Length in MB\n"	\
		"\t-b totbytes Length in bytes\n"	\
		"\t-v          Verbose\n"	\
		"\t-W          Allocate to pgsp warning level\n\n"

/*
 * Function prototypes
 *
 * parse_args (): Parse command line arguments
 * sys_error (): System error message function
 * error (): Error message function
 */
static int mkemptyfile (uint);
static void parse_args (int, char **);
static void cleanup (int);
static void setup_signal_handlers ();
static void int_handler (int signal);
static void segv_handler (int signal, int code, struct sigcontext *scp);
static void bus_handler (int signal, int code, struct sigcontext *scp);
static void sys_error (const char *, int);
static void error (const char *, int);


/*
 * Global variables
 *
 * *flg: command line parsing variables
 * filename:
 * fd:
 * nprocs:
 * childpid:
 */
static int bflg,lflg,mflg,pflg = 0;
static int Fflg,Mflg,Vflg,Wflg = 0;
char	*filename;
int	fd;
int	nprocs;
int	childpid [MAXPROCS];

int	nloops, objtype, pgspblks;
pid_t	parent_pid;
size_t	length;

/*---------------------------------------------------------------------+
|                               main                                   |
| ==================================================================== |
|                                                                      |
|                                                                      |
| Function:  Main program  (see prolog for more details)               |
|                                                                      |
| Returns:   (0)  Successful completion                                |
|            (-1) Error occurred                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int main (int argc, char **argv)
{
	caddr_t region;
	int *wptr;
	int rc;
	int word, nwords, addr, last, checksum, checksum2;
	int loop;
	pid_t pid;
	int proc;
	int fd;
	int map_flags;

	/*
	 * Parse command line arguments and print out program header
	 */
	parse_args (argc, argv);
	printf ("%s: IPC Shared Memory TestSuite program\n", *argv);

	setup_signal_handlers ();
	parent_pid = getpid ();

	/*
	 * Show options in effect.
	 */
	printf ("\n\tObject type to map = %s\n",
			 (objtype == MEMOBJ) ? "Anonymous memory" : "File");
	printf ("\tNumber of loops    = %d\n", nloops);
	printf ("\tNumber of procs    = %d\n", nprocs);
	printf ("\tBytes per process  = %ld (%ldMB)\n", (long)length, (long)length/MB);

	/*
	 * Determine the number of words for that size.
 	 */
	nwords = length / sizeof(int);

	/*
	 * Create shared memory mapping before forking
	 */
	if (objtype == FILEOBJ) {
		fd = mkemptyfile(length);
		map_flags = MAP_FILE;
	} else {
		fd = -1;
		map_flags = MAP_ANONYMOUS;
	}

	/*
	 * Map the object with the specified flags.
	 */
	if ((region = mmap(0, length, PROT_READ | PROT_WRITE,
				map_flags | MAP_SHARED, fd, 0))
		 == MAP_FAILED) {
		sys_error ("mmap failed", __LINE__);
	}

	/*
	 * Fork off the additional processes.
	 */
	for (proc = 1; proc < nprocs; proc++) {
		/*
		 * Child leaves loop, parent continues to fork.
		 */
		if ((pid = fork()) < (pid_t)0) {
			sys_error ("fork failed\n", __LINE__);
		} else if (pid == 0)
			break;
		else
			childpid[proc] = pid;
	}

	pid = getpid();

	/*
	 * Initialize each word in the region with a unique value.
	 */
        checksum = 0;
        for (word = 0, wptr = (int *)region; word < nwords; word++, wptr++) {
		if (Vflg) {
			if (word && word % WPERMB == 0)
				printf ("\t[%d] %ldMB initialized...\n",
					pid, (long)word/WPERMB);
		}
		*wptr = word;
                checksum += word;
        }
	if (Vflg) {
		printf ("\t[%d] checksum = %d\n", pid, checksum);
	}

	for (loop = 1; loop <= nloops; loop++) {

	/*
	 * Read back each word in the region and check its value.
	 */
        checksum2 = 0;
        last = -1;
        for (word = 0, wptr = (int *)region; word < nwords; word++, wptr++) {
		if (Vflg) {
			if (word && word % WPERMB == 0)
				printf ("\t[%d][%d] %ldMB verified...\n", pid, loop,
					(long)word/WPERMB);
		}
                if (*wptr != word) {
                        addr = ((intptr_t)wptr & 0x0fffffff)/4096;
                        if (addr != last) {
                                last = addr;
				if (Vflg) {
					printf ("\t[%d][%d] page in error at addr = %d\n",
						pid, loop, addr);
				}
                        }
			if (Vflg) {
				printf ("\t[%d][%d] Word = %d, Value = %d\n",
					pid, loop, word, *wptr);
			}
                }
                checksum2 += *wptr;
        }
	if (Vflg) {
		printf ("\t[%d][%d] checksum2 = %d\n", pid, loop, checksum2);
	}

	if (checksum2 == checksum) {
		if (Vflg) {
			printf ("\t[%d][%d] Check sums compare\n", pid, loop);
		}
	} else {
		if (Vflg) {
			printf ("\t[%d][%d] Check sums DON'T compare\n", pid, loop);
		}
	}

	} /* end loop */

	/*
	 * Unmap the region.
	 */
	if ((rc = munmap(region, length)) != 0) {
		sys_error ("munmap failed", __LINE__);
	}
	/*
	 * Program completed successfully -- exit
	 */
	if (pid != parent_pid) exit (0);

	printf ("\nsuccessful!\n");
	cleanup(0);

	return 0;
	/*NOTREACHED*/
}

static void cleanup (int rc)
{
	int i;

	/*
	 * Close and remove any file we've created.
	 */
	if (Fflg) {
		close (fd);
		unlink (filename);
	}

	/*
	 * Kill any child processes we've started.
	 */
	if (rc) {
		for (i = 1; i < nprocs; i++) {
			kill (childpid[i], SIGKILL);
		}
	}

	exit(rc);
}

/*
 * segv_handler - signal handler for SIGSEGV
 */
static void segv_handler (int signal, int code, struct sigcontext *scp)
{
#ifndef _LINUX_
	int except;
	ulong_t dar, dsisr;
	int rc;

	/*
	 * Get the exception type.  This is either an errno value
	 * or an exception value from <sys/m_except.h>.
	 */
# ifdef _IA64
	except  = scp->sc_context.__excp_type;
# else
	except	= scp->sc_jmpbuf.jmp_context.excp_type;
# endif

	/*
	 * Get the Data Address Register and Interrupt Status Register
	 * for the exception.
	 */
# ifdef _IA64
	dar     = scp->sc_context.__ifa;
	dsisr   = scp->sc_context.__isr;
# else
	dar	= scp->sc_jmpbuf.jmp_context.except[0];
	dsisr	= scp->sc_jmpbuf.jmp_context.except[1];
# endif

	printf ("SIGSEGV occurred on address 0x%08x.\n", dar);

	/*
	 * Determine if the operation was a load or a store.
	 * Definition of bits in DSISR are in <sys/machine.h>.
	 */
	if (dsisr & DSISR_ST) {
		printf ("The operation was a store.\n");
	} else {
		printf ("The operation was a load.\n");
	}

	switch (except) {

	case EFAULT:
		printf ("Exception was due to a bad address.\n");
		break;

	case EXCEPT_PROT:
		printf ("Exception was due to a protection violation.\n");
		break;

	default:
		printf ("Exception type = 0x%08x.\n", except);
	}
#else
	printf("An unexpected segmentation fault occurred... Exiting\n");
#endif

	cleanup(1);
}

/*
 * bus_handler - signal handler for SIGBUS
 */
static void bus_handler (int signal, int code, struct sigcontext *scp)
{
#ifndef _LINUX_
	int except;
	ulong_t dar, dsisr;
	int rc;

	/*
	 * Get the exception type.  This is either an errno value
	 * or an exception value from <sys/m_except.h>.
	 */
# ifdef _IA64
	except  = scp->sc_context.__excp_type;
# else
	except	= scp->sc_jmpbuf.jmp_context.excp_type;
# endif

	/*
	 * Get the Data Address Register and Interrupt Status Register
	 * for the exception.
	 */
# ifdef _IA64
	dar     = scp->sc_context.__ifa;
	dsisr   = scp->sc_context.__isr;
# else
	dar	= scp->sc_jmpbuf.jmp_context.except[0];
	dsisr	= scp->sc_jmpbuf.jmp_context.except[1];
# endif

	printf ("SIGBUS occurred on address 0x%08x:\n", dar);

	switch (except) {

	case ENOSPC:
		printf ("A mapper fault required disk allocation and \
			no space is left on the device.\n");
		break;

	case EDQUOT:
		printf ("A mapper fault required disk allocation and \
			the disc quota was exceeded.\n");
		break;

	case EXCEPT_EOF:
		printf ("An mmap() mapper referenced beyond end-of-file.\n");
		break;

	default:
		printf ("Exception type = 0x%08x.\n", except);
	}
#else
	printf("An unexpected bus error occurred... Exiting\n");
#endif

	cleanup(1);
}

/*
 * int_handler - signal handler for SIGINT
 */
static void int_handler (int sig)
{
	cleanup(1);
}

/*
 * mkemptyfile()
 *
 * Make an empty temporary file of a given size and return its descriptor.
 */
static int mkemptyfile (uint size)
{
#ifdef _LINUX_

	filename = (char *)malloc(256);

	sprintf(filename, "%s/%sXXXXXX", TEMPDIR, TEMPNAME);
	fd = mkstemp(filename);

#else
	/* Get a new file name
	 */
	filename = tempnam(TEMPDIR, TEMPNAME);

	/* Create the file.
	 * O_EXCL:	I'm supposed to be getting a unique name so if
	 *		a file already exists by this name then fail.
	 * 0700:	Set up for r/w access by owner only.
	 */
	if ((fd = open(filename, O_CREAT | O_EXCL | O_RDWR, 0700)) == -1)
		return(-1);

#endif // _LINUX_

	/* Now extend it to the requested length.
	 */
	if (lseek(fd, size-1, SEEK_SET) == -1)
		return(-1);

	if (write(fd, "\0", 1) == -1)
		return(-1);

	/* Sync the file out.
	 */
	fsync(fd);

	return(fd);
}

/*---------------------------------------------------------------------+
|                          setup_signal_handlers                       |
| ==================================================================== |
|                                                                      |
| Function:  Sets up signal handlers for following signals:            |
|                                                                      |
|            SIGINT  - interrupt, generated from terminal spec. char   |
|            SIGBUS  - bus error (specification execption)             |
|            SIGSEGV - segmentation violation                          |
|                                                                      |
+---------------------------------------------------------------------*/
static void setup_signal_handlers ()
{
	struct sigaction sigact;

	sigact.sa_flags = 0;
	sigfillset (&sigact.sa_mask);

	/*
	 * Establish the signal handlers for SIGSEGV, SIGBUS & SIGINT
	 */
	sigact.sa_handler = (void (*)(int)) segv_handler;
	if (sigaction (SIGSEGV, &sigact, NULL) < 0)
		sys_error ("sigaction failed", __LINE__);

	sigact.sa_handler = (void (*)(int)) bus_handler;
	if (sigaction (SIGBUS, &sigact, NULL) < 0)
		sys_error ("sigaction failed", __LINE__);

	sigact.sa_handler = (void (*)(int)) int_handler;
	if (sigaction (SIGINT, &sigact, NULL) < 0)
		sys_error ("sigaction failed", __LINE__);
}


/*---------------------------------------------------------------------+
|                             parse_args ()                            |
| ==================================================================== |
|                                                                      |
| Function:  Parse the command line arguments & initialize global      |
|            variables.                                                |
|                                                                      |
| Updates:   (command line options)                                    |
|                                                                      |
|            [-s] size: shared memory segment size                     |
|                                                                      |
+---------------------------------------------------------------------*/
static void parse_args (int argc, char **argv)
{
	int	opt;
	int 	bytes = 0, megabytes = 0;
	int	errflag = 0;
	char	*program_name = *argv;
	extern char 	*optarg;	/* Command line option */

	/*
	 * Parse command line options.
	 */
	while ((opt = getopt(argc, argv, "DFMWvb:l:m:p:")) != EOF)
	{
		switch (opt)
		{
		case 'F':	/* map a file */
			Fflg++;
			break;
		case 'M':	/* map anonymous memory */
			Mflg++;
			break;
		case 'v':	/* verbose */
			Vflg++;
			break;
		case 'W':	// allocate to pgsp warning level
			Wflg++;
			break;
		case 'b':	/* length in bytes */
			bflg++;
			bytes = atoi(optarg);
			break;
		case 'l':	/* number of loops */
			lflg++;
			nloops = atoi(optarg);
			break;
		case 'm':	/* length in MB */
			mflg++;
			megabytes = atoi(optarg);
			break;
		case 'p':	/* number of processes */
			pflg++;
			nprocs = atoi(optarg);
			break;
		default:
			errflag++;
			break;
		}
	}
	if (errflag) {
		fprintf (stderr, USAGE, program_name);
		exit (2);
	}

	/*
	 * Determine the number of processes to run.
 	 */
	if (pflg) {
		if (nprocs > MAXPROCS)
			nprocs = MAXPROCS;
	} else {
		nprocs = 1;
	}

	/*
	 * Determine the type of object to map.
 	 */
	if (Fflg) {
		objtype = FILEOBJ;
	} else if (Mflg) {
		objtype = MEMOBJ;
	} else {
		objtype = MEMOBJ;
	}

	/*
	 * The 'W' flag is used to determine the size of an anonymous
	 * region that will use the amount of paging space available
	 * close to the paging space warning level.
	 */

        if (Wflg)
#ifdef _LINUX_
	        printf("Option 'W' not implemented in Linux (psdanger() and SIGDANGER)\n");
#else
	{
		pgspblks = psdanger(SIGDANGER);
		pgspblks -= 256;	// leave a little room
		if (pgspblks < 0)
			pgspblks = 0;
		bytes = pgspblks * PAGE_SIZE;
		bflg = 1;
		mflg = 0;
	}
#endif

	/*
	 * Determine size of region to map in bytes for each process.
	 */
	if (mflg) {
		length = megabytes * MB;
	} else if (bflg) {
		length = bytes;
	} else {
		length = 16 * MB;
	}

	/*
	 * Set default loops.
	 */
	if (!lflg) {
		nloops = 1;
	}
}



/*---------------------------------------------------------------------+
|                             sys_error ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Creates system error message and calls error ()           |
|                                                                      |
+---------------------------------------------------------------------*/
static void sys_error (const char *msg, int line)
{
	char syserr_msg [256];

	sprintf (syserr_msg, "%s: %s\n", msg, strerror (errno));
	error (syserr_msg, line);
}


/*---------------------------------------------------------------------+
|                               error ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Prints out message and exits...                           |
|                                                                      |
+---------------------------------------------------------------------*/
static void error (const char *msg, int line)
{
	fprintf (stderr, "ERROR [line: %d] %s\n", line, msg);
	cleanup (1);
}
