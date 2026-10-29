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
 * doio -	a general purpose io initiator with system call and
 *		write logging.  See doio.h for the structure which defines
 *		what doio requests should look like.
 *
 *		Currently doio can handle read,write,reada,writea,ssread,
 *		sswrite, and many varieties of listio requests.
 *		For disk io, if the O_SSD flag is set doio will allocate
 *		the appropriate amount of ssd and do the transfer - thus, doio
 *		can handle all of the primitive types of file io.
 *
 * programming
 * notes:
 * -----------
 *	messages should generally be printed using doio_fprintf().
 *
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#ifdef CRAY
#include <sys/iosw.h>
#endif
#ifdef sgi
#include <aio.h>		/* for aio_read,write */
#include <inttypes.h>		/* for uint64_t type */
#include <siginfo.h>		/* signal handlers & SA_SIGINFO */
#endif
#ifndef CRAY
#include <sys/uio.h>		/* for struct iovec (readv) */
#include <sys/mman.h>		/* for mmap(2) */
#include <sys/ipc.h>		/* for i/o buffer in shared memory */
#include <sys/shm.h>		/* for i/o buffer in shared memory */
#endif
#include <sys/wait.h>
#ifdef CRAY
#include <sys/listio.h>
#include <sys/panic.h>
#endif
#include <sys/time.h>		/* for delays */

#include "doio.h"
#include "write_log.h"
#include "tso_random_range.h"
#include "string_to_tokens.h"
#include "pattern.h"

#define	NMEMALLOC	32
#define	MEM_DATA	1	/* data space                           */
#define	MEM_SHMEM	2	/* System V shared memory               */
#define	MEM_T3ESHMEM	3	/* T3E Shared Memory                    */
#define	MEM_MMAP	4	/* mmap(2)                              */

#define	MEMF_PRIVATE	0001
#define	MEMF_AUTORESRV	0002
#define	MEMF_LOCAL	0004
#define	MEMF_SHARED	0010

#define	MEMF_FIXADDR	0100
#define	MEMF_ADDR	0200
#define	MEMF_AUTOGROW	0400
#define	MEMF_FILE	01000	/* regular file -- unlink on close      */
#define	MEMF_MPIN	010000	/* use mpin(2) to lock pages in memory */

struct memalloc {
	int memtype;
	int flags;
	int nblks;
	char *name;
	void *space;		/* memory address of allocated space */
	int fd;			/* FD open for mmaping */
	int size;
} Memalloc[NMEMALLOC];

/*
 * Structure for maintaining open file test descriptors.  Used by
 * alloc_fd().
 */

struct fd_cache {
	char c_file[MAX_FNAME_LENGTH + 1];
	int c_oflags;
	int c_fd;
	long c_rtc;
#ifdef sgi
	int c_memalign;		/* from F_DIOINFO */
	int c_miniosz;
	int c_maxiosz;
#endif
#ifndef CRAY
	void *c_memaddr;	/* mmapped address */
	int c_memlen;		/* length of above region */
#endif
};

/*
 * Name-To-Value map
 * Used to map cmdline arguments to values
 */
struct smap {
	char *string;
	int value;
};

struct aio_info {
	int busy;
	int id;
	int fd;
	int strategy;
	volatile int done;
#ifdef CRAY
	struct iosw iosw;
#endif
#ifdef sgi
	aiocb_t aiocb;
	int aio_ret;		/* from aio_return */
	int aio_errno;		/* from aio_error */
#endif
	int sig;
	int signalled;
	struct sigaction osa;
};

/* ---------------------------------------------------------------------------
 *
 * A new paradigm of doing the r/w system call where there is a "stub"
 * function that builds the info for the system call, then does the system
 * call; this is called by code that is common to all system calls and does
 * the syscall return checking, async I/O wait, iosw check, etc.
 *
 * Flags:
 *	WRITE, ASYNC, SSD/SDS,
 *	FILE_LOCK, WRITE_LOG, VERIFY_DATA,
 */

struct status {
	int rval;		/* syscall return */
	int err;		/* errno */
	int *aioid;		/* list of async I/O structures */
};

struct syscall_info {
	char *sy_name;
	int sy_type;
	struct status *(*sy_syscall) ();
	int (*sy_buffer) ();
	char *(*sy_format) ();
	int sy_flags;
	int sy_bits;
};

#define	SY_WRITE		00001
#define	SY_ASYNC		00010
#define	SY_IOSW			00020
#define	SY_SDS			00100

#ifndef O_SSD
#define O_SSD 0			/* so code compiles on a CRAY2 */
#endif

#ifdef sgi
#define UINT64_T uint64_t
#else
#define UINT64_T unsigned long
#endif

#ifndef O_PARALLEL
#define O_PARALLEL 0		/* so O_PARALLEL may be used in expressions */
#endif

#define PPID_CHECK_INTERVAL 5	/* check ppid every <-- iterations */
#define	MAX_AIO		256	/* maximum number of async I/O ops */
#ifdef _CRAYMPP
#define	MPP_BUMP	16	/* page un-alignment for MPP */
#else
#define	MPP_BUMP	0
#endif

#define	SYSERR strerror(errno)

/*
 * getopt() string of supported cmdline arguments.
 */

#define OPTS	"aC:d:ehm:n:kr:w:vU:V:M:N:"

#define DEF_RELEASE_INTERVAL	0

/*
 * Flags set in parse_cmdline() to indicate which options were selected
 * on the cmdline.
 */

int a_opt = 0;			/* abort on data compare errors     */
int e_opt = 0;			/* exec() after fork()'ing          */
int C_opt = 0;			/* Data Check Type                  */
int d_opt = 0;			/* delay between operations         */
int k_opt = 0;			/* lock file regions during writes  */
int m_opt = 0;			/* generate periodic messages       */
int n_opt = 0;			/* nprocs                           */
int r_opt = 0;			/* resource release interval        */
int w_opt = 0;			/* file write log file              */
int v_opt = 0;			/* verify writes if set             */
int U_opt = 0;			/* upanic() on varios conditions    */
int V_opt = 0;			/* over-ride default validation fd type */
int M_opt = 0;			/* data buffer allocation types     */
char TagName[40];		/* name of this doio (see Monster)  */

/*
 * Misc globals initialized in parse_cmdline()
 */

char *Prog = NULL;		/* set up in parse_cmdline()                */
int Upanic_Conditions;		/* set by args to -U                        */
int Release_Interval;		/* arg to -r                                */
int Nprocs;			/* arg to -n                                */
char *Write_Log;		/* arg to -w                                */
char *Infile;			/* input file (defaults to stdin)           */
int *Children;			/* pids of child procs                      */
int Nchildren = 0;
int Nsiblings = 0;		/* tfork'ed siblings                        */
int Execd = 0;
int Message_Interval = 0;
int Npes = 0;			/* non-zero if built as an mpp multi-pe app */
int Vpe = -1;			/* Virtual pe number if Npes >= 0           */
int Reqno = 1;			/* request # - used in some error messages  */
int Reqskipcnt = 0;		/* count of I/O requests that are skipped   */
int Validation_Flags;
char *(*Data_Check) ();		/* function to call for data checking       */
int (*Data_Fill) ();		/* function to call for data filling        */
int Nmemalloc = 0;		/* number of memory allocation strategies   */
int delayop = 0;		/* delay between operations - type of delay */
int delaytime = 0;		/* delay between operations - how long      */

struct wlog_file Wlog;

int active_mmap_rw = 0;		/* Indicates that mmapped I/O is occurring. */
			    /* Used by sigbus_action() in the child doio. */
int havesigint = 0;

#define SKIP_REQ	-2	/* skip I/O request */

/*
 * Global file descriptors
 */

int Wfd_Append;			/* for appending to the write-log       */
int Wfd_Random;			/* for overlaying write-log entries     */

#define FD_ALLOC_INCR	32	/* allocate this many fd_map structs    */
				/* at a time */

/*
 * Globals for tracking Sds and Core usage
 */

char *Memptr;			/* ptr to core buffer space             */
int Memsize;			/* # bytes pointed to by Memptr         */
				/* maintained by alloc_mem()            */

int Sdsptr;			/* sds offset (always 0)                */
int Sdssize;			/* # bytes of allocated sds space       */
				/* Maintained by alloc_sds()            */
char Host[16];
char Pattern[128];
int Pattern_Length;

/*
 * Signal handlers, and related globals
 */

char *syserrno(int err);
void doio(void);
void doio_delay(void);
char *format_oflags(int oflags);
char *format_strat(int strategy);
char *format_rw(struct io_req *ioreq, int fd, void *buffer,
		int signo, char *pattern, void *iosw);
#ifdef CRAY
char *format_sds(struct io_req *ioreq, void *buffer, int sds char *pattern);
#endif /* CRAY */

int do_read(struct io_req *req);
int do_write(struct io_req *req);
int lock_file_region(char *fname, int fd, int type, int start, int nbytes);

#ifdef CRAY
char *format_listio(struct io_req *ioreq, int lcmd,
		    struct listreq *list, int nent, int fd, char *pattern);
#endif /* CRAY */

int do_listio(struct io_req *req);

#if defined(_CRAY1) || defined(CRAY)
int do_ssdio(struct io_req *req);
#endif /* defined(_CRAY1) || defined(CRAY) */

char *fmt_ioreq(struct io_req *ioreq, struct syscall_info *sy, int fd);

#ifdef CRAY
struct status *sy_listio(struct io_req *req, struct syscall_info *sysc,
			 int fd, char *addr);
int listio_mem(struct io_req *req, int offset, int fmstride,
	       int *min, int *max);
char *fmt_listio(struct io_req *req, struct syscall_info *sy,
		 int fd, char *addr);
#endif /* CRAY */

#ifdef sgi
struct status *sy_pread(struct io_req *req, struct syscall_info *sysc,
			int fd, char *addr);
struct status *sy_pwrite(struct io_req *req, struct syscall_info *sysc,
			 int fd, char *addr);
char *fmt_pread(struct io_req *req, struct syscall_info *sy,
		int fd, char *addr);
#endif /* sgi */

#ifndef CRAY
struct status *sy_readv(struct io_req *req, struct syscall_info *sysc,
			int fd, char *addr);
struct status *sy_writev(struct io_req *req, struct syscall_info *sysc,
			 int fd, char *addr);
struct status *sy_rwv(struct io_req *req, struct syscall_info *sysc,
		      int fd, char *addr, int rw);
char *fmt_readv(struct io_req *req, struct syscall_info *sy,
		int fd, char *addr);
#endif /* !CRAY */

#ifdef sgi
struct status *sy_aread(struct io_req *req, struct syscall_info *sysc,
			int fd, char *addr);
struct status *sy_awrite(struct io_req *req, struct syscall_info *sysc,
			 int fd, char *addr)
struct status *sy_arw(struct io_req *req, struct syscall_info *sysc,
		      int fd, char *addr, int rw);
char *fmt_aread(struct io_req *req, struct syscall_info *sy,
		int fd, char *addr);
#endif /* sgi */

#ifndef CRAY
struct status *sy_mmread(struct io_req *req, struct syscall_info *sysc,
			 int fd, char *addr);
struct status *sy_mmwrite(struct io_req *req, struct syscall_info *sysc,
			  int fd, char *addr);
struct status *sy_mmrw(struct io_req *req, struct syscall_info *sysc,
		       int fd, char *addr, int rw);
char *fmt_mmrw(struct io_req *req, struct syscall_info *sy, int fd, char *addr);
#endif /* !CRAY */

int do_rw(struct io_req *req);

#ifdef sgi
int do_fcntl(struct io_req *req);
#endif /* sgi */

#ifndef CRAY
int do_sync(struct io_req *req);
#endif /* !CRAY */

int doio_pat_fill(char *addr, int mem_needed, char *Pattern,
		  int Pattern_Length, int shift);
char *doio_pat_check(char *buf, int offset, int length,
		     char *pattern, int pattern_length, int patshift);
char *check_file(char *file, int offset, int length, char *pattern,
		 int pattern_length, int patshift, int fsa);
int doio_fprintf(FILE * stream, char *format, ...);
int alloc_mem(int nbytes);

#if defined(_CRAY1) || defined(CRAY)
int alloc_sds(int nbytes);
#endif /* defined(_CRAY1) || defined(CRAY) */

int alloc_fd(char *file, int oflags);
struct fd_cache *alloc_fdcache(char *file, int oflags);

#ifdef sgi
void signal_info(int sig, siginfo_t * info, void *v);
void cleanup_handler(int sig, siginfo_t * info, void *v);
void die_handler(int sig, siginfo_t * info, void *v);
void sigbus_handler(int sig, siginfo_t * info, void *v);
#else /* !sgi */
void cleanup_handler(int sig);
void die_handler(int sig);

#ifndef CRAY
void sigbus_handler(int sig);
#endif /* !CRAY */
#endif /* sgi */

void noop_handler(int sig);
void sigint_handler(int sig);
void aio_handler(int sig);
void dump_aio(void);

#ifdef sgi
void cb_handler(sigval_t val);
#endif /* sgi */

struct aio_info *aio_slot(int aio_id);
int aio_register(int fd, int strategy, int sig);
int aio_unregister(int aio_id);

#ifndef __linux__
int aio_wait(int aio_id);
#endif /* !__linux__ */

char *hms(time_t t);
int aio_done(struct aio_info *ainfo);
void doio_upanic(int mask);
int parse_cmdline(int argc, char **argv, char *opts);

#ifndef CRAY
void parse_memalloc(char *arg);
void dump_memalloc(void);
#endif /* !CRAY */

void parse_delay(char *arg);
int usage(FILE * stream);
void help(FILE * stream);

/*
 * Upanic conditions, and a map from symbolics to values
 */

#define U_CORRUPTION	0001	/* upanic on data corruption    */
#define U_IOSW	    	0002	/* upanic on bad iosw           */
#define U_RVAL	    	0004	/* upanic on bad rval           */

#define U_ALL	    	(U_CORRUPTION | U_IOSW | U_RVAL)

struct smap Upanic_Args[] = {
	{"corruption", U_CORRUPTION},
	{"iosw", U_IOSW},
	{"rval", U_RVAL},
	{"all", U_ALL},
	{NULL, 0}
};

struct aio_info Aio_Info[MAX_AIO];

/* -C data-fill/check type */
#define	C_DEFAULT	1
struct smap checkmap[] = {
	{"default", C_DEFAULT},
	{NULL, 0},
};

/* -d option delay types */
#define	DELAY_SELECT	1
#define	DELAY_SLEEP	2
#define	DELAY_SGINAP	3
#define	DELAY_ALARM	4
#define	DELAY_ITIMER	5	/* POSIX timer                          */

struct smap delaymap[] = {
	{"select", DELAY_SELECT},
	{"sleep", DELAY_SLEEP},
#ifdef sgi
	{"sginap", DELAY_SGINAP},
#endif
	{"alarm", DELAY_ALARM},
	{NULL, 0},
};

/******
*
* strerror() does similar actions.

char *
syserrno(int err)
{
    static char sys_errno[10];
    sprintf(sys_errno, "%d", errno);
    return(sys_errno);
}

******/

int main(int argc, char **argv)
{
	int i, pid, stat, ex_stat;
#ifdef CRAY
	sigset_t omask;
#elif defined(linux)
	sigset_t omask, block_mask;
#else
	int omask;
#endif
	struct sigaction sa;

	umask(0);		/* force new file modes to known values */
#if _CRAYMPP
	Npes = sysconf(_SC_CRAY_NPES);	/* must do this before parse_cmdline */
	Vpe = sysconf(_SC_CRAY_VPE);
#endif

	TagName[0] = '\0';
	parse_cmdline(argc, argv, OPTS);

	random_range_seed(getpid());	/* initialize random number generator */

	/*
	 * If this is a re-exec of doio, jump directly into the doio function.
	 */

	if (Execd) {
		doio();
		exit(E_SETUP);
	}

	/*
	 * Stop on all but a few signals...
	 */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = sigint_handler;
	sa.sa_flags = SA_RESETHAND;	/* sigint is ignored after the */
	/* first time */
	for (i = 1; i <= NSIG; i++) {
		switch (i) {
#ifdef SIGRECOVERY
		case SIGRECOVERY:
			break;
#endif
#ifdef SIGCKPT
		case SIGCKPT:
#endif
#ifdef SIGRESTART
		case SIGRESTART:
#endif
		case SIGTSTP:
		case SIGSTOP:
		case SIGCONT:
		case SIGCHLD:
		case SIGBUS:
		case SIGSEGV:
		case SIGQUIT:
			break;
		default:
			sigaction(i, &sa, NULL);
		}
	}

	/*
	 * If we're logging write operations, make a dummy call to wlog_open
	 * to initialize the write history file.  This call must be done in
	 * the parent, to ensure that the history file exists and/or has
	 * been truncated before any children attempt to open it, as the doio
	 * children are not allowed to truncate the file.
	 */

	if (w_opt) {
		strcpy(Wlog.w_file, Write_Log);

		if (wlog_open(&Wlog, 1, 0666) < 0) {
			doio_fprintf(stderr,
				     "Could not create/truncate write log %s\n",
				     Write_Log);
			exit(2);
		}

		wlog_close(&Wlog);
	}

	/*
	 * Malloc space for the children pid array.  Initialize all entries
	 * to -1.
	 */

	Children = malloc(sizeof(int) * Nprocs);
	for (i = 0; i < Nprocs; i++) {
		Children[i] = -1;
	}

	sigemptyset(&block_mask);
	sigaddset(&block_mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &block_mask, &omask);

	/*
	 * Fork Nprocs.  This [parent] process is a watchdog, to notify the
	 * invoker of procs which exit abnormally, and to make sure that all
	 * child procs get cleaned up.  If the -e option was used, we will also
	 * re-exec.  This is mostly for unicos/mk on mpp's, to ensure that not
	 * all of the doio's don't end up in the same pe.
	 *
	 * Note - if Nprocs is 1, or this doio is a multi-pe app (Npes > 1),
	 * jump directly to doio().  multi-pe apps can't fork(), and there is
	 * no reason to fork() for 1 proc.
	 */

	if (Nprocs == 1 || Npes > 1) {
		doio();
		exit(0);
	} else {
		for (i = 0; i < Nprocs; i++) {
			if ((pid = fork()) == -1) {
				doio_fprintf(stderr,
					     "(parent) Could not fork %d children:  %s (%d)\n",
					     i + 1, SYSERR, errno);
				exit(E_SETUP);
			}

			Children[Nchildren] = pid;
			Nchildren++;

			if (pid == 0) {
				if (e_opt) {
					char *exec_path;

					exec_path = argv[0];
					argv[0] = malloc(strlen(exec_path) + 2);
					sprintf(argv[0], "-%s", exec_path);

					execvp(exec_path, argv);
					doio_fprintf(stderr,
						     "(parent) Could not execvp %s:  %s (%d)\n",
						     exec_path, SYSERR, errno);
					exit(E_SETUP);
				} else {
					doio();
					exit(E_SETUP);
				}
			}
		}

		/*
		 * Parent spins on wait(), until all children exit.
		 */

		ex_stat = E_NORMAL;

		while (Nprocs) {
			if ((pid = wait(&stat)) == -1) {
				if (errno == EINTR)
					continue;
			}

			for (i = 0; i < Nchildren; i++)
				if (Children[i] == pid)
					Children[i] = -1;

			Nprocs--;

			if (WIFEXITED(stat)) {
				switch (WEXITSTATUS(stat)) {
				case E_NORMAL:
					/* noop */
					break;

				case E_INTERNAL:
					doio_fprintf(stderr,
						     "(parent) pid %d exited because of an internal error\n",
						     pid);
					ex_stat |= E_INTERNAL;
					break;

				case E_SETUP:
					doio_fprintf(stderr,
						     "(parent) pid %d exited because of a setup error\n",
						     pid);
					ex_stat |= E_SETUP;
					break;

				case E_COMPARE:
					doio_fprintf(stderr,
						     "(parent) pid %d exited because of data compare errors\n",
						     pid);

					ex_stat |= E_COMPARE;

					if (a_opt)
						kill(0, SIGINT);

					break;

				case E_USAGE:
					doio_fprintf(stderr,
						     "(parent) pid %d exited because of a usage error\n",
						     pid);

					ex_stat |= E_USAGE;
					break;

				default:
					doio_fprintf(stderr,
						     "(parent) pid %d exited with unknown status %d\n",
						     pid, WEXITSTATUS(stat));
					ex_stat |= E_INTERNAL;
					break;
				}
			} else if (WIFSIGNALED(stat)
				   && WTERMSIG(stat) != SIGINT) {
				doio_fprintf(stderr,
					     "(parent) pid %d terminated by signal %d\n",
					     pid, WTERMSIG(stat));

				ex_stat |= E_SIGNAL;
			}

			fflush(NULL);
		}
	}

	exit(ex_stat);

}				/* main */

/*
 * main doio function.  Each doio child starts here, and never returns.
 */

void doio(void)
{
	int rval, i, infd, nbytes;
	char *cp;
	struct io_req ioreq;
	struct sigaction sa, def_action, ignore_action, exit_action;
#ifndef CRAY
	struct sigaction sigbus_action;
#endif

	Memsize = Sdssize = 0;

	/*
	 * Initialize the Pattern - write-type syscalls will replace Pattern[1]
	 * with the pattern passed in the request.  Make sure that
	 * strlen(Pattern) is not mod 16 so that out of order words will be
	 * detected.
	 */

	gethostname(Host, sizeof(Host));
	if ((cp = strchr(Host, '.')) != NULL)
		*cp = '\0';

	Pattern_Length = sprintf(Pattern, "-:%d:%s:%s*", getpid(), Host, Prog);

	if (!(Pattern_Length % 16)) {
		Pattern_Length = sprintf(Pattern, "-:%d:%s:%s**",
					 getpid(), Host, Prog);
	}

	/*
	 * Open a couple of descriptors for the write-log file.  One descriptor
	 * is for appending, one for random access.  Write logging is done for
	 * file corruption detection.  The program doio_check is capable of
	 * doing corruption detection based on a doio write-log.
	 */

	if (w_opt) {

		strcpy(Wlog.w_file, Write_Log);

		if (wlog_open(&Wlog, 0, 0666) == -1) {
			doio_fprintf(stderr,
				     "Could not open write log file (%s): wlog_open() failed\n",
				     Write_Log);
			exit(E_SETUP);
		}
	}

	/*
	 * Open the input stream - either a file or stdin
	 */

	if (Infile == NULL) {
		infd = 0;
	} else {
		if ((infd = open(Infile, O_RDWR)) == -1) {
			doio_fprintf(stderr,
				     "Could not open input file (%s):  %s (%d)\n",
				     Infile, SYSERR, errno);
			exit(E_SETUP);
		}
	}

	/*
	 * Define a set of signals that should never be masked.  Receipt of
	 * these signals generally indicates a programming error, and we want
	 * a corefile at the point of error.  We put SIGQUIT in this list so
	 * that ^\ will force a user core dump.
	 *
	 * Note:  the handler for these should be SIG_DFL, all of them
	 * produce a corefile as the default action.
	 */

	ignore_action.sa_handler = SIG_IGN;
	ignore_action.sa_flags = 0;
	sigemptyset(&ignore_action.sa_mask);

	def_action.sa_handler = SIG_DFL;
	def_action.sa_flags = 0;
	sigemptyset(&def_action.sa_mask);

#ifdef sgi
	exit_action.sa_sigaction = cleanup_handler;
	exit_action.sa_flags = SA_SIGINFO;
	sigemptyset(&exit_action.sa_mask);

	sa.sa_sigaction = die_handler;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);

	sigbus_action.sa_sigaction = sigbus_handler;
	sigbus_action.sa_flags = SA_SIGINFO;
	sigemptyset(&sigbus_action.sa_mask);
#else
	exit_action.sa_handler = cleanup_handler;
	exit_action.sa_flags = 0;
	sigemptyset(&exit_action.sa_mask);

	sa.sa_handler = die_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);

#ifndef CRAY
	sigbus_action.sa_handler = sigbus_handler;
	sigbus_action.sa_flags = 0;
	sigemptyset(&sigbus_action.sa_mask);
#endif
#endif

	for (i = 1; i <= NSIG; i++) {
		switch (i) {
			/* Signals to terminate program on */
		case SIGINT:
			sigaction(i, &exit_action, NULL);
			break;

#ifndef CRAY
			/* This depends on active_mmap_rw */
		case SIGBUS:
			sigaction(i, &sigbus_action, NULL);
			break;
#endif

			/* Signals to Ignore... */
		case SIGSTOP:
		case SIGCONT:
#ifdef SIGRECOVERY
		case SIGRECOVERY:
#endif
			sigaction(i, &ignore_action, NULL);
			break;

			/* Signals to trap & report & die */
			/*case SIGTRAP: */
			/*case SIGABRT: */
#ifdef SIGERR			/* cray only signals */
		case SIGERR:
		case SIGBUFIO:
		case SIGINFO:
#endif
			/*case SIGFPE: */
		case SIGURG:
		case SIGHUP:
		case SIGTERM:
		case SIGPIPE:
		case SIGIO:
		case SIGUSR1:
		case SIGUSR2:
			sigaction(i, &sa, NULL);
			break;

			/* Default Action for all other signals */
		default:
			sigaction(i, &def_action, NULL);
			break;
		}
	}

	/*
	 * Main loop - each doio proc does this until the read returns eof (0).
	 * Call the appropriate io function based on the request type.
	 */

	while ((nbytes = read(infd, (char *)&ioreq, sizeof(ioreq)))) {

		/*
		 * Periodically check our ppid.  If it is 1, the child exits to
		 * help clean up in the case that the main doio process was
		 * killed.
		 */

		if (Reqno && ((Reqno % PPID_CHECK_INTERVAL) == 0)) {
			if (getppid() == 1) {
				doio_fprintf(stderr,
					     "Parent doio process has exited\n");
				alloc_mem(-1);
				exit(E_SETUP);
			}
		}

		if (nbytes == -1) {
			doio_fprintf(stderr,
				     "read of %d bytes from input failed:  %s (%d)\n",
				     sizeof(ioreq), SYSERR, errno);
			alloc_mem(-1);
			exit(E_SETUP);
		}

		if (nbytes != sizeof(ioreq)) {
			doio_fprintf(stderr,
				     "read wrong # bytes from input stream, expected %d, got %d\n",
				     sizeof(ioreq), nbytes);
			alloc_mem(-1);
			exit(E_SETUP);
		}

		if (ioreq.r_magic != DOIO_MAGIC) {
			doio_fprintf(stderr,
				     "got a bad magic # from input stream.  Expected 0%o, got 0%o\n",
				     DOIO_MAGIC, ioreq.r_magic);
			alloc_mem(-1);
			exit(E_SETUP);
		}

		/*
		 * If we're on a Release_Interval multiple, relase all ssd and
		 * core space, and close all fd's in Fd_Map[].
		 */

		if (Reqno && Release_Interval && !(Reqno % Release_Interval)) {
			if (Memsize) {
#ifdef NOTDEF
				sbrk(-1 * Memsize);
#else
				alloc_mem(-1);
#endif
			}
#ifdef _CRAY1
			if (Sdssize) {
				ssbreak(-1 * btoc(Sdssize));
				Sdsptr = 0;
				Sdssize = 0;
			}
#endif /* _CRAY1 */

			alloc_fd(NULL, 0);
		}

		switch (ioreq.r_type) {
		case READ:
		case READA:
			rval = do_read(&ioreq);
			break;

		case WRITE:
		case WRITEA:
			rval = do_write(&ioreq);
			break;

		case READV:
		case AREAD:
		case PREAD:
		case LREAD:
		case LREADA:
		case LSREAD:
		case LSREADA:
		case WRITEV:
		case AWRITE:
		case PWRITE:
		case MMAPR:
		case MMAPW:
		case LWRITE:
		case LWRITEA:
		case LSWRITE:
		case LSWRITEA:
		case LEREAD:
		case LEREADA:
		case LEWRITE:
		case LEWRITEA:
			rval = do_rw(&ioreq);
			break;

#ifdef CRAY
		case SSREAD:
		case SSWRITE:
			rval = do_ssdio(&ioreq);
			break;

		case LISTIO:
			rval = do_listio(&ioreq);
			break;
#endif

#ifdef sgi
		case RESVSP:
		case UNRESVSP:
#ifdef F_FSYNC
		case DFFSYNC:
#endif
			rval = do_fcntl(&ioreq);
			break;
#endif /* sgi */

#ifndef CRAY
		case FSYNC2:
		case FDATASYNC:
			rval = do_sync(&ioreq);
			break;
#endif
		default:
			doio_fprintf(stderr,
				     "Don't know how to handle io request type %d\n",
				     ioreq.r_type);
			alloc_mem(-1);
			exit(E_SETUP);
		}

		if (rval == SKIP_REQ) {
			Reqskipcnt++;
		} else if (rval != 0) {
			alloc_mem(-1);
			doio_fprintf(stderr,
				     "doio(): operation %d returned != 0\n",
				     ioreq.r_type);
			exit(E_SETUP);
		}

		if (Message_Interval && Reqno % Message_Interval == 0) {
			doio_fprintf(stderr,
				     "Info:  %d requests done (%d skipped) by this process\n",
				     Reqno, Reqskipcnt);
		}

		Reqno++;

		if (delayop != 0)
			doio_delay();
	}

	/*
	 * Child exits normally
	 */
	alloc_mem(-1);
	exit(E_NORMAL);

}				/* doio */

void doio_delay(void)
{
	struct timeval tv_delay;
	struct sigaction sa_al, sa_old;
	sigset_t al_mask;

	switch (delayop) {
	case DELAY_SELECT:
		tv_delay.tv_sec = delaytime / 1000000;
		tv_delay.tv_usec = delaytime % 1000000;
		/*doio_fprintf(stdout, "delay_select: %d %d\n",
		   tv_delay.tv_sec, tv_delay.tv_usec); */
		select(0, NULL, NULL, NULL, &tv_delay);
		break;

	case DELAY_SLEEP:
		sleep(delaytime);
		break;

#ifdef sgi
	case DELAY_SGINAP:
		sginap(delaytime);
		break;
#endif

	case DELAY_ALARM:
		sa_al.sa_flags = 0;
		sa_al.sa_handler = noop_handler;
		sigemptyset(&sa_al.sa_mask);
		sigaction(SIGALRM, &sa_al, &sa_old);
		sigemptyset(&al_mask);
		alarm(delaytime);
		sigsuspend(&al_mask);
		sigaction(SIGALRM, &sa_old, 0);
		break;
	}
}

/*
 * Format IO requests, returning a pointer to the formatted text.
 *
 * format_strat	- formats the async i/o completion strategy
 * format_rw	- formats a read[a]/write[a] request
 * format_sds	- formats a ssread/sswrite request
 * format_listio- formats a listio request
 *
 * ioreq is the doio io request structure.
 */

struct smap sysnames[] = {
	{"READ", READ},
	{"WRITE", WRITE},
	{"READA", READA},
	{"WRITEA", WRITEA},
	{"SSREAD", SSREAD},
	{"SSWRITE", SSWRITE},
	{"LISTIO", LISTIO},
	{"LREAD", LREAD},
	{"LREADA", LREADA},
	{"LWRITE", LWRITE},
	{"LWRITEA", LWRITEA},
	{"LSREAD", LSREAD},
	{"LSREADA", LSREADA},
	{"LSWRITE", LSWRITE},
	{"LSWRITEA", LSWRITEA},

	/* Irix System Calls */
	{"PREAD", PREAD},
	{"PWRITE", PWRITE},
	{"AREAD", AREAD},
	{"AWRITE", AWRITE},
	{"LLREAD", LLREAD},
	{"LLAREAD", LLAREAD},
	{"LLWRITE", LLWRITE},
	{"LLAWRITE", LLAWRITE},
	{"RESVSP", RESVSP},
	{"UNRESVSP", UNRESVSP},
	{"DFFSYNC", DFFSYNC},

	/* Irix and Linux System Calls */
	{"READV", READV},
	{"WRITEV", WRITEV},
	{"MMAPR", MMAPR},
	{"MMAPW", MMAPW},
	{"FSYNC2", FSYNC2},
	{"FDATASYNC", FDATASYNC},

	{"unknown", -1},
};

struct smap aionames[] = {
	{"poll", A_POLL},
	{"signal", A_SIGNAL},
	{"recall", A_RECALL},
	{"recalla", A_RECALLA},
	{"recalls", A_RECALLS},
	{"suspend", A_SUSPEND},
	{"callback", A_CALLBACK},
	{"synch", 0},
	{"unknown", -1},
};

char *format_oflags(int oflags)
{
	char flags[255];

	flags[0] = '\0';
	switch (oflags & 03) {
	case O_RDONLY:
		strcat(flags, "O_RDONLY,");
		break;
	case O_WRONLY:
		strcat(flags, "O_WRONLY,");
		break;
	case O_RDWR:
		strcat(flags, "O_RDWR,");
		break;
	default:
		strcat(flags, "O_weird");
		break;
	}

	if (oflags & O_EXCL)
		strcat(flags, "O_EXCL,");

	if (oflags & O_SYNC)
		strcat(flags, "O_SYNC,");
#ifdef CRAY
	if (oflags & O_RAW)
		strcat(flags, "O_RAW,");
	if (oflags & O_WELLFORMED)
		strcat(flags, "O_WELLFORMED,");
#ifdef O_SSD
	if (oflags & O_SSD)
		strcat(flags, "O_SSD,");
#endif
	if (oflags & O_LDRAW)
		strcat(flags, "O_LDRAW,");
	if (oflags & O_PARALLEL)
		strcat(flags, "O_PARALLEL,");
	if (oflags & O_BIG)
		strcat(flags, "O_BIG,");
	if (oflags & O_PLACE)
		strcat(flags, "O_PLACE,");
	if (oflags & O_ASYNC)
		strcat(flags, "O_ASYNC,");
#endif

#ifdef sgi
	if (oflags & O_DIRECT)
		strcat(flags, "O_DIRECT,");
	if (oflags & O_DSYNC)
		strcat(flags, "O_DSYNC,");
	if (oflags & O_RSYNC)
		strcat(flags, "O_RSYNC,");
#endif

	return (strdup(flags));
}

char *format_strat(int strategy)
{
	char msg[64];
	char *aio_strat;

	switch (strategy) {
	case A_POLL:
		aio_strat = "POLL";
		break;
	case A_SIGNAL:
		aio_strat = "SIGNAL";
		break;
	case A_RECALL:
		aio_strat = "RECALL";
		break;
	case A_RECALLA:
		aio_strat = "RECALLA";
		break;
	case A_RECALLS:
		aio_strat = "RECALLS";
		break;
	case A_SUSPEND:
		aio_strat = "SUSPEND";
		break;
	case A_CALLBACK:
		aio_strat = "CALLBACK";
		break;
	case 0:
		aio_strat = "<zero>";
		break;
	default:
		sprintf(msg, "<error:%#o>", strategy);
		aio_strat = strdup(msg);
		break;
	}

	return (aio_strat);
}

char *format_rw(struct io_req *ioreq, int fd, void *buffer, int signo,
		char *pattern, void *iosw)
{
	static char *errbuf = NULL;
	char *aio_strat, *cp;
	struct read_req *readp = &ioreq->r_data.read;
	struct write_req *writep = &ioreq->r_data.write;
	struct read_req *readap = &ioreq->r_data.read;
	struct write_req *writeap = &ioreq->r_data.write;

	if (errbuf == NULL)
		errbuf = malloc(32768);

	cp = errbuf;
	cp += sprintf(cp, "Request number %d\n", Reqno);

	switch (ioreq->r_type) {
	case READ:
		cp += sprintf(cp, "syscall:  read(%d, %#lo, %d)\n",
			      fd, (unsigned long)buffer, readp->r_nbytes);
		cp +=
		    sprintf(cp,
			    "          fd %d is file %s - open flags are %#o\n",
			    fd, readp->r_file, readp->r_oflags);
		cp +=
		    sprintf(cp, "          read done at file offset %d\n",
			    readp->r_offset);
		break;

	case WRITE:
		cp += sprintf(cp, "syscall:  write(%d, %#lo, %d)\n",
			      fd, (unsigned long)buffer, writep->r_nbytes);
		cp +=
		    sprintf(cp,
			    "          fd %d is file %s - open flags are %#o\n",
			    fd, writep->r_file, writep->r_oflags);
		cp +=
		    sprintf(cp,
			    "          write done at file offset %d - pattern is %s\n",
			    writep->r_offset, pattern);
		break;

	case READA:
		aio_strat = format_strat(readap->r_aio_strat);

		cp += sprintf(cp, "syscall:  reada(%d, %#lo, %d, %#lo, %d)\n",
			      fd, (unsigned long)buffer, readap->r_nbytes,
			      (unsigned long)iosw, signo);
		cp +=
		    sprintf(cp,
			    "          fd %d is file %s - open flags are %#o\n",
			    fd, readap->r_file, readp->r_oflags);
		cp +=
		    sprintf(cp, "          reada done at file offset %d\n",
			    readap->r_offset);
		cp +=
		    sprintf(cp,
			    "          async io completion strategy is %s\n",
			    aio_strat);
		break;

	case WRITEA:
		aio_strat = format_strat(writeap->r_aio_strat);

		cp += sprintf(cp, "syscall:  writea(%d, %#lo, %d, %#lo, %d)\n",
			      fd, (unsigned long)buffer, writeap->r_nbytes,
			      (unsigned long)iosw, signo);
		cp +=
		    sprintf(cp,
			    "          fd %d is file %s - open flags are %#o\n",
			    fd, writeap->r_file, writeap->r_oflags);
		cp +=
		    sprintf(cp,
			    "          writea done at file offset %d - pattern is %s\n",
			    writeap->r_offset, pattern);
		cp +=
		    sprintf(cp,
			    "          async io completion strategy is %s\n",
			    aio_strat);
		break;

	}

	return errbuf;
}

#ifdef CRAY
char *format_sds(struct io_req *ioreq, void *buffer, int sds, char *pattern)
{
	int i;
	static char *errbuf = NULL;
	char *cp;

	struct ssread_req *ssreadp = &ioreq->r_data.ssread;
	struct sswrite_req *sswritep = &ioreq->r_data.sswrite;

	if (errbuf == NULL)
		errbuf = malloc(32768);

	cp = errbuf;
	cp += sprintf(cp, "Request number %d\n", Reqno);

	switch (ioreq->r_type) {
	case SSREAD:
		cp += sprintf(cp, "syscall:  ssread(%#o, %#o, %d)\n",
			      buffer, sds, ssreadp->r_nbytes);
		break;

	case SSWRITE:
		cp +=
		    sprintf(cp,
			    "syscall:  sswrite(%#o, %#o, %d) - pattern was %s\n",
			    buffer, sds, sswritep->r_nbytes, pattern);
		break;
	}
	return errbuf;
}
#endif /* CRAY */

/*
 * Perform the various sorts of disk reads
 */

int do_read(struct io_req *req)
{
	int fd, offset, nbytes, oflags, rval;
	char *addr, *file;
#ifdef CRAY
	struct aio_info *aiop;
	int aio_id, aio_strat, signo;
#endif
#ifdef sgi
	struct fd_cache *fdc;
#endif

	/*
	 * Initialize common fields - assumes r_oflags, r_file, r_offset, and
	 * r_nbytes are at the same offset in the read_req and reada_req
	 * structures.
	 */

	file = req->r_data.read.r_file;
	oflags = req->r_data.read.r_oflags;
	offset = req->r_data.read.r_offset;
	nbytes = req->r_data.read.r_nbytes;

	/*printf("read: %s, %#o, %d %d\n", file, oflags, offset, nbytes); */

	/*
	 * Grab an open file descriptor
	 * Note: must be done before memory allocation so that the direct i/o
	 *      information is available in mem. allocate
	 */

	if ((fd = alloc_fd(file, oflags)) == -1)
		return -1;

	/*
	 * Allocate core or sds - based on the O_SSD flag
	 */

#ifndef wtob
#define wtob(x)	(x * sizeof(UINT64_T))
#endif

#ifdef CRAY
	if (oflags & O_SSD) {
		if (alloc_sds(nbytes) == -1)
			return -1;

		addr = (char *)Sdsptr;
	} else {
		if ((rval =
		     alloc_mem(nbytes + wtob(1) * 2 +
			       MPP_BUMP * sizeof(UINT64_T))) < 0) {
			return rval;
		}

		addr = Memptr;

		/*
		 * if io is not raw, bump the offset by a random amount
		 * to generate non-word-aligned io.
		 */
		if (!(req->r_data.read.r_uflags & F_WORD_ALIGNED)) {
			addr += random_range(0, wtob(1) - 1, 1, NULL);
		}
	}
#else
#ifdef sgi
	/* get memory alignment for using DIRECT I/O */
	fdc = alloc_fdcache(file, oflags);

	if ((rval = alloc_mem(nbytes + wtob(1) * 2 + fdc->c_memalign)) < 0) {
		return rval;
	}

	addr = Memptr;

	if ((req->r_data.read.r_uflags & F_WORD_ALIGNED)) {
		/*
		 * Force memory alignment for Direct I/O
		 */
		if ((oflags & O_DIRECT) && ((long)addr % fdc->c_memalign != 0)) {
			addr +=
			    fdc->c_memalign - ((long)addr % fdc->c_memalign);
		}
	} else {
		addr += random_range(0, wtob(1) - 1, 1, NULL);
	}
#else
	/* what is !CRAY && !sgi ? */
	if ((rval = alloc_mem(nbytes + wtob(1) * 2)) < 0) {
		return rval;
	}

	addr = Memptr;
#endif /* !CRAY && sgi */
#endif /* CRAY */

	switch (req->r_type) {
	case READ:
		/* move to the desired file position. */
		if (lseek(fd, offset, SEEK_SET) == -1) {
			doio_fprintf(stderr,
				     "lseek(%d, %d, SEEK_SET) failed:  %s (%d)\n",
				     fd, offset, SYSERR, errno);
			return -1;
		}

		if ((rval = read(fd, addr, nbytes)) == -1) {
			doio_fprintf(stderr,
				     "read() request failed:  %s (%d)\n%s\n",
				     SYSERR, errno,
				     format_rw(req, fd, addr, -1, NULL, NULL));
			doio_upanic(U_RVAL);
			return -1;
		} else if (rval != nbytes) {
			doio_fprintf(stderr,
				     "read() request returned wrong # of bytes - expected %d, got %d\n%s\n",
				     nbytes, rval,
				     format_rw(req, fd, addr, -1, NULL, NULL));
			doio_upanic(U_RVAL);
			return -1;
		}
		break;

#ifdef CRAY
	case READA:
		/*
		 * Async read
		 */

		/* move to the desired file position. */
		if (lseek(fd, offset, SEEK_SET) == -1) {
			doio_fprintf(stderr,
				     "lseek(%d, %d, SEEK_SET) failed:  %s (%d)\n",
				     fd, offset, SYSERR, errno);
			return -1;
		}

		aio_strat = req->r_data.read.r_aio_strat;
		signo = (aio_strat == A_SIGNAL) ? SIGUSR1 : 0;

		aio_id = aio_register(fd, aio_strat, signo);
		aiop = aio_slot(aio_id);

		if (reada(fd, addr, nbytes, &aiop->iosw, signo) == -1) {
			doio_fprintf(stderr, "reada() failed: %s (%d)\n%s\n",
				     SYSERR, errno,
				     format_rw(req, fd, addr, signo, NULL,
					       &aiop->iosw));
			aio_unregister(aio_id);
			doio_upanic(U_RVAL);
			rval = -1;
		} else {
			/*
			 * Wait for io to complete
			 */

			aio_wait(aio_id);

			/*
			 * make sure the io completed without error
			 */

			if (aiop->iosw.sw_count != nbytes) {
				doio_fprintf(stderr,
					     "Bad iosw from reada()\nExpected (%d,%d,%d), got (%d,%d,%d)\n%s\n",
					     1, 0, nbytes,
					     aiop->iosw.sw_flag,
					     aiop->iosw.sw_error,
					     aiop->iosw.sw_count,
					     format_rw(req, fd, addr, signo,
						       NULL, &aiop->iosw));
				aio_unregister(aio_id);
				doio_upanic(U_IOSW);
				rval = -1;
			} else {
				aio_unregister(aio_id);
				rval = 0;
			}
		}

		if (rval == -1)
			return rval;
		break;
#endif /* CRAY */
	}

	return 0;		/* if we get here, everything went ok */
}

/*
 * Perform the verious types of disk writes.
 */

int do_write(struct io_req *req)
{
	static int pid = -1;
	int fd, nbytes, oflags, signo;
	int logged_write, rval, got_lock;
	off_t offset, woffset;
	char *addr, pattern, *file, *msg;
	struct wlog_rec wrec;
#ifdef CRAY
	int aio_strat, aio_id;
	struct aio_info *aiop;
#endif
#ifdef sgi
	struct fd_cache *fdc;
#endif

	woffset = 0;

	/*
	 * Misc variable setup
	 */

	signo = 0;
	nbytes = req->r_data.write.r_nbytes;
	offset = req->r_data.write.r_offset;
	pattern = req->r_data.write.r_pattern;
	file = req->r_data.write.r_file;
	oflags = req->r_data.write.r_oflags;

	/*printf("pwrite: %s, %#o, %d %d\n", file, oflags, offset, nbytes); */

	/*
	 * Allocate core memory and possibly sds space.  Initialize the data
	 * to be written.
	 */

	Pattern[0] = pattern;

	/*
	 * Get a descriptor to do the io on
	 */

	if ((fd = alloc_fd(file, oflags)) == -1)
		return -1;

	/*printf("write: %d, %s, %#o, %d %d\n",
	   fd, file, oflags, offset, nbytes); */

	/*
	 * Allocate SDS space for backdoor write if desired
	 */

#ifdef CRAY
	if (oflags & O_SSD) {
#ifndef _CRAYMPP
		if ((rval = alloc_mem(nbytes + wtob(1))) < 0) {
			return rval;
		}

		(*Data_Fill) (Memptr, nbytes, Pattern, Pattern_Length, 0);
		/*pattern_fill(Memptr, nbytes, Pattern, Pattern_Length, 0); */

		if (alloc_sds(nbytes) == -1)
			return -1;

		if (sswrite((long)Memptr, Sdsptr, btoc(nbytes)) == -1) {
			doio_fprintf(stderr,
				     "sswrite(%d, %d, %d) failed:  %s (%d)\n",
				     (long)Memptr, Sdsptr, btoc(nbytes), SYSERR,
				     errno);
			fflush(stderr);
			return -1;
		}

		addr = (char *)Sdsptr;
#else
		doio_fprintf(stderr,
			     "Invalid O_SSD flag was generated for MPP system\n");
		fflush(stderr);
		return -1;
#endif /* !CRAYMPP */
	} else {
		if ((rval = alloc_mem(nbytes + wtob(1)) < 0)) {
			return rval;
		}

		addr = Memptr;

		/*
		 * if io is not raw, bump the offset by a random amount
		 * to generate non-word-aligned io.
		 */

		if (!(req->r_data.write.r_uflags & F_WORD_ALIGNED)) {
			addr += random_range(0, wtob(1) - 1, 1, NULL);
		}

		(*Data_Fill) (Memptr, nbytes, Pattern, Pattern_Length, 0);
		if (addr != Memptr)
			memmove(addr, Memptr, nbytes);
	}
#else /* CRAY */
#ifdef sgi
	/* get memory alignment for using DIRECT I/O */
	fdc = alloc_fdcache(file, oflags);

	if ((rval = alloc_mem(nbytes + wtob(1) * 2 + fdc->c_memalign)) < 0) {
		return rval;
	}

	addr = Memptr;

	if ((req->r_data.write.r_uflags & F_WORD_ALIGNED)) {
		/*
		 * Force memory alignment for Direct I/O
		 */
		if ((oflags & O_DIRECT) && ((long)addr % fdc->c_memalign != 0)) {
			addr +=
			    fdc->c_memalign - ((long)addr % fdc->c_memalign);
		}
	} else {
		addr += random_range(0, wtob(1) - 1, 1, NULL);
	}

	(*Data_Fill) (Memptr, nbytes, Pattern, Pattern_Length, 0);
	if (addr != Memptr)
		memmove(addr, Memptr, nbytes);

#else /* sgi */
	if ((rval = alloc_mem(nbytes + wtob(1) * 2)) < 0) {
		return rval;
	}

	addr = Memptr;

	(*Data_Fill) (Memptr, nbytes, Pattern, Pattern_Length, 0);
	if (addr != Memptr)
		memmove(addr, Memptr, nbytes);
#endif /* sgi */
#endif /* CRAY */

	rval = -1;
	got_lock = 0;
	logged_write = 0;

	if (k_opt) {
		if (lock_file_region(file, fd, F_WRLCK, offset, nbytes) < 0) {
			alloc_mem(-1);
			exit(E_INTERNAL);
		}

		got_lock = 1;
	}

	/*
	 * Write a preliminary write-log entry.  This is done so that
	 * doio_check can do corruption detection across an interrupt/crash.
	 * Note that w_done is set to 0.  If doio_check sees this, it
	 * re-creates the file extents as if the write completed, but does not
	 * do any checking - see comments in doio_check for more details.
	 */

	if (w_opt) {
		if (pid == -1) {
			pid = getpid();
		}
		wrec.w_async = (req->r_type == WRITEA) ? 1 : 0;
		wrec.w_oflags = oflags;
		wrec.w_pid = pid;
		wrec.w_offset = offset;
		wrec.w_nbytes = nbytes;

		wrec.w_pathlen = strlen(file);
		memcpy(wrec.w_path, file, wrec.w_pathlen);
		wrec.w_hostlen = strlen(Host);
		memcpy(wrec.w_host, Host, wrec.w_hostlen);
		wrec.w_patternlen = Pattern_Length;
		memcpy(wrec.w_pattern, Pattern, wrec.w_patternlen);

		wrec.w_done = 0;

		if ((woffset = wlog_record_write(&Wlog, &wrec, -1)) == -1) {
			doio_fprintf(stderr,
				     "Could not append to write-log:  %s (%d)\n",
				     SYSERR, errno);
		} else {
			logged_write = 1;
		}
	}

	switch (req->r_type) {
	case WRITE:
		/*
		 * sync write
		 */

		if (lseek(fd, offset, SEEK_SET) == -1) {
			doio_fprintf(stderr,
				     "lseek(%d, %d, SEEK_SET) failed:  %s (%d)\n",
				     fd, offset, SYSERR, errno);
			return -1;
		}

		rval = write(fd, addr, nbytes);

		if (rval == -1) {
			doio_fprintf(stderr,
				     "write() failed:  %s (%d)\n%s\n",
				     SYSERR, errno,
				     format_rw(req, fd, addr, -1, Pattern,
					       NULL));
#ifdef sgi
			doio_fprintf(stderr,
				     "write() failed:  %s\n\twrite(%d, %#o, %d)\n\toffset %d, nbytes%%miniou(%d)=%d, oflags=%#o memalign=%d, addr%%memalign=%d\n",
				     strerror(errno),
				     fd, addr, nbytes,
				     offset,
				     fdc->c_miniosz, nbytes % fdc->c_miniosz,
				     oflags, fdc->c_memalign,
				     (long)addr % fdc->c_memalign);
#else
			doio_fprintf(stderr,
				     "write() failed:  %s\n\twrite(%d, %#o, %d)\n\toffset %d, nbytes%%1B=%d, oflags=%#o\n",
				     strerror(errno),
				     fd, addr, nbytes,
				     offset, nbytes % 4096, oflags);
#endif
			doio_upanic(U_RVAL);
		} else if (rval != nbytes) {
			doio_fprintf(stderr,
				     "write() returned wrong # bytes - expected %d, got %d\n%s\n",
				     nbytes, rval,
				     format_rw(req, fd, addr, -1, Pattern,
					       NULL));
			doio_upanic(U_RVAL);
			rval = -1;
		}

		break;

#ifdef CRAY
	case WRITEA:
		/*
		 * async write
		 */
		if (lseek(fd, offset, SEEK_SET) == -1) {
			doio_fprintf(stderr,
				     "lseek(%d, %d, SEEK_SET) failed:  %s (%d)\n",
				     fd, offset, SYSERR, errno);
			return -1;
		}

		aio_strat = req->r_data.write.r_aio_strat;
		signo = (aio_strat == A_SIGNAL) ? SIGUSR1 : 0;

		aio_id = aio_register(fd, aio_strat, signo);
		aiop = aio_slot(aio_id);

		/*
		 * init iosw and do the async write
		 */

		if (writea(fd, addr, nbytes, &aiop->iosw, signo) == -1) {
			doio_fprintf(stderr,
				     "writea() failed: %s (%d)\n%s\n",
				     SYSERR, errno,
				     format_rw(req, fd, addr, -1, Pattern,
					       NULL));
			doio_upanic(U_RVAL);
			aio_unregister(aio_id);
			rval = -1;
		} else {

			/*
			 * Wait for io to complete
			 */

			aio_wait(aio_id);

			/*
			 * check that iosw is ok
			 */

			if (aiop->iosw.sw_count != nbytes) {
				doio_fprintf(stderr,
					     "Bad iosw from writea()\nExpected (%d,%d,%d), got (%d,%d,%d)\n%s\n",
					     1, 0, nbytes,
					     aiop->iosw.sw_flag,
					     aiop->iosw.sw_error,
					     aiop->iosw.sw_count,
					     format_rw(req, fd, addr, -1,
						       Pattern, &aiop->iosw));
				aio_unregister(aio_id);
				doio_upanic(U_IOSW);
				rval = -1;
			} else {
				aio_unregister(aio_id);
				rval = 0;
			}
		}
		break;

#endif /* CRAY */
	}

	/*
	 * Verify that the data was written correctly - check_file() returns
	 * a non-null pointer which contains an error message if there are
	 * problems.
	 */

	if (v_opt) {
		msg = check_file(file, offset, nbytes, Pattern, Pattern_Length,
				 0, oflags & O_PARALLEL);
		if (msg != NULL) {
			doio_fprintf(stderr, "%s%s\n", msg,
#ifdef CRAY
				     format_rw(req, fd, addr, -1, Pattern,
					       &aiop->iosw)
#else
				     format_rw(req, fd, addr, -1, Pattern, NULL)
#endif
			    );
			doio_upanic(U_CORRUPTION);
			exit(E_COMPARE);

		}
	}

	/*
	 * General cleanup ...
	 *
	 * Write extent information to the write-log, so that doio_check can do
	 * corruption detection.  Note that w_done is set to 1, indicating that
	 * the write has been verified as complete.  We don't need to write the
	 * filename on the second logging.
	 */

	if (w_opt && logged_write) {
		wrec.w_done = 1;
		wlog_record_write(&Wlog, &wrec, woffset);
	}

	/*
	 * Unlock file region if necessary
	 */

	if (got_lock) {
		if (lock_file_region(file, fd, F_UNLCK, offset, nbytes) < 0) {
			alloc_mem(-1);
			exit(E_INTERNAL);
		}
	}

	return ((rval == -1) ? -1 : 0);
}

/*
 * Simple routine to lock/unlock a file using fcntl()
 */

int lock_file_region(char *fname, int fd, int type, int start, int nbytes)
{
	struct flock flk;

	flk.l_type = type;
	flk.l_whence = 0;
	flk.l_start = start;
	flk.l_len = nbytes;

	if (fcntl(fd, F_SETLKW, &flk) < 0) {
		doio_fprintf(stderr,
			     "fcntl(%d, %d, %#o) failed for file %s, lock type %d, offset %d, length %d:  %s (%d), open flags: %#o\n",
			     fd, F_SETLKW, &flk, fname, type,
			     start, nbytes, SYSERR, errno,
			     fcntl(fd, F_GETFL, 0));
		return -1;
	}

	return 0;
}

/*
 * Perform a listio request.
 */

#ifdef CRAY
char *format_listio(struct io_req *ioreq, int lcmd, struct listreq *list,
		    int nent, int fd, char *pattern)
{
	static char *errbuf = NULL;
	struct listio_req *liop = &ioreq->r_data.listio;
	struct listreq *listreq;
	char *cp, *cmd, *opcode, *aio_strat;
	int i;

	switch (lcmd) {
	case LC_START:
		cmd = "LC_START";
		break;
	case LC_WAIT:
		cmd = "LC_WAIT";
		break;
	default:
		cmd = "???";
		break;
	}

	if (errbuf == NULL)
		errbuf = malloc(32768);

	cp = errbuf;
	cp += sprintf(cp, "Request number %d\n", Reqno);

	cp += sprintf(cp, "syscall:  listio(%s, %#o, %d)\n\n", cmd, list, nent);

	aio_strat = format_strat(liop->r_aio_strat);

	for (i = 0; i < nent; i++) {
		cp += sprintf(cp, "struct lioreq for request element %d\n", i);
		cp += sprintf(cp, "----------------------------------------\n");

		listreq = list + i;

		switch (listreq->li_opcode) {
		case LO_READ:
			opcode = "LO_READ";
			break;
		case LO_WRITE:
			opcode = "LO_WRITE";
			break;
		default:
			opcode = "???";
			break;
		}

		cp += sprintf(cp, "          li_opcode =    %s\n", opcode);
		cp +=
		    sprintf(cp, "          li_drvr =      %#o\n",
			    listreq->li_drvr);
		cp +=
		    sprintf(cp, "          li_flags =     %#o\n",
			    listreq->li_flags);
		cp +=
		    sprintf(cp, "          li_offset =    %d\n",
			    listreq->li_offset);
		cp +=
		    sprintf(cp, "          li_fildes =    %d\n",
			    listreq->li_fildes);
		cp +=
		    sprintf(cp, "          li_buf =       %#o\n",
			    listreq->li_buf);
		cp +=
		    sprintf(cp, "          li_nbyte =     %d\n",
			    listreq->li_nbyte);
		cp +=
		    sprintf(cp, "          li_status =    %#o (%d, %d, %d)\n",
			    listreq->li_status, listreq->li_status->sw_flag,
			    listreq->li_status->sw_error,
			    listreq->li_status->sw_count);
		cp +=
		    sprintf(cp, "          li_signo =     %d\n",
			    listreq->li_signo);
		cp +=
		    sprintf(cp, "          li_nstride =   %d\n",
			    listreq->li_nstride);
		cp +=
		    sprintf(cp, "          li_filstride = %d\n",
			    listreq->li_filstride);
		cp +=
		    sprintf(cp, "          li_memstride = %d\n",
			    listreq->li_memstride);
		cp +=
		    sprintf(cp, "          io completion strategy is %s\n",
			    aio_strat);
	}
	return errbuf;
}
#endif /* CRAY */

int do_listio(struct io_req *req)
{
#ifdef CRAY
	struct listio_req *lio;
	int fd, oflags, signo, nb, i;
	int logged_write, rval, got_lock;
	int aio_strat, aio_id;
	int min_byte, max_byte;
	int mem_needed;
	int foffset, fstride, mstride, nstrides;
	char *moffset;
	long offset, woffset;
	char *addr, *msg;
	sigset_t block_mask, omask;
	struct wlog_rec wrec;
	struct aio_info *aiop;
	struct listreq lio_req;

	lio = &req->r_data.listio;

	/*
	 * If bytes per stride is less than the stride size, drop the request
	 * since it will cause overlapping strides, and we cannot predict
	 * the order they will complete in.
	 */

	if (lio->r_filestride && abs(lio->r_filestride) < lio->r_nbytes) {
		doio_fprintf(stderr,
			     "do_listio():  Bogus listio request - abs(filestride) [%d] < nbytes [%d]\n",
			     abs(lio->r_filestride), lio->r_nbytes);
		return -1;
	}

	/*
	 * Allocate core memory.  Initialize the data to be written.  Make
	 * sure we get enough, based on the memstride.
	 */

	mem_needed =
	    stride_bounds(0, lio->r_memstride, lio->r_nstrides,
			  lio->r_nbytes, NULL, NULL);

	if ((rval = alloc_mem(mem_needed + wtob(1))) < 0) {
		return rval;
	}

	/*
	 * Set the memory address pointer.  If the io is not raw, adjust
	 * addr by a random amount, so that non-raw io is not necessarily
	 * word aligned.
	 */

	addr = Memptr;

	if (!(lio->r_uflags & F_WORD_ALIGNED)) {
		addr += random_range(0, wtob(1) - 1, 1, NULL);
	}

	if (lio->r_opcode == LO_WRITE) {
		Pattern[0] = lio->r_pattern;
		(*Data_Fill) (Memptr, mem_needed, Pattern, Pattern_Length, 0);
		if (addr != Memptr)
			memmove(addr, Memptr, mem_needed);
	}

	/*
	 * Get a descriptor to do the io on.  No need to do an lseek, as this
	 * is encoded in the listio request.
	 */

	if ((fd = alloc_fd(lio->r_file, lio->r_oflags)) == -1) {
		return -1;
	}

	rval = -1;
	got_lock = 0;
	logged_write = 0;

	/*
	 * If the opcode is LO_WRITE, lock all regions of the file that
	 * are touched by this listio request.  Currently, we use
	 * stride_bounds() to figure out the min and max bytes affected, and
	 * lock the entire region, regardless of the file stride.
	 */

	if (lio->r_opcode == LO_WRITE && k_opt) {
		stride_bounds(lio->r_offset,
			      lio->r_filestride, lio->r_nstrides,
			      lio->r_nbytes, &min_byte, &max_byte);

		if (lock_file_region(lio->r_file, fd, F_WRLCK,
				     min_byte, (max_byte - min_byte + 1)) < 0) {
			doio_fprintf(stderr,
				     "stride_bounds(%d, %d, %d, %d, ..., ...) set min_byte to %d, max_byte to %d\n",
				     lio->r_offset, lio->r_filestride,
				     lio->r_nstrides, lio->r_nbytes, min_byte,
				     max_byte);
			return -1;
		} else {
			got_lock = 1;
		}
	}

	/*
	 * async write
	 */

	aio_strat = lio->r_aio_strat;
	signo = (aio_strat == A_SIGNAL) ? SIGUSR1 : 0;

	aio_id = aio_register(fd, aio_strat, signo);
	aiop = aio_slot(aio_id);

	/*
	 * Form the listio request, and make the call.
	 */

	lio_req.li_opcode = lio->r_opcode;
	lio_req.li_drvr = 0;
	lio_req.li_flags = LF_LSEEK;
	lio_req.li_offset = lio->r_offset;
	lio_req.li_fildes = fd;

	if (lio->r_memstride >= 0 || lio->r_nstrides <= 1) {
		lio_req.li_buf = addr;
	} else {
		lio_req.li_buf = addr + mem_needed - lio->r_nbytes;
	}

	lio_req.li_nbyte = lio->r_nbytes;
	lio_req.li_status = &aiop->iosw;
	lio_req.li_signo = signo;
	lio_req.li_nstride = lio->r_nstrides;
	lio_req.li_filstride = lio->r_filestride;
	lio_req.li_memstride = lio->r_memstride;

	/*
	 * If signo != 0, block signo while we're in the system call, so that
	 * we don't get interrupted syscall failures.
	 */

	if (signo) {
		sigemptyset(&block_mask);
		sigaddset(&block_mask, signo);
		sigprocmask(SIG_BLOCK, &block_mask, &omask);
	}

	if (listio(lio->r_cmd, &lio_req, 1) < 0) {
		doio_fprintf(stderr,
			     "listio() failed: %s (%d)\n%s\n",
			     SYSERR, errno,
			     format_listio(req, lio->r_cmd, &lio_req, 1, fd,
					   Pattern));
		aio_unregister(aio_id);
		doio_upanic(U_RVAL);
		goto lio_done;
	}

	if (signo) {
		sigprocmask(SIG_SETMASK, &omask, NULL);
	}

	/*
	 * Wait for io to complete
	 */

	aio_wait(aio_id);

	nstrides = lio->r_nstrides ? lio->r_nstrides : 1;
	if (aiop->iosw.sw_count != lio->r_nbytes * nstrides) {
		doio_fprintf(stderr,
			     "Bad iosw from listio()\nExpected (%d,%d,%d), got (%d,%d,%d)\n%s\n",
			     1, 0, lio->r_nbytes * lio->r_nstrides,
			     aiop->iosw.sw_flag,
			     aiop->iosw.sw_error, aiop->iosw.sw_count,
			     format_listio(req, lio->r_cmd, &lio_req, 1, fd,
					   Pattern));
		aio_unregister(aio_id);
		doio_upanic(U_IOSW);
		goto lio_done;
	}

	aio_unregister(aio_id);

	/*
	 * Verify that the data was written correctly - check_file() returns
	 * a non-null pointer which contains an error message if there are
	 * problems.
	 *
	 * For listio, we basically have to make 1 call to check_file for each
	 * stride.
	 */

	if (v_opt && lio_req.li_opcode == LO_WRITE) {
		fstride = lio->r_filestride ? lio->r_filestride : lio->r_nbytes;
		mstride = lio->r_memstride ? lio->r_memstride : lio->r_nbytes;
		foffset = lio->r_offset;

		if (mstride > 0 || lio->r_nstrides <= 1) {
			moffset = addr;
		} else {
			moffset = addr + mem_needed - lio->r_nbytes;
		}

		for (i = 0; i < lio_req.li_nstride; i++) {
			msg = check_file(lio->r_file,
					 foffset, lio->r_nbytes,
					 Pattern, Pattern_Length,
					 moffset - addr,
					 lio->r_oflags & O_PARALLEL);

			if (msg != NULL) {
				doio_fprintf(stderr, "%s\n%s\n",
					     msg,
					     format_listio(req, lio->r_cmd,
							   &lio_req, 1, fd,
							   Pattern));
				doio_upanic(U_CORRUPTION);
				exit(E_COMPARE);
			}

			moffset += mstride;
			foffset += fstride;
		}

	}

	rval = 0;

lio_done:

	/*
	 * General cleanup ...
	 *
	 */

	/*
	 * Release file locks if necessary
	 */

	if (got_lock) {
		if (lock_file_region(lio->r_file, fd, F_UNLCK,
				     min_byte, (max_byte - min_byte + 1)) < 0) {
			return -1;
		}
	}

	return rval;
#else
	return -1;
#endif
}

/*
 * perform ssread/sswrite operations
 */

#ifdef _CRAY1

int do_ssdio(struct io_req *req)
{
	int nbytes, nb;
	char errbuf[BSIZE];

	nbytes = req->r_data.ssread.r_nbytes;

	/*
	 * Grab core and sds space
	 */

	if ((nb = alloc_mem(nbytes)) < 0)
		return nb;

	if (alloc_sds(nbytes) == -1)
		return -1;

	if (req->r_type == SSWRITE) {

		/*
		 * Init data and ship it to the ssd
		 */

		Pattern[0] = req->r_data.sswrite.r_pattern;
		/*pattern_fill(Memptr, nbytes, Pattern, Pattern_Length, 0); */
		(*Data_Fill) (Memptr, nbytes, Pattern, Pattern_Length, 0);

		if (sswrite((long)Memptr, (long)Sdsptr, btoc(nbytes)) == -1) {
			doio_fprintf(stderr, "sswrite() failed:  %s (%d)\n%s\n",
				     SYSERR, errno,
				     format_sds(req, Memptr, Sdsptr, Pattern));
			doio_upanic(U_RVAL);
			return -1;
		}
	} else {
		/*
		 * read from sds
		 */

		if (ssread((long)Memptr, (long)Sdsptr, btoc(nbytes)) == -1) {
			doio_fprintf(stderr, "ssread() failed: %s (%d)\n%s\n",
				     SYSERR, errno,
				     format_sds(req, Memptr, Sdsptr, Pattern));

			doio_upanic(U_RVAL);
			return -1;
		}
	}

	/*
	 * Verify data if SSWRITE and v_opt
	 */

	if (v_opt && req->r_type == SSWRITE) {
		ssread((long)Memptr, (long)Sdsptr, btoc(nbytes));

		if (pattern_check(Memptr, nbytes, Pattern, Pattern_Length, 0) ==
		    -1) {
			doio_fprintf(stderr,
				     "sds DATA COMPARE ERROR - ABORTING\n%s\n",
				     format_sds(req, Memptr, Sdsptr, Pattern));

			doio_upanic(U_CORRUPTION);
			exit(E_COMPARE);
		}
	}
}

#else

#ifdef CRAY

int do_ssdio(struct io_req *req)
{
	doio_fprintf(stderr,
		     "Internal Error - do_ssdio() called on a non-cray1 system\n");
	alloc_mem(-1);
	exit(E_INTERNAL);
}

#endif /* CRAY */

#endif /* _CRAY1 */

char *fmt_ioreq(struct io_req *ioreq, struct syscall_info *sy, int fd)
{
	static char *errbuf = NULL;
	char *cp;
	struct rw_req *io;
	struct smap *aname;
#ifdef CRAY
	struct stat sbuf;
#endif
#ifdef sgi
	struct dioattr finfo;
#endif

	if (errbuf == NULL)
		errbuf = malloc(32768);

	io = &ioreq->r_data.io;

	/*
	 * Look up async I/O completion strategy
	 */
	for (aname = aionames;
	     aname->value != -1 && aname->value != io->r_aio_strat; aname++) ;

	cp = errbuf;
	cp += sprintf(cp, "Request number %d\n", Reqno);

	cp +=
	    sprintf(cp, "          fd %d is file %s - open flags are %#o %s\n",
		    fd, io->r_file, io->r_oflags, format_oflags(io->r_oflags));

	if (sy->sy_flags & SY_WRITE) {
		cp +=
		    sprintf(cp,
			    "          write done at file offset %d - pattern is %c (%#o)\n",
			    io->r_offset,
			    (io->r_pattern == '\0') ? '?' : io->r_pattern,
			    io->r_pattern);
	} else {
		cp += sprintf(cp, "          read done at file offset %d\n",
			      io->r_offset);
	}

	if (sy->sy_flags & SY_ASYNC) {
		cp +=
		    sprintf(cp,
			    "          async io completion strategy is %s\n",
			    aname->string);
	}

	cp +=
	    sprintf(cp,
		    "          number of requests is %d, strides per request is %d\n",
		    io->r_nent, io->r_nstrides);

	cp += sprintf(cp, "          i/o byte count = %d\n", io->r_nbytes);

	cp += sprintf(cp, "          memory alignment is %s\n",
		      (io->
		       r_uflags & F_WORD_ALIGNED) ? "aligned" : "unaligned");

#ifdef CRAY
	if (io->r_oflags & O_RAW) {
		cp +=
		    sprintf(cp,
			    "          RAW I/O: offset %% 4096 = %d length %% 4096 = %d\n",
			    io->r_offset % 4096, io->r_nbytes % 4096);
		fstat(fd, &sbuf);
		cp +=
		    sprintf(cp,
			    "          optimal file xfer size: small: %d large: %d\n",
			    sbuf.st_blksize, sbuf.st_oblksize);
		cp +=
		    sprintf(cp, "          cblks %d cbits %#o\n", sbuf.st_cblks,
			    sbuf.st_cbits);
	}
#endif
#ifdef sgi
	if (io->r_oflags & O_DIRECT) {

		if (fcntl(fd, F_DIOINFO, &finfo) == -1) {
			cp +=
			    sprintf(cp,
				    "          Error %s (%d) getting direct I/O info\n",
				    strerror(errno), errno);
			finfo.d_mem = 1;
			finfo.d_miniosz = 1;
			finfo.d_maxiosz = 1;
		}

		cp +=
		    sprintf(cp,
			    "          DIRECT I/O: offset %% %d = %d length %% %d = %d\n",
			    finfo.d_miniosz, io->r_offset % finfo.d_miniosz,
			    io->r_nbytes, io->r_nbytes % finfo.d_miniosz);
		cp +=
		    sprintf(cp,
			    "          mem alignment 0x%x xfer size: small: %d large: %d\n",
			    finfo.d_mem, finfo.d_miniosz, finfo.d_maxiosz);
	}
#endif

	return (errbuf);
}

/*
 * Issue listio requests
 */
#ifdef CRAY
struct status *sy_listio(struct io_req *req, struct syscall_info *sysc, int fd,
			 char *addr)
{
	int offset, nbytes, nstrides, nents, aio_strat;
	int aio_id, signo, o, i, lc;
	char *a;
	struct listreq *lio_req, *l;
	struct aio_info *aiop;
	struct status *status;

	/*
	 * Initialize common fields - assumes r_oflags, r_file, r_offset, and
	 * r_nbytes are at the same offset in the read_req and reada_req
	 * structures.
	 */
	offset = req->r_data.io.r_offset;
	nbytes = req->r_data.io.r_nbytes;
	nstrides = req->r_data.io.r_nstrides;
	nents = req->r_data.io.r_nent;
	aio_strat = req->r_data.io.r_aio_strat;

	lc = (sysc->sy_flags & SY_ASYNC) ? LC_START : LC_WAIT;

	status = malloc(sizeof(struct status));
	if (status == NULL) {
		doio_fprintf(stderr, "malloc failed, %s/%d\n",
			     __FILE__, __LINE__);
		return NULL;
	}
	status->aioid = malloc((nents + 1) * sizeof(int));
	if (status->aioid == NULL) {
		doio_fprintf(stderr, "malloc failed, %s/%d\n",
			     __FILE__, __LINE__);
		return NULL;
	}

	signo = (aio_strat == A_SIGNAL) ? SIGUSR1 : 0;

	lio_req = malloc(nents * sizeof(struct listreq));
	if (lio_req == NULL) {
		doio_fprintf(stderr, "malloc failed, %s/%d\n",
			     __FILE__, __LINE__);
		return NULL;
	}
	for (l = lio_req, a = addr, o = offset, i = 0;
	     i < nents; l++, a += nbytes, o += nbytes, i++) {

		aio_id = aio_register(fd, aio_strat, signo);
		aiop = aio_slot(aio_id);
		status->aioid[i] = aio_id;

		l->li_opcode = (sysc->sy_flags & SY_WRITE) ? LO_WRITE : LO_READ;
		l->li_offset = o;
		l->li_fildes = fd;
		l->li_buf = a;
		l->li_nbyte = nbytes;
		l->li_status = &aiop->iosw;
		l->li_signo = signo;
		l->li_nstride = nstrides;
		l->li_filstride = 0;
		l->li_memstride = 0;
		l->li_drvr = 0;
		l->li_flags = LF_LSEEK;
	}

	status->aioid[nents] = -1;	/* end sentinel */

	if ((status->rval = listio(lc, lio_req, nents)) == -1) {
		status->err = errno;
	}

	free(lio_req);
	return (status);
}

/*
 * Calculate the size of a request in bytes and min/max boundaries
 *
 * This assumes filestride & memstride = 0.
 */
int listio_mem(struct io_req *req, int offset, int fmstride, int *min, int *max)
{
	int i, size;

	size = stride_bounds(offset, fmstride,
			     req->r_data.io.r_nstrides * req->r_data.io.r_nent,
			     req->r_data.io.r_nbytes, min, max);
	return (size);
}

char *fmt_listio(struct io_req *req, struct syscall_info *sy, int fd,
		 char *addr)
{
	static char *errbuf = NULL;
	char *cp;
	char *c, *opcode;
	int i;

	if (errbuf == NULL) {
		errbuf = malloc(32768);
		if (errbuf == NULL) {
			doio_fprintf(stderr, "malloc failed, %s/%d\n",
				     __FILE__, __LINE__);
			return NULL;
		}
	}

	c = (sy->sy_flags & SY_ASYNC) ? "lc_wait" : "lc_start";

	cp = errbuf;
	cp += sprintf(cp, "syscall:  listio(%s, (?), %d)\n",
		      c, req->r_data.io.r_nent);

	cp += sprintf(cp, "          data buffer at %#o\n", addr);

	return (errbuf);
}
#endif /* CRAY */

#ifdef sgi
struct status *sy_pread(struct io_req *req, struct syscall_info *sysc, int fd,
			char *addr)
{
	int rc;
	struct status *status;

	rc = pread(fd, addr, req->r_data.io.r_nbytes, req->r_data.io.r_offset);

	status = malloc(sizeof(struct status));
	if (status == NULL) {
		doio_fprintf(stderr, "malloc failed, %s/%d\n",
			     __FILE__, __LINE__);
		return NULL;
	}
	status->aioid = NULL;
	status->rval = rc;
	status->err = errno;

	return (status);
}

struct status *sy_pwrite(struct io_req *req, struct syscall_info *sysc, int fd,
			 char *addr)
{
	int rc;
	struct status *status;

	rc = pwrite(fd, addr, req->r_data.io.r_nbytes, req->r_data.io.r_offset);

	status = malloc(sizeof(struct status));
	if (status == NULL) {
		doio_fprintf(stderr, "malloc failed, %s/%d\n",
			     __FILE__, __LINE__);
		return NULL;
	}
	status->aioid = NULL;
	status->rval = rc;
	status->err = errno;

	return (status);
}

char *fmt_pread(struct io_req *req, struct syscall_info *sy, int fd, char *addr)
{
	static char *errbuf = NULL;
	char *cp;

	if (errbuf == NULL) {
		errbuf = malloc(32768);
		if (errbuf == NULL) {
			doio_fprintf(stderr, "malloc failed, %s/%d\n",
				     __FILE__, __LINE__);
			return NULL;
		}
	}

	cp = errbuf;
	cp += sprintf(cp, "syscall:  %s(%d, 0x%lx, %d)\n",
		      sy->sy_name, fd, addr, req->r_data.io.r_nbytes);
	return (errbuf);
}
#endif /* sgi */

#ifndef CRAY
struct status *sy_readv(struct io_req *req, struct syscall_info *sysc, int fd,
			char *addr)
{
	struct status *sy_rwv();
	return sy_rwv(req, sysc, fd, addr, 0);
}

struct status *sy_writev(struct io_req *req, struct syscall_info *sysc, int fd,
			 char *addr)
{
	struct status *sy_rwv();
	return sy_rwv(req, sysc, fd, addr, 1);
}

struct status *sy_rwv(struct io_req *req, struct syscall_info *sysc, int fd,
		      char *addr, int rw)
{
	int rc;
	struct status *status;
	struct iovec iov[2];

	status = malloc(sizeof(struct status));
	if (status == NULL) {
		doio_fprintf(stderr, "malloc failed, %s/%d\n",
			     __FILE__, __LINE__);
		return NULL;
	}
	status->aioid = NULL;

	/* move to the desired file position. */
	if ((rc = lseek(fd, req->r_data.io.r_offset, SEEK_SET)) == -1) {
		status->rval = rc;
		status->err = errno;
		return (status);
	}

	iov[0].iov_base = addr;
	iov[0].iov_len = req->r_data.io.r_nbytes;

	if (rw)
		rc = writev(fd, iov, 1);
	else
		rc = readv(fd, iov, 1);
	status->aioid = NULL;
	status->rval = rc;
	status->err = errno;
	return (status);
}

char *fmt_readv(struct io_req *req, struct syscall_info *sy, int fd, char *addr)
{
	static char errbuf[32768];
	char *cp;

	cp = errbuf;
	cp += sprintf(cp, "syscall:  %s(%d, (iov on stack), 1)\n",
		      sy->sy_name, fd);
	return (errbuf);
}
#endif /* !CRAY */

#ifdef sgi
struct status *sy_aread(struct io_req *req, struct syscall_info *sysc, int fd,
			char *addr)
{
	struct status *sy_arw();
	return sy_arw(req, sysc, fd, addr, 0);
}

struct status *sy_awrite(struct io_req *req, struct syscall_info *sysc, int fd,
			 char *addr)
{
	struct status *sy_arw();
	return sy_arw(req, sysc, fd, addr, 1);
}

/*
  #define sy_aread(A, B, C, D)	sy_arw(A, B, C, D, 0)
  #define sy_awrite(A, B, C, D)	sy_arw(A, B, C, D, 1)
 */

struct status *sy_arw(struct io_req *req, struct syscall_info *sysc, int fd,
		      char *addr, int rw)
{
	/* POSIX 1003.1b-1993 Async read */
	struct status *status;
	int rc;
	int aio_id, aio_strat, signo;
	struct aio_info *aiop;

	status = malloc(sizeof(struct status));
	if (status == NULL) {
		doio_fprintf(stderr, "malloc failed, %s/%d\n",
			     __FILE__, __LINE__);
		return NULL;
	}
	aio_strat = req->r_data.io.r_aio_strat;
	signo = (aio_strat == A_SIGNAL) ? SIGUSR1 : 0;

	aio_id = aio_register(fd, aio_strat, signo);
	aiop = aio_slot(aio_id);

	memset((void *)&aiop->aiocb, 0, sizeof(aiocb_t));

	aiop->aiocb.aio_fildes = fd;
	aiop->aiocb.aio_nbytes = req->r_data.io.r_nbytes;
	aiop->aiocb.aio_offset = req->r_data.io.r_offset;
	aiop->aiocb.aio_buf = addr;
	aiop->aiocb.aio_reqprio = 0;	/* must be 0 */
	aiop->aiocb.aio_lio_opcode = 0;

	if (aio_strat == A_SIGNAL) {	/* siginfo(2) stuff */
		aiop->aiocb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
		aiop->aiocb.aio_sigevent.sigev_signo = signo;
	} else if (aio_strat == A_CALLBACK) {
		aiop->aiocb.aio_sigevent.sigev_signo = 0;
		aiop->aiocb.aio_sigevent.sigev_notify = SIGEV_CALLBACK;
		aiop->aiocb.aio_sigevent.sigev_func = cb_handler;
		aiop->aiocb.aio_sigevent.sigev_value.sival_int = aio_id;
	} else {
		aiop->aiocb.aio_sigevent.sigev_notify = SIGEV_NONE;
		aiop->aiocb.aio_sigevent.sigev_signo = 0;
	}

	if (rw)
		rc = aio_write(&aiop->aiocb);
	else
		rc = aio_read(&aiop->aiocb);

	status->aioid = malloc(2 * sizeof(int));
	if (status->aioid == NULL) {
		doio_fprintf(stderr, "malloc failed, %s/%d\n",
			     __FILE__, __LINE__);
		return NULL;
	}
	status->aioid[0] = aio_id;
	status->aioid[1] = -1;
	status->rval = rc;
	status->err = errno;
	return (status);
}

char *fmt_aread(struct io_req *req, struct syscall_info *sy, int fd, char *addr)
{
	static char errbuf[32768];
	char *cp;

	cp = errbuf;
	cp += sprintf(cp, "syscall:  %s(&aiop->aiocb)\n", sy->sy_name);
	return (errbuf);
}
#endif /* sgi */

#ifndef CRAY

struct status *sy_mmread(struct io_req *req, struct syscall_info *sysc, int fd,
			 char *addr)
{
	struct status *sy_mmrw();
	return sy_mmrw(req, sysc, fd, addr, 0);
}

struct status *sy_mmwrite(struct io_req *req, struct syscall_info *sysc, int fd,
			  char *addr)
{
	struct status *sy_mmrw();
	return sy_mmrw(req, sysc, fd, addr, 1);
}

struct status *sy_mmrw(struct io_req *req, struct syscall_info *sysc, int fd,
		       char *addr, int rw)
{
	/*
	 * mmap read/write
	 * This version is oriented towards mmaping the file to memory
	 * ONCE and keeping it mapped.
	 */
	struct status *status;
	void *mrc = NULL, *memaddr = NULL;
	struct fd_cache *fdc;
	struct stat sbuf;
	int rc;

	status = malloc(sizeof(struct status));
	if (status == NULL) {
		doio_fprintf(stderr, "malloc failed, %s/%d\n",
			     __FILE__, __LINE__);
		return NULL;
	}
	status->aioid = NULL;
	status->rval = -1;

	fdc = alloc_fdcache(req->r_data.io.r_file, req->r_data.io.r_oflags);

	if (v_opt || fdc->c_memaddr == NULL) {
		if (fstat(fd, &sbuf) < 0) {
			doio_fprintf(stderr, "fstat failed, errno=%d\n", errno);
			status->err = errno;
			return (status);
		}

		fdc->c_memlen = (int)sbuf.st_size;
		mrc = mmap(NULL, (int)sbuf.st_size,
			   rw ? PROT_WRITE | PROT_READ : PROT_READ,
			   MAP_SHARED, fd, 0);

		if (mrc == MAP_FAILED) {
			doio_fprintf(stderr, "mmap() failed - 0x%lx %d\n",
				     mrc, errno);
			status->err = errno;
			return (status);
		}

		fdc->c_memaddr = mrc;
	}

	memaddr = (void *)((char *)fdc->c_memaddr + req->r_data.io.r_offset);

	active_mmap_rw = 1;
	if (rw)
		memcpy(memaddr, addr, req->r_data.io.r_nbytes);
	else
		memcpy(addr, memaddr, req->r_data.io.r_nbytes);
	if (v_opt)
		msync(fdc->c_memaddr, (int)sbuf.st_size, MS_SYNC);
	active_mmap_rw = 0;

	status->rval = req->r_data.io.r_nbytes;
	status->err = 0;

	if (v_opt) {
		rc = munmap(mrc, (int)sbuf.st_size);
	}

	return (status);
}

char *fmt_mmrw(struct io_req *req, struct syscall_info *sy, int fd, char *addr)
{
	static char errbuf[32768];
	char *cp;
	struct fd_cache *fdc;
	void *memaddr;

	fdc = alloc_fdcache(req->r_data.io.r_file, req->r_data.io.r_oflags);

	cp = errbuf;
	cp += sprintf(cp, "syscall:  %s(NULL, %d, %s, MAP_SHARED, %d, 0)\n",
		      sy->sy_name,
		      fdc->c_memlen,
		      (sy->sy_flags & SY_WRITE) ? "PROT_WRITE" : "PROT_READ",
		      fd);

	cp += sprintf(cp, "\tfile is mmaped to: 0x%lx\n",
		      (unsigned long)fdc->c_memaddr);

	memaddr = (void *)((char *)fdc->c_memaddr + req->r_data.io.r_offset);

	cp += sprintf(cp, "\tfile-mem=0x%lx, length=%d, buffer=0x%lx\n",
		      (unsigned long)memaddr, req->r_data.io.r_nbytes,
		      (unsigned long)addr);

	return (errbuf);
}
#endif /* !CRAY */

struct syscall_info syscalls[] = {
#ifdef CRAY
	{"listio-read-sync", LREAD,
	 sy_listio, NULL, fmt_listio,
	 SY_IOSW},
	{"listio-read-strides-sync", LSREAD,
	 sy_listio, listio_mem, fmt_listio,
	 SY_IOSW},
	{"listio-read-reqs-sync", LEREAD,
	 sy_listio, listio_mem, fmt_listio,
	 SY_IOSW},
	{"listio-read-async", LREADA,
	 sy_listio, NULL, fmt_listio,
	 SY_IOSW | SY_ASYNC},
	{"listio-read-strides-async", LSREADA,
	 sy_listio, listio_mem, fmt_listio,
	 SY_IOSW | SY_ASYNC},
	{"listio-read-reqs-async", LEREADA,
	 sy_listio, listio_mem, fmt_listio,
	 SY_IOSW | SY_ASYNC},
	{"listio-write-sync", LWRITE,
	 sy_listio, listio_mem, fmt_listio,
	 SY_IOSW | SY_WRITE},
	{"listio-write-strides-sync", LSWRITE,
	 sy_listio, listio_mem, fmt_listio,
	 SY_IOSW | SY_WRITE},
	{"listio-write-reqs-sync", LEWRITE,
	 sy_listio, listio_mem, fmt_listio,
	 SY_IOSW | SY_WRITE},
	{"listio-write-async", LWRITEA,
	 sy_listio, listio_mem, fmt_listio,
	 SY_IOSW | SY_WRITE | SY_ASYNC},
	{"listio-write-strides-async", LSWRITEA,
	 sy_listio, listio_mem, fmt_listio,
	 SY_IOSW | SY_WRITE | SY_ASYNC},
	{"listio-write-reqs-async", LEWRITEA,
	 sy_listio, listio_mem, fmt_listio,
	 SY_IOSW | SY_WRITE | SY_ASYNC},
#endif

#ifdef sgi
	{"aread", AREAD,
	 sy_aread, NULL, fmt_aread,
	 SY_IOSW | SY_ASYNC},
	{"awrite", AWRITE,
	 sy_awrite, NULL, fmt_aread,
	 SY_IOSW | SY_WRITE | SY_ASYNC},
	{"pread", PREAD,
	 sy_pread, NULL, fmt_pread,
	 0},
	{"pwrite", PWRITE,
	 sy_pwrite, NULL, fmt_pread,
	 SY_WRITE},
#endif

#ifndef CRAY
	{"readv", READV,
	 sy_readv, NULL, fmt_readv,
	 0},
	{"writev", WRITEV,
	 sy_writev, NULL, fmt_readv,
	 SY_WRITE},
	{"mmap-read", MMAPR,
	 sy_mmread, NULL, fmt_mmrw,
	 0},
	{"mmap-write", MMAPW,
	 sy_mmwrite, NULL, fmt_mmrw,
	 SY_WRITE},
#endif

	{NULL, 0,
	 0, 0, 0,
	 0},
};

int do_rw(struct io_req *req)
{
	static int pid = -1;
	int fd, offset, nbytes, nstrides, nents, oflags;
	int rval, mem_needed, i;
	int logged_write, got_lock, pattern;
	off_t woffset;
	int min_byte, max_byte;
	char *addr, *file, *msg;
	struct status *s;
	struct wlog_rec wrec;
	struct syscall_info *sy;
#if defined(CRAY) || defined(sgi)
	struct aio_info *aiop;
	struct iosw *iosw;
#endif
#ifdef sgi
	struct fd_cache *fdc;
#endif

	woffset = 0;

	/*
	 * Initialize common fields - assumes r_oflags, r_file, r_offset, and
	 * r_nbytes are at the same offset in the read_req and reada_req
	 * structures.
	 */
	file = req->r_data.io.r_file;
	oflags = req->r_data.io.r_oflags;
	offset = req->r_data.io.r_offset;
	nbytes = req->r_data.io.r_nbytes;
	nstrides = req->r_data.io.r_nstrides;
	nents = req->r_data.io.r_nent;
	pattern = req->r_data.io.r_pattern;

	if (nents >= MAX_AIO) {
		doio_fprintf(stderr,
			     "do_rw: too many list requests, %d.  Maximum is %d\n",
			     nents, MAX_AIO);
		return (-1);
	}

	/*
	 * look up system call info
	 */
	for (sy = syscalls; sy->sy_name != NULL && sy->sy_type != req->r_type;
	     sy++) ;

	if (sy->sy_name == NULL) {
		doio_fprintf(stderr, "do_rw: unknown r_type %d.\n",
			     req->r_type);
		return (-1);
	}

	/*
	 * Get an open file descriptor
	 * Note: must be done before memory allocation so that the direct i/o
	 *      information is available in mem. allocate
	 */

	if ((fd = alloc_fd(file, oflags)) == -1)
		return -1;

	/*
	 * Allocate core memory and possibly sds space.  Initialize the
	 * data to be written.  Make sure we get enough, based on the
	 * memstride.
	 *
	 * need:
	 *      1 extra word for possible partial-word address "bump"
	 *      1 extra word for dynamic pattern overrun
	 *      MPP_BUMP extra words for T3E non-hw-aligned memory address.
	 */

	if (sy->sy_buffer != NULL) {
		mem_needed = (*sy->sy_buffer) (req, 0, 0, NULL, NULL);
	} else {
		mem_needed = nbytes;
	}

#ifdef CRAY
	if ((rval =
	     alloc_mem(mem_needed + wtob(1) * 2 +
		       MPP_BUMP * sizeof(UINT64_T))) < 0) {
		return rval;
	}
#else
#ifdef sgi
	/* get memory alignment for using DIRECT I/O */
	fdc = alloc_fdcache(file, oflags);

	if ((rval = alloc_mem(mem_needed + wtob(1) * 2 + fdc->c_memalign)) < 0) {
		return rval;
	}
#else
	/* what is !CRAY && !sgi ? */
	if ((rval = alloc_mem(mem_needed + wtob(1) * 2)) < 0) {
		return rval;
	}
#endif /* sgi */
#endif /* CRAY */

	Pattern[0] = pattern;

	/*
	 * Allocate SDS space for backdoor write if desired
	 */

	if (oflags & O_SSD) {
#ifdef CRAY
#ifndef _CRAYMPP
		if (alloc_sds(nbytes) == -1)
			return -1;

		if (sy->sy_flags & SY_WRITE) {
			/*pattern_fill(Memptr, mem_needed, Pattern, Pattern_Length, 0); */
			(*Data_Fill) (Memptr, nbytes, Pattern, Pattern_Length,
				      0);

			if (sswrite((long)Memptr, Sdsptr, btoc(mem_needed)) ==
			    -1) {
				doio_fprintf(stderr,
					     "sswrite(%d, %d, %d) failed:  %s (%d)\n",
					     (long)Memptr, Sdsptr,
					     btoc(mem_needed), SYSERR, errno);
				fflush(stderr);
				return -1;
			}
		}

		addr = (char *)Sdsptr;
#else
		doio_fprintf(stderr,
			     "Invalid O_SSD flag was generated for MPP system\n");
		fflush(stderr);
		return -1;
#endif /* _CRAYMPP */
#else /* CRAY */
		doio_fprintf(stderr,
			     "Invalid O_SSD flag was generated for non-Cray system\n");
		fflush(stderr);
		return -1;
#endif /* CRAY */
	} else {
		addr = Memptr;

		/*
		 * if io is not raw, bump the offset by a random amount
		 * to generate non-word-aligned io.
		 *
		 * On MPP systems, raw I/O must start on an 0x80 byte boundary.
		 * For non-aligned I/O, bump the address from 1 to 8 words.
		 */

		if (!(req->r_data.io.r_uflags & F_WORD_ALIGNED)) {
#ifdef _CRAYMPP
			addr +=
			    random_range(0, MPP_BUMP, 1, NULL) * sizeof(int);
#endif
			addr += random_range(0, wtob(1) - 1, 1, NULL);
		}
#ifdef sgi
		/*
		 * Force memory alignment for Direct I/O
		 */
		if ((oflags & O_DIRECT) && ((long)addr % fdc->c_memalign != 0)) {
			addr +=
			    fdc->c_memalign - ((long)addr % fdc->c_memalign);
		}
#endif

		/*
		 * FILL must be done on a word-aligned buffer.
		 * Call the fill function with Memptr which is aligned,
		 * then memmove it to the right place.
		 */
		if (sy->sy_flags & SY_WRITE) {
			(*Data_Fill) (Memptr, mem_needed, Pattern,
				      Pattern_Length, 0);
			if (addr != Memptr)
				memmove(addr, Memptr, mem_needed);
		}
	}

	rval = 0;
	got_lock = 0;
	logged_write = 0;

	/*
	 * Lock data if this is a write and locking option is set
	 */
	if (sy->sy_flags & SY_WRITE && k_opt) {
		if (sy->sy_buffer != NULL) {
			(*sy->sy_buffer) (req, offset, 0, &min_byte, &max_byte);
		} else {
			min_byte = offset;
			max_byte = offset + (nbytes * nstrides * nents);
		}

		if (lock_file_region(file, fd, F_WRLCK,
				     min_byte, (max_byte - min_byte + 1)) < 0) {
			doio_fprintf(stderr,
				     "file lock failed:\n%s\n",
				     fmt_ioreq(req, sy, fd));
			doio_fprintf(stderr,
				     "          buffer(req, %d, 0, 0x%x, 0x%x)\n",
				     offset, min_byte, max_byte);
			alloc_mem(-1);
			exit(E_INTERNAL);
		}

		got_lock = 1;
	}

	/*
	 * Write a preliminary write-log entry.  This is done so that
	 * doio_check can do corruption detection across an interrupt/crash.
	 * Note that w_done is set to 0.  If doio_check sees this, it
	 * re-creates the file extents as if the write completed, but does not
	 * do any checking - see comments in doio_check for more details.
	 */

	if (sy->sy_flags & SY_WRITE && w_opt) {
		if (pid == -1) {
			pid = getpid();
		}

		wrec.w_async = (sy->sy_flags & SY_ASYNC) ? 1 : 0;
		wrec.w_oflags = oflags;
		wrec.w_pid = pid;
		wrec.w_offset = offset;
		wrec.w_nbytes = nbytes;	/* mem_needed -- total length */

		wrec.w_pathlen = strlen(file);
		memcpy(wrec.w_path, file, wrec.w_pathlen);
		wrec.w_hostlen = strlen(Host);
		memcpy(wrec.w_host, Host, wrec.w_hostlen);
		wrec.w_patternlen = Pattern_Length;
		memcpy(wrec.w_pattern, Pattern, wrec.w_patternlen);

		wrec.w_done = 0;

		if ((woffset = wlog_record_write(&Wlog, &wrec, -1)) == -1) {
			doio_fprintf(stderr,
				     "Could not append to write-log:  %s (%d)\n",
				     SYSERR, errno);
		} else {
			logged_write = 1;
		}
	}

	s = (*sy->sy_syscall) (req, sy, fd, addr);

	if (s->rval == -1) {
		doio_fprintf(stderr,
			     "%s() request failed:  %s (%d)\n%s\n%s\n",
			     sy->sy_name, SYSERR, errno,
			     fmt_ioreq(req, sy, fd),
			     (*sy->sy_format) (req, sy, fd, addr));

		doio_upanic(U_RVAL);

		for (i = 0; i < nents; i++) {
			if (s->aioid == NULL)
				break;
			aio_unregister(s->aioid[i]);
		}
		rval = -1;
	} else {
		/*
		 * If the syscall was async, wait for I/O to complete
		 */
#ifndef __linux__
		if (sy->sy_flags & SY_ASYNC) {
			for (i = 0; i < nents; i++) {
				aio_wait(s->aioid[i]);
			}
		}
#endif

		/*
		 * Check the syscall how-much-data-written return.  Look
		 * for this in either the return value or the 'iosw'
		 * structure.
		 */

		if (sy->sy_flags & SY_IOSW) {
#ifdef CRAY
			for (i = 0; i < nents; i++) {
				if (s->aioid == NULL)
					break;	/* >>> error condition? */
				aiop = aio_slot(s->aioid[i]);
				iosw = &aiop->iosw;
				if (iosw->sw_error != 0) {
					doio_fprintf(stderr,
						     "%s() iosw error set: %s\n%s\n%s\n",
						     sy->sy_name,
						     strerror(iosw->sw_error),
						     fmt_ioreq(req, sy, fd),
						     (*sy->sy_format) (req, sy,
								       fd,
								       addr));
					doio_upanic(U_IOSW);
					rval = -1;
				} else if (iosw->sw_count != nbytes * nstrides) {
					doio_fprintf(stderr,
						     "Bad iosw from %s() #%d\nExpected (%d,%d,%d), got (%d,%d,%d)\n%s\n%s\n",
						     sy->sy_name, i,
						     1, 0, nbytes * nstrides,
						     iosw->sw_flag,
						     iosw->sw_error,
						     iosw->sw_count,
						     fmt_ioreq(req, sy, fd),
						     (*sy->sy_format) (req, sy,
								       fd,
								       addr));
					doio_upanic(U_IOSW);
					rval = -1;
				}

				aio_unregister(s->aioid[i]);
			}
#endif /* CRAY */
#ifdef sgi
			for (i = 0; s->aioid[i] != -1; i++) {
				if (s->aioid == NULL) {
					doio_fprintf(stderr,
						     "aioid == NULL!\n");
					break;
				}
				aiop = aio_slot(s->aioid[i]);

				/*
				 * make sure the io completed without error
				 */
				if (aiop->aio_errno != 0) {
					doio_fprintf(stderr,
						     "%s() aio error set: %s (%d)\n%s\n%s\n",
						     sy->sy_name,
						     strerror(aiop->aio_errno),
						     aiop->aio_errno,
						     fmt_ioreq(req, sy, fd),
						     (*sy->sy_format) (req, sy,
								       fd,
								       addr));
					doio_upanic(U_IOSW);
					rval = -1;
				} else if (aiop->aio_ret != nbytes) {
					doio_fprintf(stderr,
						     "Bad aio return from %s() #%d\nExpected (%d,%d), got (%d,%d)\n%s\n%s\n",
						     sy->sy_name, i,
						     0, nbytes,
						     aiop->aio_errno,
						     aiop->aio_ret,
						     fmt_ioreq(req, sy, fd),
						     (*sy->sy_format) (req, sy,
								       fd,
								       addr));
					aio_unregister(s->aioid[i]);
					doio_upanic(U_IOSW);
					return -1;
				} else {
					aio_unregister(s->aioid[i]);
					rval = 0;
				}
			}
#endif /* sgi */
		} else {

			if (s->rval != mem_needed) {
				doio_fprintf(stderr,
					     "%s() request returned wrong # of bytes - expected %d, got %d\n%s\n%s\n",
					     sy->sy_name, nbytes, s->rval,
					     fmt_ioreq(req, sy, fd),
					     (*sy->sy_format) (req, sy, fd,
							       addr));
				rval = -1;
				doio_upanic(U_RVAL);
			}
		}
	}

	/*
	 * Verify that the data was written correctly - check_file() returns
	 * a non-null pointer which contains an error message if there are
	 * problems.
	 */

	if (rval == 0 && sy->sy_flags & SY_WRITE && v_opt) {
		msg = check_file(file, offset, nbytes * nstrides * nents,
				 Pattern, Pattern_Length, 0,
				 oflags & O_PARALLEL);
		if (msg != NULL) {
			doio_fprintf(stderr, "%s\n%s\n%s\n",
				     msg,
				     fmt_ioreq(req, sy, fd),
				     (*sy->sy_format) (req, sy, fd, addr));
			doio_upanic(U_CORRUPTION);
			exit(E_COMPARE);
		}
	}

	/*
	 * General cleanup ...
	 *
	 * Write extent information to the write-log, so that doio_check can do
	 * corruption detection.  Note that w_done is set to 1, indicating that
	 * the write has been verified as complete.  We don't need to write the
	 * filename on the second logging.
	 */

	if (w_opt && logged_write) {
		wrec.w_done = 1;
		wlog_record_write(&Wlog, &wrec, woffset);
	}

	/*
	 * Unlock file region if necessary
	 */

	if (got_lock) {
		if (lock_file_region(file, fd, F_UNLCK,
				     min_byte, (max_byte - min_byte + 1)) < 0) {
			alloc_mem(-1);
			exit(E_INTERNAL);
		}
	}

	if (s->aioid != NULL)
		free(s->aioid);
	free(s);
	return (rval == -1) ? -1 : 0;
}

/*
 * fcntl-based requests
 *   - F_FRESVSP
 *   - F_UNRESVSP
 *   - F_FSYNC
 */
#ifdef sgi
int do_fcntl(struct io_req *req)
{
	int fd, oflags, offset, nbytes;
	int rval, op;
	int got_lock;
	int min_byte, max_byte;
	char *file, *msg;
	struct flock flk;

	/*
	 * Initialize common fields - assumes r_oflags, r_file, r_offset, and
	 * r_nbytes are at the same offset in the read_req and reada_req
	 * structures.
	 */
	file = req->r_data.io.r_file;
	oflags = req->r_data.io.r_oflags;
	offset = req->r_data.io.r_offset;
	nbytes = req->r_data.io.r_nbytes;

	flk.l_type = 0;
	flk.l_whence = SEEK_SET;
	flk.l_start = offset;
	flk.l_len = nbytes;

	/*
	 * Get an open file descriptor
	 */

	if ((fd = alloc_fd(file, oflags)) == -1)
		return -1;

	rval = 0;
	got_lock = 0;

	/*
	 * Lock data if this is locking option is set
	 */
	if (k_opt) {
		min_byte = offset;
		max_byte = offset + nbytes;

		if (lock_file_region(file, fd, F_WRLCK,
				     min_byte, (nbytes + 1)) < 0) {
			doio_fprintf(stderr, "file lock failed:\n");
			doio_fprintf(stderr,
				     "          buffer(req, %d, 0, 0x%x, 0x%x)\n",
				     offset, min_byte, max_byte);
			alloc_mem(-1);
			exit(E_INTERNAL);
		}

		got_lock = 1;
	}

	switch (req->r_type) {
	case RESVSP:
		op = F_RESVSP;
		msg = "f_resvsp";
		break;
	case UNRESVSP:
		op = F_UNRESVSP;
		msg = "f_unresvsp";
		break;
#ifdef F_FSYNC
	case DFFSYNC:
		op = F_FSYNC;
		msg = "f_fsync";
		break;
#endif
	}

	rval = fcntl(fd, op, &flk);

	if (rval == -1) {
		doio_fprintf(stderr,
			     "fcntl %s request failed: %s (%d)\n\tfcntl(%d, %s %d, {%d %lld ==> %lld}\n",
			     msg, SYSERR, errno,
			     fd, msg, op, flk.l_whence,
			     (long long)flk.l_start, (long long)flk.l_len);

		doio_upanic(U_RVAL);
		rval = -1;
	}

	/*
	 * Unlock file region if necessary
	 */

	if (got_lock) {
		if (lock_file_region(file, fd, F_UNLCK,
				     min_byte, (max_byte - min_byte + 1)) < 0) {
			alloc_mem(-1);
			exit(E_INTERNAL);
		}
	}

	return (rval == -1) ? -1 : 0;
}
#endif /* sgi */

/*
 *  fsync(2) and fdatasync(2)
 */
#ifndef CRAY
int do_sync(struct io_req *req)
{
	int fd, oflags;
	int rval;
	char *file;

	/*
	 * Initialize common fields - assumes r_oflags, r_file, r_offset, and
	 * r_nbytes are at the same offset in the read_req and reada_req
	 * structures.
	 */
	file = req->r_data.io.r_file;
	oflags = req->r_data.io.r_oflags;

	/*
	 * Get an open file descriptor
	 */

	if ((fd = alloc_fd(file, oflags)) == -1)
		return -1;

	rval = 0;
	switch (req->r_type) {
	case FSYNC2:
		rval = fsync(fd);
		break;
	case FDATASYNC:
		rval = fdatasync(fd);
		break;
	default:
		rval = -1;
	}
	return (rval == -1) ? -1 : 0;
}
#endif /* !CRAY */

int
doio_pat_fill(char *addr, int mem_needed, char *Pattern, int Pattern_Length,
	      int shift)
{
	return pattern_fill(addr, mem_needed, Pattern, Pattern_Length, 0);
}

char *doio_pat_check(char *buf, int offset, int length, char *pattern,
		     int pattern_length, int patshift)
{
	static char errbuf[4096];
	int nb, i, pattern_index;
	char *cp, *bufend, *ep;
	char actual[33], expected[33];

	if (pattern_check(buf, length, pattern, pattern_length, patshift) != 0) {
		ep = errbuf;
		ep +=
		    sprintf(ep,
			    "Corrupt regions follow - unprintable chars are represented as '.'\n");
		ep +=
		    sprintf(ep,
			    "-----------------------------------------------------------------\n");

		pattern_index = patshift % pattern_length;;
		cp = buf;
		bufend = buf + length;

		while (cp < bufend) {
			if (*cp != pattern[pattern_index]) {
				nb = bufend - cp;
				if ((unsigned int)nb > sizeof(expected) - 1) {
					nb = sizeof(expected) - 1;
				}

				ep +=
				    sprintf(ep,
					    "corrupt bytes starting at file offset %d\n",
					    offset + (int)(cp - buf));

				/*
				 * Fill in the expected and actual patterns
				 */
				memset(expected, 0x00, sizeof(expected));
				memset(actual, 0x00, sizeof(actual));

				for (i = 0; i < nb; i++) {
					expected[i] =
					    pattern[(pattern_index +
						     i) % pattern_length];
					if (!isprint(expected[i])) {
						expected[i] = '.';
					}

					actual[i] = cp[i];
					if (!isprint(actual[i])) {
						actual[i] = '.';
					}
				}

				ep +=
				    sprintf(ep,
					    "    1st %2d expected bytes:  %s\n",
					    nb, expected);
				ep +=
				    sprintf(ep,
					    "    1st %2d actual bytes:    %s\n",
					    nb, actual);
				fflush(stderr);
				return errbuf;
			} else {
				cp++;
				pattern_index++;

				if (pattern_index == pattern_length) {
					pattern_index = 0;
				}
			}
		}
		return errbuf;
	}

	return NULL;
}

/*
 * Check the contents of a file beginning at offset, for length bytes.  It
 * is assumed that there is a string of pattern bytes in this area of the
 * file.  Use normal buffered reads to do the verification.
 *
 * If there is a data mismatch, write a detailed message into a static buffer
 * suitable for the caller to print.  Otherwise print NULL.
 *
 * The fsa flag is set to non-zero if the buffer should be read back through
 * the FSA (unicos/mk).  This implies the file will be opened
 * O_PARALLEL|O_RAW|O_WELLFORMED to do the validation.  We must do this because
 * FSA will not allow the file to be opened for buffered io if it was
 * previously opened for O_PARALLEL io.
 */

char *check_file(char *file, int offset, int length, char *pattern,
		 int pattern_length, int patshift, int fsa)
{
	static char errbuf[4096];
	int fd, nb, flags;
	char *buf, *em, *ep;
#ifdef sgi
	struct fd_cache *fdc;
#endif

	buf = Memptr;

	if (V_opt) {
		flags = Validation_Flags | O_RDONLY;
	} else {
		flags = O_RDONLY;
		if (fsa) {
#ifdef CRAY
			flags |= O_PARALLEL | O_RAW | O_WELLFORMED;
#endif
		}
	}

	if ((fd = alloc_fd(file, flags)) == -1) {
		sprintf(errbuf,
			"Could not open file %s with flags %#o (%s) for data comparison:  %s (%d)\n",
			file, flags, format_oflags(flags), SYSERR, errno);
		return errbuf;
	}

	if (lseek(fd, offset, SEEK_SET) == -1) {
		sprintf(errbuf,
			"Could not lseek to offset %d in %s for verification:  %s (%d)\n",
			offset, file, SYSERR, errno);
		return errbuf;
	}
#ifdef sgi
	/* Irix: Guarantee a properly aligned address on Direct I/O */
	fdc = alloc_fdcache(file, flags);
	if ((flags & O_DIRECT) && ((long)buf % fdc->c_memalign != 0)) {
		buf += fdc->c_memalign - ((long)buf % fdc->c_memalign);
	}
#endif

	if ((nb = read(fd, buf, length)) == -1) {
#ifdef sgi
		sprintf(errbuf,
			"Could not read %d bytes from %s for verification:  %s (%d)\n\tread(%d, 0x%lx, %d)\n\tbuf %% alignment(%d) = %ld\n",
			length, file, SYSERR, errno,
			fd, buf, length,
			fdc->c_memalign, (long)buf % fdc->c_memalign);
#else
		sprintf(errbuf,
			"Could not read %d bytes from %s for verification:  %s (%d)\n",
			length, file, SYSERR, errno);

#endif
		return errbuf;
	}

	if (nb != length) {
		sprintf(errbuf,
			"Read wrong # bytes from %s.  Expected %d, got %d\n",
			file, length, nb);
		return errbuf;
	}

	if ((em =
	     (*Data_Check) (buf, offset, length, pattern, pattern_length,
			    patshift)) != NULL) {
		ep = errbuf;
		ep += sprintf(ep, "*** DATA COMPARISON ERROR ***\n");
		ep +=
		    sprintf(ep, "check_file(%s, %d, %d, %s, %d, %d) failed\n\n",
			    file, offset, length, pattern, pattern_length,
			    patshift);
		ep +=
		    sprintf(ep, "Comparison fd is %d, with open flags %#o\n",
			    fd, flags);
		strcpy(ep, em);
		return (errbuf);
	}
	return NULL;
}

/*
 * Function to single-thread stdio output.
 */

int doio_fprintf(FILE * stream, char *format, ...)
{
	static int pid = -1;
	char *date;
	int rval;
	struct flock flk;
	va_list arglist;
	struct timeval ts;
	gettimeofday(&ts, NULL);
	date = hms(ts.tv_sec);

	if (pid == -1) {
		pid = getpid();
	}

	flk.l_whence = flk.l_start = flk.l_len = 0;
	flk.l_type = F_WRLCK;
	fcntl(fileno(stream), F_SETLKW, &flk);

	va_start(arglist, format);
	rval = fprintf(stream, "\n%s%s (%5d) %s\n", Prog, TagName, pid, date);
	rval += fprintf(stream, "---------------------\n");
	vfprintf(stream, format, arglist);
	va_end(arglist);

	fflush(stream);

	flk.l_type = F_UNLCK;
	fcntl(fileno(stream), F_SETLKW, &flk);

	return rval;
}

/*
 * Simple function for allocating core memory.  Uses Memsize and Memptr to
 * keep track of the current amount allocated.
 */
#ifndef CRAY
int alloc_mem(int nbytes)
{
	char *cp;
	void *addr;
	int me = 0, flags, key, shmid;
	static int mturn = 0;	/* which memory type to use */
	struct memalloc *M;
	char filename[255];
#ifdef __linux__
	struct shmid_ds shm_ds;
#endif

#ifdef __linux__
	memset(&shm_ds, 0x00, sizeof(struct shmid_ds));
#endif

	/* nbytes = -1 means "free all allocated memory" */
	if (nbytes == -1) {

		for (me = 0; me < Nmemalloc; me++) {
			if (Memalloc[me].space == NULL)
				continue;

			switch (Memalloc[me].memtype) {
			case MEM_DATA:
#ifdef sgi
				if (Memalloc[me].flags & MEMF_MPIN)
					munpin(Memalloc[me].space,
					       Memalloc[me].size);
#endif
				free(Memalloc[me].space);
				Memalloc[me].space = NULL;
				Memptr = NULL;
				Memsize = 0;
				break;
			case MEM_SHMEM:
#ifdef sgi
				if (Memalloc[me].flags & MEMF_MPIN)
					munpin(Memalloc[me].space,
					       Memalloc[me].size);
#endif
				shmdt(Memalloc[me].space);
				Memalloc[me].space = NULL;
#ifdef sgi
				shmctl(Memalloc[me].fd, IPC_RMID);
#else
				shmctl(Memalloc[me].fd, IPC_RMID, &shm_ds);
#endif
				break;
			case MEM_MMAP:
#ifdef sgi
				if (Memalloc[me].flags & MEMF_MPIN)
					munpin(Memalloc[me].space,
					       Memalloc[me].size);
#endif
				munmap(Memalloc[me].space, Memalloc[me].size);
				close(Memalloc[me].fd);
				if (Memalloc[me].flags & MEMF_FILE) {
					unlink(Memalloc[me].name);
				}
				Memalloc[me].space = NULL;
				break;
			default:
				doio_fprintf(stderr,
					     "alloc_mem: HELP! Unknown memory space type %d index %d\n",
					     Memalloc[me].memtype, me);
				break;
			}
		}
		return 0;
	}

	/*
	 * Select a memory area (currently round-robbin)
	 */

	if (mturn >= Nmemalloc)
		mturn = 0;

	M = &Memalloc[mturn];

	switch (M->memtype) {
	case MEM_DATA:
		if (nbytes > M->size) {
			if (M->space != NULL) {
#ifdef sgi
				if (M->flags & MEMF_MPIN)
					munpin(M->space, M->size);
#endif
				free(M->space);
			}
			M->space = NULL;
			M->size = 0;
		}

		if (M->space == NULL) {
			if ((cp = malloc(nbytes)) == NULL) {
				doio_fprintf(stderr,
					     "malloc(%d) failed:  %s (%d)\n",
					     nbytes, SYSERR, errno);
				return -1;
			}
#ifdef sgi
			if (M->flags & MEMF_MPIN) {
				if (mpin(cp, nbytes) == -1) {
					doio_fprintf(stderr,
						     "mpin(0x%lx, %d) failed:  %s (%d)\n",
						     cp, nbytes, SYSERR, errno);
				}
			}
#endif
			M->space = (void *)cp;
			M->size = nbytes;
		}
		break;

	case MEM_MMAP:
		if (nbytes > M->size) {
			if (M->space != NULL) {
#ifdef sgi
				if (M->flags & MEMF_MPIN)
					munpin(M->space, M->size);
#endif
				munmap(M->space, M->size);
				close(M->fd);
				if (M->flags & MEMF_FILE)
					unlink(M->name);
			}
			M->space = NULL;
			M->size = 0;
		}

		if (M->space == NULL) {
			if (strchr(M->name, '%')) {
				sprintf(filename, M->name, getpid());
				M->name = strdup(filename);
			}

			if ((M->fd =
			     open(M->name, O_CREAT | O_RDWR, 0666)) == -1) {
				doio_fprintf(stderr,
					     "alloc_mmap: error %d (%s) opening '%s'\n",
					     errno, SYSERR, M->name);
				return (-1);
			}

			addr = NULL;
			flags = 0;
			M->size = nbytes * 4;

			/* bias addr if MEMF_ADDR | MEMF_FIXADDR */
			/* >>> how to pick a memory address? */

			/* bias flags on MEMF_PRIVATE etc */
			if (M->flags & MEMF_PRIVATE)
				flags |= MAP_PRIVATE;
#ifdef sgi
			if (M->flags & MEMF_LOCAL)
				flags |= MAP_LOCAL;
			if (M->flags & MEMF_AUTORESRV)
				flags |= MAP_AUTORESRV;
			if (M->flags & MEMF_AUTOGROW)
				flags |= MAP_AUTOGROW;
#endif
			if (M->flags & MEMF_SHARED)
				flags |= MAP_SHARED;

/*printf("alloc_mem, about to mmap, fd=%d, name=(%s)\n", M->fd, M->name);*/
			if ((M->space = mmap(addr, M->size,
					     PROT_READ | PROT_WRITE,
					     flags, M->fd, 0))
			    == MAP_FAILED) {
				doio_fprintf(stderr,
					     "alloc_mem: mmap error. errno %d (%s)\n\tmmap(addr 0x%x, size %d, read|write 0x%x, mmap flags 0x%x [%#o], fd %d, 0)\n\tfile %s\n",
					     errno, SYSERR, addr, M->size,
					     PROT_READ | PROT_WRITE, flags,
					     M->flags, M->fd, M->name);
				doio_fprintf(stderr, "\t%s%s%s%s%s",
					     (flags & MAP_PRIVATE) ? "private "
					     : "",
#ifdef sgi
					     (flags & MAP_LOCAL) ? "local " :
					     "",
					     (flags & MAP_AUTORESRV) ?
					     "autoresrv " : "",
					     (flags & MAP_AUTOGROW) ?
					     "autogrow " : "",
#endif
					     (flags & MAP_SHARED) ? "shared" :
					     "");
				return (-1);
			}
		}
		break;

	case MEM_SHMEM:
		if (nbytes > M->size) {
			if (M->space != NULL) {
#ifdef sgi
				if (M->flags & MEMF_MPIN)
					munpin(M->space, M->size);
#endif
				shmdt(M->space);
#ifdef sgi
				shmctl(M->fd, IPC_RMID);
#else
				shmctl(M->fd, IPC_RMID, &shm_ds);
#endif
			}
			M->space = NULL;
			M->size = 0;
		}

		if (M->space == NULL) {
			if (!strcmp(M->name, "private")) {
				key = IPC_PRIVATE;
			} else {
				sscanf(M->name, "%i", &key);
			}

			M->size = M->nblks ? M->nblks * 512 : nbytes;

			if (nbytes > M->size) {
#ifdef DEBUG
				doio_fprintf(stderr,
					     "MEM_SHMEM: nblks(%d) too small:  nbytes=%d  Msize=%d, skipping this req.\n",
					     M->nblks, nbytes, M->size);
#endif
				return SKIP_REQ;
			}

			shmid = shmget(key, M->size, IPC_CREAT | 0666);
			if (shmid == -1) {
				doio_fprintf(stderr,
					     "shmget(0x%x, %d, CREAT) failed: %s (%d)\n",
					     key, M->size, SYSERR, errno);
				return (-1);
			}
			M->fd = shmid;
			M->space = shmat(shmid, NULL, SHM_RND);
			if (M->space == (void *)-1) {
				doio_fprintf(stderr,
					     "shmat(0x%x, NULL, SHM_RND) failed: %s (%d)\n",
					     shmid, SYSERR, errno);
				return (-1);
			}
#ifdef sgi
			if (M->flags & MEMF_MPIN) {
				if (mpin(M->space, M->size) == -1) {
					doio_fprintf(stderr,
						     "mpin(0x%lx, %d) failed:  %s (%d)\n",
						     M->space, M->size, SYSERR,
						     errno);
				}
			}
#endif
		}
		break;

	default:
		doio_fprintf(stderr,
			     "alloc_mem: HELP! Unknown memory space type %d index %d\n",
			     Memalloc[me].memtype, mturn);
		break;
	}

	Memptr = M->space;
	Memsize = M->size;

	mturn++;
	return 0;
}
#else /* CRAY */
int alloc_mem(int nbytes)
{
	char *cp;
	int ip;
	static char *malloc_space;

	/*
	 * The "unicos" version of this did some stuff with sbrk;
	 * this caused problems with async I/O on irix, and now appears
	 * to be causing problems with FSA I/O on unicos/mk.
	 */
#ifdef NOTDEF
	if (nbytes > Memsize) {
		if ((cp = (char *)sbrk(nbytes - Memsize)) == (char *)-1) {
			doio_fprintf(stderr, "sbrk(%d) failed:  %s (%d)\n",
				     nbytes - Memsize, SYSERR, errno);
			return -1;
		}

		if (Memsize == 0)
			Memptr = cp;
		Memsize += nbytes - Memsize;
	}
#else

	/* nbytes = -1 means "free all allocated memory" */
	if (nbytes == -1) {
		free(malloc_space);
		Memptr = NULL;
		Memsize = 0;
		return 0;
	}

	if (nbytes > Memsize) {
		if (Memsize != 0)
			free(malloc_space);

		if ((cp = malloc_space = malloc(nbytes)) == NULL) {
			doio_fprintf(stderr, "malloc(%d) failed:  %s (%d)\n",
				     nbytes, SYSERR, errno);
			return -1;
		}
#ifdef _CRAYT3E
		/* T3E requires memory to be aligned on 0x40 word boundaries */
		ip = (int)cp;
		if ((ip & 0x3F) != 0) {
			doio_fprintf(stderr,
				     "malloc(%d) = 0x%x(0x%x) not aligned by 0x%x\n",
				     nbytes, cp, ip, ip & 0x3f);

			free(cp);
			if ((cp = malloc_space = malloc(nbytes + 0x40)) == NULL) {
				doio_fprintf(stderr,
					     "malloc(%d) failed:  %s (%d)\n",
					     nbytes, SYSERR, errno);
				return -1;
			}
			ip = (int)cp;
			cp += (0x40 - (ip & 0x3F));
		}
#endif /* _CRAYT3E */
		Memptr = cp;
		Memsize = nbytes;
	}
#endif /* NOTDEF */
	return 0;
}
#endif /* CRAY */

/*
 * Simple function for allocating sds space.  Uses Sdssize and Sdsptr to
 * keep track of location and size of currently allocated chunk.
 */

#ifdef _CRAY1

int alloc_sds(int nbytes)
{
	int nblks;

	if (nbytes > Sdssize) {
		if ((nblks = ssbreak(btoc(nbytes - Sdssize))) == -1) {
			doio_fprintf(stderr, "ssbreak(%d) failed:  %s (%d)\n",
				     btoc(nbytes - Sdssize), SYSERR, errno);
			return -1;
		}

		Sdssize = ctob(nblks);
		Sdsptr = 0;
	}

	return 0;
}

#else

#ifdef CRAY

int alloc_sds(int nbytes)
{
	doio_fprintf(stderr,
		     "Internal Error - alloc_sds() called on a CRAY2 system\n");
	alloc_mem(-1);
	exit(E_INTERNAL);
}

#endif

#endif /* _CRAY1 */

/*
 * Function to maintain a file descriptor cache, so that doio does not have
 * to do so many open() and close() calls.  Descriptors are stored in the
 * cache by file name, and open flags.  Each entry also has a _rtc value
 * associated with it which is used in aging.  If doio cannot open a file
 * because it already has too many open (ie. system limit hit) it will close
 * the one in the cache that has the oldest _rtc value.
 *
 * If alloc_fd() is called with a file of NULL, it will close all descriptors
 * in the cache, and free the memory in the cache.
 */

int alloc_fd(char *file, int oflags)
{
	struct fd_cache *fdc;
	struct fd_cache *alloc_fdcache(char *file, int oflags);

	fdc = alloc_fdcache(file, oflags);
	if (fdc != NULL)
		return (fdc->c_fd);
	else
		return (-1);
}

struct fd_cache *alloc_fdcache(char *file, int oflags)
{
	int fd;
	struct fd_cache *free_slot, *oldest_slot, *cp;
	static int cache_size = 0;
	static struct fd_cache *cache = NULL;
#ifdef sgi
	struct dioattr finfo;
#endif

	/*
	 * If file is NULL, it means to free up the fd cache.
	 */

	if (file == NULL && cache != NULL) {
		for (cp = cache; cp < &cache[cache_size]; cp++) {
			if (cp->c_fd != -1) {
				close(cp->c_fd);
			}
#ifndef CRAY
			if (cp->c_memaddr != NULL) {
				munmap(cp->c_memaddr, cp->c_memlen);
			}
#endif
		}

		free(cache);
		cache = NULL;
		cache_size = 0;
		return 0;
	}

	free_slot = NULL;
	oldest_slot = NULL;

	/*
	 * Look for a fd in the cache.  If one is found, return it directly.
	 * Otherwise, when this loop exits, oldest_slot will point to the
	 * oldest fd slot in the cache, and free_slot will point to an
	 * unoccupied slot if there are any.
	 */

	for (cp = cache; cp != NULL && cp < &cache[cache_size]; cp++) {
		if (cp->c_fd != -1 &&
		    cp->c_oflags == oflags && strcmp(cp->c_file, file) == 0) {
#ifdef CRAY
			cp->c_rtc = _rtc();
#else
			cp->c_rtc = Reqno;
#endif
			return cp;
		}

		if (cp->c_fd == -1) {
			if (free_slot == NULL) {
				free_slot = cp;
			}
		} else {
			if (oldest_slot == NULL ||
			    cp->c_rtc < oldest_slot->c_rtc) {
				oldest_slot = cp;
			}
		}
	}

	/*
	 * No matching file/oflags pair was found in the cache.  Attempt to
	 * open a new fd.
	 */

	if ((fd = open(file, oflags, 0666)) < 0) {
		if (errno != EMFILE) {
			doio_fprintf(stderr,
				     "Could not open file %s with flags %#o (%s): %s (%d)\n",
				     file, oflags, format_oflags(oflags),
				     SYSERR, errno);
			alloc_mem(-1);
			exit(E_SETUP);
		}

		/*
		 * If we get here, we have as many open fd's as we can have.
		 * Close the oldest one in the cache (pointed to by
		 * oldest_slot), and attempt to re-open.
		 */

		close(oldest_slot->c_fd);
		oldest_slot->c_fd = -1;
		free_slot = oldest_slot;

		if ((fd = open(file, oflags, 0666)) < 0) {
			doio_fprintf(stderr,
				     "Could not open file %s with flags %#o (%s):  %s (%d)\n",
				     file, oflags, format_oflags(oflags),
				     SYSERR, errno);
			alloc_mem(-1);
			exit(E_SETUP);
		}
	}

/*printf("alloc_fd: new file %s flags %#o fd %d\n", file, oflags, fd);*/

	/*
	 * If we get here, fd is our open descriptor.  If free_slot is NULL,
	 * we need to grow the cache, otherwise free_slot is the slot that
	 * should hold the fd info.
	 */

	if (free_slot == NULL) {
		cache =
		    (struct fd_cache *)realloc(cache,
					       sizeof(struct fd_cache) *
					       (FD_ALLOC_INCR + cache_size));
		if (cache == NULL) {
			doio_fprintf(stderr,
				     "Could not malloc() space for fd chace");
			alloc_mem(-1);
			exit(E_SETUP);
		}

		cache_size += FD_ALLOC_INCR;

		for (cp = &cache[cache_size - FD_ALLOC_INCR];
		     cp < &cache[cache_size]; cp++) {
			cp->c_fd = -1;
		}

		free_slot = &cache[cache_size - FD_ALLOC_INCR];
	}

	/*
	 * finally, fill in the cache slot info
	 */

	free_slot->c_fd = fd;
	free_slot->c_oflags = oflags;
	strcpy(free_slot->c_file, file);
#ifdef CRAY
	free_slot->c_rtc = _rtc();
#else
	free_slot->c_rtc = Reqno;
#endif

#ifdef sgi
	if (oflags & O_DIRECT) {
		if (fcntl(fd, F_DIOINFO, &finfo) == -1) {
			finfo.d_mem = 1;
			finfo.d_miniosz = 1;
			finfo.d_maxiosz = 1;
		}
	} else {
		finfo.d_mem = 1;
		finfo.d_miniosz = 1;
		finfo.d_maxiosz = 1;
	}

	free_slot->c_memalign = finfo.d_mem;
	free_slot->c_miniosz = finfo.d_miniosz;
	free_slot->c_maxiosz = finfo.d_maxiosz;
#endif /* sgi */
#ifndef CRAY
	free_slot->c_memaddr = NULL;
	free_slot->c_memlen = 0;
#endif

	return free_slot;
}

/*
 *
 *			Signal Handling Section
 *
 *
 */

#ifdef sgi
/*
 * "caller-id" for signals
 */
void signal_info(int sig, siginfo_t * info, void *v)
{
	int haveit = 0;

	if (info != NULL) {
		switch (info->si_code) {
		case SI_USER:
			doio_fprintf(stderr,
				     "signal_info: si_signo %d si_errno %d si_code SI_USER pid %d uid %d\n",
				     info->si_signo, info->si_errno,
				     info->si_pid, info->si_uid);
			haveit = 1;
			break;

		case SI_QUEUE:
			doio_fprintf(stderr,
				     "signal_info  si_signo %d si_code = SI_QUEUE\n",
				     info->si_signo);
			haveit = 1;
			break;
		}

		if (!haveit) {
			if ((info->si_signo == SIGSEGV) ||
			    (info->si_signo == SIGBUS)) {
				doio_fprintf(stderr,
					     "signal_info  si_signo %d si_errno %d si_code = %d  si_addr=%p  active_mmap_rw=%d havesigint=%d\n",
					     info->si_signo, info->si_errno,
					     info->si_code, info->si_addr,
					     active_mmap_rw, havesigint);
				haveit = 1;
			}
		}

		if (!haveit) {
			doio_fprintf(stderr,
				     "signal_info: si_signo %d si_errno %d unknown code %d\n",
				     info->si_signo, info->si_errno,
				     info->si_code);
		}
	} else {
		doio_fprintf(stderr, "signal_info: sig %d\n", sig);
	}
}

void cleanup_handler(int sig, siginfo_t * info, void *v)
{
	havesigint = 1;		/* in case there's a followup signal */
	/*signal_info(sig, info, v); *//* be quiet on "normal" kill */
	alloc_mem(-1);
	exit(0);
}

void die_handler(int sig, siginfo_t * info, void *v)
{
	doio_fprintf(stderr, "terminating on signal %d\n", sig);
	signal_info(sig, info, v);
	alloc_mem(-1);
	exit(1);
}

void sigbus_handler(int sig, siginfo_t * info, void *v)
{
	/* While we are doing a memcpy to/from an mmapped region we can
	   get a SIGBUS for a variety of reasons--and not all of them
	   should be considered failures.

	   Under normal conditions if we get a SIGINT it means we've been
	   told to shutdown.  However, if we're currently doing the above-
	   mentioned memcopy then the kernel will follow that SIGINT with
	   a SIGBUS.  We can guess that we're in this situation by seeing
	   that the si_errno field in the siginfo structure has EINTR as
	   an errno.  (We might make the guess stronger by looking at the
	   si_addr field to see that it's not faulting off the end of the
	   mmapped region, but it seems that in such a case havesigint
	   would not have been set so maybe that doesn't make the guess
	   stronger.)
	 */

	if (active_mmap_rw && havesigint && (info->si_errno == EINTR)) {
		cleanup_handler(sig, info, v);
	} else {
		die_handler(sig, info, v);
	}
}
#else

void cleanup_handler(int sig)
{
	havesigint = 1;		/* in case there's a followup signal */
	alloc_mem(-1);
	exit(0);
}

void die_handler(int sig)
{
	doio_fprintf(stderr, "terminating on signal %d\n", sig);
	alloc_mem(-1);
	exit(1);
}

#ifndef CRAY
void sigbus_handler(int sig)
{
	/* See sigbus_handler() in the 'ifdef sgi' case for details.  Here,
	   we don't have the siginfo stuff so the guess is weaker but we'll
	   do it anyway.
	 */

	if (active_mmap_rw && havesigint)
		cleanup_handler(sig);
	else
		die_handler(sig);
}
#endif /* !CRAY */
#endif /* sgi */

void noop_handler(int sig)
{
	return;
}

/*
 * SIGINT handler for the parent (original doio) process.  It simply sends
 * a SIGINT to all of the doio children.  Since they're all in the same
 * pgrp, this can be done with a single kill().
 */

void sigint_handler(int sig)
{
	int i;

	for (i = 0; i < Nchildren; i++) {
		if (Children[i] != -1) {
			kill(Children[i], SIGINT);
		}
	}
}

/*
 * Signal handler used to inform a process when async io completes.  Referenced
 * in do_read() and do_write().  Note that the signal handler is not
 * re-registered.
 */

void aio_handler(int sig)
{
	unsigned int i;
	struct aio_info *aiop;

	for (i = 0; i < sizeof(Aio_Info) / sizeof(Aio_Info[0]); i++) {
		aiop = &Aio_Info[i];

		if (aiop->strategy == A_SIGNAL && aiop->sig == sig) {
			aiop->signalled++;

			if (aio_done(aiop)) {
				aiop->done++;
			}
		}
	}
}

/*
 * dump info on all open aio slots
 */
void dump_aio(void)
{
	unsigned int i, count;

	count = 0;
	for (i = 0; i < sizeof(Aio_Info) / sizeof(Aio_Info[0]); i++) {
		if (Aio_Info[i].busy) {
			count++;
			fprintf(stderr,
				"Aio_Info[%03d] id=%d fd=%d signal=%d signaled=%d\n",
				i, Aio_Info[i].id,
				Aio_Info[i].fd,
				Aio_Info[i].sig, Aio_Info[i].signalled);
			fprintf(stderr, "\tstrategy=%s\n",
				format_strat(Aio_Info[i].strategy));
		}
	}
	fprintf(stderr, "%d active async i/os\n", count);
}

#ifdef sgi
/*
 * Signal handler called as a callback, not as a signal.
 * 'val' is the value from sigev_value and is assumed to be the
 * Aio_Info[] index.
 */
void cb_handler(sigval_t val)
{
	struct aio_info *aiop;

/*printf("cb_handler requesting slot %d\n", val.sival_int);*/
	aiop = aio_slot(val.sival_int);
/*printf("cb_handler, aiop=%p\n", aiop);*/

/*printf("%d in cb_handler\n", getpid() );*/
	if (aiop->strategy == A_CALLBACK) {
		aiop->signalled++;

		if (aio_done(aiop)) {
			aiop->done++;
		}
	}
}
#endif

struct aio_info *aio_slot(int aio_id)
{
	unsigned int i;
	static int id = 1;
	struct aio_info *aiop;

	aiop = NULL;

	for (i = 0; i < sizeof(Aio_Info) / sizeof(Aio_Info[0]); i++) {
		if (aio_id == -1) {
			if (!Aio_Info[i].busy) {
				aiop = &Aio_Info[i];
				aiop->busy = 1;
				aiop->id = id++;
				break;
			}
		} else {
			if (Aio_Info[i].busy && Aio_Info[i].id == aio_id) {
				aiop = &Aio_Info[i];
				break;
			}
		}
	}

	if (aiop == NULL) {
		doio_fprintf(stderr, "aio_slot(%d) not found.  Request %d\n",
			     aio_id, Reqno);
		dump_aio();
		alloc_mem(-1);
		exit(E_INTERNAL);
	}

	return aiop;
}

int aio_register(int fd, int strategy, int sig)
{
	struct aio_info *aiop;
	struct sigaction sa;

	aiop = aio_slot(-1);

	aiop->fd = fd;
	aiop->strategy = strategy;
	aiop->done = 0;
#ifdef CRAY
	memset((char *)&aiop->iosw, 0x00, sizeof(aiop->iosw));
#endif

	if (strategy == A_SIGNAL) {
		aiop->sig = sig;
		aiop->signalled = 0;

		sa.sa_handler = aio_handler;
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);

		sigaction(sig, &sa, &aiop->osa);
	} else {
		aiop->sig = -1;
		aiop->signalled = 0;
	}

	return aiop->id;
}

int aio_unregister(int aio_id)
{
	struct aio_info *aiop;

	aiop = aio_slot(aio_id);

	if (aiop->strategy == A_SIGNAL) {
		sigaction(aiop->sig, &aiop->osa, NULL);
	}

	aiop->busy = 0;
	return 0;
}

#ifndef __linux__
int aio_wait(int aio_id)
{
#ifdef RECALL_SIZEOF
	long mask[RECALL_SIZEOF];
#endif
	sigset_t signalset;
	struct aio_info *aiop;
#ifdef CRAY
	struct iosw *ioswlist[1];
#endif
#ifdef sgi
	const aiocb_t *aioary[1];
#endif
	int r, cnt;

	aiop = aio_slot(aio_id);
/*printf("%d aiop B =%p\n", getpid(), aiop);*/

	switch (aiop->strategy) {
	case A_POLL:
		while (!aio_done(aiop)) ;
		break;

	case A_SIGNAL:
		sigemptyset(&signalset);
		sighold(aiop->sig);

		while (!aiop->signalled || !aiop->done) {
			sigsuspend(&signalset);
			sighold(aiop->sig);
		}
		break;

#ifdef CRAY
	case A_RECALL:
		ioswlist[0] = &aiop->iosw;
		if (recall(aiop->fd, 1, ioswlist) < 0) {
			doio_fprintf(stderr, "recall() failed:  %s (%d)\n",
				     SYSERR, errno);
			exit(E_SETUP);
		}
		break;

#ifdef RECALL_SIZEOF

	case A_RECALLA:
		RECALL_INIT(mask);
		RECALL_SET(mask, aiop->fd);
		if (recalla(mask) < 0) {
			doio_fprintf(stderr, "recalla() failed:  %s (%d)\n",
				     SYSERR, errno);
			exit(E_SETUP);
		}

		RECALL_CLR(mask, aiop->fd);
		break;
#endif

	case A_RECALLS:
		ioswlist[0] = &aiop->iosw;
		if (recalls(1, ioswlist) < 0) {
			doio_fprintf(stderr, "recalls failed:  %s (%d)\n",
				     SYSERR, errno);
			exit(E_SETUP);
		}
		break;
#endif /* CRAY */

#ifdef sgi
	case A_CALLBACK:
		aioary[0] = &aiop->aiocb;
		cnt = 0;
		do {
			r = aio_suspend(aioary, 1, NULL);
			if (r == -1) {
				doio_fprintf(stderr,
					     "aio_suspend failed: %s (%d)\n",
					     SYSERR, errno);
				exit(E_SETUP);
			}
			cnt++;
		} while (aiop->done == 0);

#if 0
		/*
		 * after having this set for a while, I've decided that
		 * it's too noisy
		 */
		if (cnt > 1)
			doio_fprintf(stderr,
				     "aio_wait: callback wait took %d tries\n",
				     cnt);
#endif

		/*
		 * Note: cb_handler already calls aio_done
		 */
		break;

	case A_SUSPEND:
		aioary[0] = &aiop->aiocb;
		r = aio_suspend(aioary, 1, NULL);
		if (r == -1) {
			doio_fprintf(stderr, "aio_suspend failed: %s (%d)\n",
				     SYSERR, errno);
			exit(E_SETUP);
		}

		aio_done(aiop);
		break;
#endif
	}

/*printf("aio_wait: errno %d return %d\n", aiop->aio_errno, aiop->aio_ret);*/

	return 0;
}
#endif /* !linux */

/*
 * Format specified time into HH:MM:SS format.  t is the time to format
 * in seconds (as returned from time(2)).
 */

char *hms(time_t t)
{
	static char ascii_time[9];
	struct tm *ltime;

	ltime = localtime(&t);
	strftime(ascii_time, sizeof(ascii_time), "%H:%M:%S", ltime);

	return ascii_time;
}

/*
 * Simple routine to check if an async io request has completed.
 */

int aio_done(struct aio_info *ainfo)
{
#ifdef CRAY
	return ainfo->iosw.sw_flag;
#endif

#ifdef sgi
	if ((ainfo->aio_errno = aio_error(&ainfo->aiocb)) == -1) {
		doio_fprintf(stderr, "aio_done: aio_error failed: %s (%d)\n",
			     SYSERR, errno);
		exit(E_SETUP);
	}
	/*printf("%d aio_done aio_errno=%d\n", getpid(), ainfo->aio_errno); */
	if (ainfo->aio_errno != EINPROGRESS) {
		if ((ainfo->aio_ret = aio_return(&ainfo->aiocb)) == -1) {
			doio_fprintf(stderr,
				     "aio_done: aio_return failed: %s (%d)\n",
				     SYSERR, errno);
			exit(E_SETUP);
		}
	}

	return (ainfo->aio_errno != EINPROGRESS);
#else
	return -1;		/* invalid */
#endif
}

/*
 * Routine to handle upanic() - it first attempts to set the panic flag.  If
 * the flag cannot be set, an error message is issued.  A call to upanic
 * with PA_PANIC is then done unconditionally, in case the panic flag was set
 * from outside the program (as with the panic(8) program).
 *
 * Note - we only execute the upanic code if -U was used, and the passed in
 * mask is set in the Upanic_Conditions bitmask.
 */

void doio_upanic(int mask)
{
	if (U_opt == 0 || (mask & Upanic_Conditions) == 0) {
		return;
	}
#ifdef CRAY
	if (upanic(PA_SET) < 0) {
		doio_fprintf(stderr,
			     "WARNING - Could not set the panic flag - upanic(PA_SET) failed:  %s (%d)\n",
			     SYSERR, errno);
	}

	upanic(PA_PANIC);
#endif
#ifdef sgi
	syssgi(1005);		/* syssgi test panic - DEBUG kernels only */
#endif
	doio_fprintf(stderr, "WARNING - upanic() failed\n");
}

/*
 * Parse cmdline options/arguments and set appropriate global variables.
 * If the cmdline is valid, return 0 to caller.  Otherwise exit with a status
 * of 1.
 */

int parse_cmdline(int argc, char **argv, char *opts)
{
	int c;
	char cc, *cp = NULL, *tok = NULL;
	extern int opterr;
	extern int optind;
	extern char *optarg;
	struct smap *s;
	char *memargs[NMEMALLOC];
	int nmemargs, ma;

	if (*argv[0] == '-') {
		argv[0]++;
		Execd = 1;
	}

	if ((Prog = strrchr(argv[0], '/')) == NULL) {
		Prog = argv[0];
	} else {
		Prog++;
	}

	opterr = 0;
	while ((c = getopt(argc, argv, opts)) != EOF) {
		switch ((char)c) {
		case 'a':
			a_opt++;
			break;

		case 'C':
			C_opt++;
			for (s = checkmap; s->string != NULL; s++)
				if (!strcmp(s->string, optarg))
					break;
			if (s->string == NULL && tok != NULL) {
				fprintf(stderr,
					"%s%s:  Illegal -C arg (%s).  Must be one of: ",
					Prog, TagName, tok);

				for (s = checkmap; s->string != NULL; s++)
					fprintf(stderr, "%s ", s->string);
				fprintf(stderr, "\n");
				exit(1);
			}

			switch (s->value) {
			case C_DEFAULT:
				Data_Fill = doio_pat_fill;
				Data_Check = doio_pat_check;
				break;
			default:
				fprintf(stderr,
					"%s%s:  Unrecognised -C arg '%s' %d",
					Prog, TagName, s->string, s->value);
				exit(1);
			}
			break;

		case 'd':	/* delay between i/o ops */
			parse_delay(optarg);
			break;

		case 'e':
			if (Npes > 1 && Nprocs > 1) {
				fprintf(stderr,
					"%s%s:  Warning - Program is a multi-pe application - exec option is ignored.\n",
					Prog, TagName);
			}
			e_opt++;
			break;

		case 'h':
			help(stdout);
			exit(0);
			break;

		case 'k':
			k_opt++;
			break;

		case 'm':
			Message_Interval = strtol(optarg, &cp, 10);
			if (*cp != '\0' || Message_Interval < 0) {
				fprintf(stderr,
					"%s%s:  Illegal -m arg (%s):  Must be an integer >= 0\n",
					Prog, TagName, optarg);
				exit(1);
			}
			m_opt++;
			break;

		case 'M':	/* memory allocation types */
#ifndef CRAY
			nmemargs = string_to_tokens(optarg, memargs, 32, ",");
			for (ma = 0; ma < nmemargs; ma++) {
				parse_memalloc(memargs[ma]);
			}
			/*dump_memalloc(); */
#else
			fprintf(stderr,
				"%s%s: Error: -M isn't supported on this platform\n",
				Prog, TagName);
			exit(1);
#endif
			M_opt++;
			break;

		case 'N':
			sprintf(TagName, "(%.39s)", optarg);
			break;

		case 'n':
			Nprocs = strtol(optarg, &cp, 10);
			if (*cp != '\0' || Nprocs < 1) {
				fprintf(stderr,
					"%s%s:  Illegal -n arg (%s):  Must be integer > 0\n",
					Prog, TagName, optarg);
				exit(E_USAGE);
			}

			if (Npes > 1 && Nprocs > 1) {
				fprintf(stderr,
					"%s%s:  Program has been built as a multi-pe app.  -n1 is the only nprocs value allowed\n",
					Prog, TagName);
				exit(E_SETUP);
			}
			n_opt++;
			break;

		case 'r':
			Release_Interval = strtol(optarg, &cp, 10);
			if (*cp != '\0' || Release_Interval < 0) {
				fprintf(stderr,
					"%s%s:  Illegal -r arg (%s):  Must be integer >= 0\n",
					Prog, TagName, optarg);
				exit(E_USAGE);
			}

			r_opt++;
			break;

		case 'w':
			Write_Log = optarg;
			w_opt++;
			break;

		case 'v':
			v_opt++;
			break;

		case 'V':
			if (strcasecmp(optarg, "sync") == 0) {
				Validation_Flags = O_SYNC;
			} else if (strcasecmp(optarg, "buffered") == 0) {
				Validation_Flags = 0;
#ifdef CRAY
			} else if (strcasecmp(optarg, "parallel") == 0) {
				Validation_Flags = O_PARALLEL;
			} else if (strcasecmp(optarg, "ldraw") == 0) {
				Validation_Flags = O_LDRAW;
			} else if (strcasecmp(optarg, "raw") == 0) {
				Validation_Flags = O_RAW;
#endif
#ifdef sgi
			} else if (strcasecmp(optarg, "direct") == 0) {
				Validation_Flags = O_DIRECT;
#endif
			} else {
				if (sscanf
				    (optarg, "%i%c", &Validation_Flags,
				     &cc) != 1) {
					fprintf(stderr,
						"%s:  Invalid -V argument (%s) - must be a decimal, hex, or octal\n",
						Prog, optarg);
					fprintf(stderr,
						"    number, or one of the following strings:  'sync',\n");
					fprintf(stderr,
						"    'buffered', 'parallel', 'ldraw', or 'raw'\n");
					exit(E_USAGE);
				}
			}
			V_opt++;
			break;
		case 'U':
			tok = strtok(optarg, ",");
			while (tok != NULL) {
				for (s = Upanic_Args; s->string != NULL; s++)
					if (strcmp(s->string, tok) == 0)
						break;

				if (s->string == NULL) {
					fprintf(stderr,
						"%s%s:  Illegal -U arg (%s).  Must be one of: ",
						Prog, TagName, tok);

					for (s = Upanic_Args; s->string != NULL;
					     s++)
						fprintf(stderr, "%s ",
							s->string);

					fprintf(stderr, "\n");

					exit(1);
				}

				Upanic_Conditions |= s->value;
				tok = strtok(NULL, ",");
			}

			U_opt++;
			break;

		case '?':
			usage(stderr);
			exit(E_USAGE);
			break;
		}
	}

	/*
	 * Supply defaults
	 */

	if (!C_opt) {
		Data_Fill = doio_pat_fill;
		Data_Check = doio_pat_check;
	}

	if (!U_opt)
		Upanic_Conditions = 0;

	if (!n_opt)
		Nprocs = 1;

	if (!r_opt)
		Release_Interval = DEF_RELEASE_INTERVAL;

	if (!M_opt) {
		Memalloc[Nmemalloc].memtype = MEM_DATA;
		Memalloc[Nmemalloc].flags = 0;
		Memalloc[Nmemalloc].name = NULL;
		Memalloc[Nmemalloc].space = NULL;
		Nmemalloc++;
	}

	/*
	 * Initialize input stream
	 */

	if (argc == optind) {
		Infile = NULL;
	} else {
		Infile = argv[optind++];
	}

	if (argc != optind) {
		usage(stderr);
		exit(E_USAGE);
	}

	return 0;
}

/*
 * Parse memory allocation types
 *
 * Types are:
 *  Data
 *  T3E-shmem:blksize[:nblks]
 *  SysV-shmem:shmid:blksize:nblks
 *	if shmid is "private", use IPC_PRIVATE
 *	and nblks is not required
 *
 *  mmap:flags:filename:blksize[:nblks]
 *   flags are one of:
 *	p - private (MAP_PRIVATE)
 *	a - private, MAP_AUTORESRV
 *	l - local (MAP_LOCAL)
 *	s - shared (nblks required)
 *
 *   plus any of:
 *	f - fixed address (MAP_FIXED)
 *	A - use an address without MAP_FIXED
 *	a - autogrow (map once at startup)
 *
 *  mmap:flags:devzero
 *	mmap /dev/zero  (shared not allowd)
 *	maps the first 4096 bytes of /dev/zero
 *
 * - put a directory at the beginning of the shared
 *   regions saying what pid has what region.
 *	DIRMAGIC
 *	BLKSIZE
 *	NBLKS
 *	nblks worth of directories - 1 int pids
 */
#ifndef CRAY
void parse_memalloc(char *arg)
{
	char *allocargs[NMEMALLOC];
	int nalloc;
	struct memalloc *M;

	if (Nmemalloc >= NMEMALLOC) {
		doio_fprintf(stderr, "Error - too many memory types (%d).\n",
			     Nmemalloc);
		return;
	}

	M = &Memalloc[Nmemalloc];

	nalloc = string_to_tokens(arg, allocargs, 32, ":");
	if (!strcmp(allocargs[0], "data")) {
		M->memtype = MEM_DATA;
		M->flags = 0;
		M->name = NULL;
		M->space = NULL;
		Nmemalloc++;
		if (nalloc >= 2) {
			if (strchr(allocargs[1], 'p'))
				M->flags |= MEMF_MPIN;
		}
	} else if (!strcmp(allocargs[0], "mmap")) {
		/* mmap:flags:filename[:size] */
		M->memtype = MEM_MMAP;
		M->flags = 0;
		M->space = NULL;
		if (nalloc >= 1) {
			if (strchr(allocargs[1], 'p'))
				M->flags |= MEMF_PRIVATE;
			if (strchr(allocargs[1], 'a'))
				M->flags |= MEMF_AUTORESRV;
			if (strchr(allocargs[1], 'l'))
				M->flags |= MEMF_LOCAL;
			if (strchr(allocargs[1], 's'))
				M->flags |= MEMF_SHARED;

			if (strchr(allocargs[1], 'f'))
				M->flags |= MEMF_FIXADDR;
			if (strchr(allocargs[1], 'A'))
				M->flags |= MEMF_ADDR;
			if (strchr(allocargs[1], 'G'))
				M->flags |= MEMF_AUTOGROW;

			if (strchr(allocargs[1], 'U'))
				M->flags |= MEMF_FILE;
		} else {
			M->flags |= MEMF_PRIVATE;
		}

		if (nalloc > 2) {
			if (!strcmp(allocargs[2], "devzero")) {
				M->name = "/dev/zero";
				if (M->flags &
				    ((MEMF_PRIVATE | MEMF_LOCAL) == 0))
					M->flags |= MEMF_PRIVATE;
			} else {
				M->name = allocargs[2];
			}
		} else {
			M->name = "/dev/zero";
			if (M->flags & ((MEMF_PRIVATE | MEMF_LOCAL) == 0))
				M->flags |= MEMF_PRIVATE;
		}
		Nmemalloc++;

	} else if (!strcmp(allocargs[0], "shmem")) {
		/* shmem:shmid:size */
		M->memtype = MEM_SHMEM;
		M->flags = 0;
		M->space = NULL;
		if (nalloc >= 2) {
			M->name = allocargs[1];
		} else {
			M->name = NULL;
		}
		if (nalloc >= 3) {
			sscanf(allocargs[2], "%i", &M->nblks);
		} else {
			M->nblks = 0;
		}
		if (nalloc >= 4) {
			if (strchr(allocargs[3], 'p'))
				M->flags |= MEMF_MPIN;
		}

		Nmemalloc++;
	} else {
		doio_fprintf(stderr, "Error - unknown memory type '%s'.\n",
			     allocargs[0]);
		exit(1);
	}
}

void dump_memalloc(void)
{
	int ma;
	char *mt;

	if (Nmemalloc == 0) {
		printf("No memory allocation strategies devined\n");
		return;
	}

	for (ma = 0; ma < Nmemalloc; ma++) {
		switch (Memalloc[ma].memtype) {
		case MEM_DATA:
			mt = "data";
			break;
		case MEM_SHMEM:
			mt = "shmem";
			break;
		case MEM_MMAP:
			mt = "mmap";
			break;
		default:
			mt = "unknown";
			break;
		}
		printf("mstrat[%d] = %d %s\n", ma, Memalloc[ma].memtype, mt);
		printf("\tflags=%#o name='%s' nblks=%d\n",
		       Memalloc[ma].flags,
		       Memalloc[ma].name, Memalloc[ma].nblks);
	}
}

#endif /* !CRAY */

/*
 * -d <op>:<time> - doio inter-operation delay
 *	currently this permits ONE type of delay between operations.
 */

void parse_delay(char *arg)
{
	char *delayargs[NMEMALLOC];
	int ndelay;
	struct smap *s;

	ndelay = string_to_tokens(arg, delayargs, 32, ":");
	if (ndelay < 2) {
		doio_fprintf(stderr,
			     "Illegal delay arg (%s). Must be operation:time\n",
			     arg);
		exit(1);
	}
	for (s = delaymap; s->string != NULL; s++)
		if (!strcmp(s->string, delayargs[0]))
			break;
	if (s->string == NULL) {
		fprintf(stderr,
			"Illegal Delay arg (%s).  Must be one of: ", arg);

		for (s = delaymap; s->string != NULL; s++)
			fprintf(stderr, "%s ", s->string);
		fprintf(stderr, "\n");
		exit(1);
	}

	delayop = s->value;

	sscanf(delayargs[1], "%i", &delaytime);

	if (ndelay > 2) {
		fprintf(stderr, "Warning: extra delay arguments ignored.\n");
	}
}

/*
 * Usage clause - obvious
 */

int usage(FILE * stream)
{
	/*
	 * Only do this if we are on vpe 0, to avoid seeing it from every
	 * process in the application.
	 */

	if (Npes > 1 && Vpe != 0) {
		return 0;
	}

	fprintf(stream,
		"usage%s:  %s [-aekv] [-m message_interval] [-n nprocs] [-r release_interval] [-w write_log] [-V validation_ftype] [-U upanic_cond] [infile]\n",
		TagName, Prog);
	return 0;
}

void help(FILE * stream)
{
	/*
	 * Only the app running on vpe 0 gets to issue help - this prevents
	 * everybody in the application from doing this.
	 */

	if (Npes > 1 && Vpe != 0) {
		return;
	}

	usage(stream);
	fprintf(stream, "\n");
	fprintf(stream,
		"\t-a                   abort - kill all doio processes on data compare\n");
	fprintf(stream,
		"\t                     errors.  Normally only the erroring process exits\n");
	fprintf(stream, "\t-C data-pattern-type \n");
	fprintf(stream,
		"\t                     Available data patterns are:\n");
	fprintf(stream, "\t                     default - repeating pattern\n");
	fprintf(stream, "\t-d Operation:Time    Inter-operation delay.\n");
	fprintf(stream, "\t                     Operations are:\n");
	fprintf(stream,
		"\t                         select:time (1 second=1000000)\n");
	fprintf(stream, "\t                         sleep:time (1 second=1)\n");
#ifdef sgi
	fprintf(stream,
		"\t                         sginap:time (1 second=CLK_TCK=100)\n");
#endif
	fprintf(stream, "\t                         alarm:time (1 second=1)\n");
	fprintf(stream,
		"\t-e                   Re-exec children before entering the main\n");
	fprintf(stream,
		"\t                     loop.  This is useful for spreading\n");
	fprintf(stream,
		"\t                     procs around on multi-pe systems.\n");
	fprintf(stream,
		"\t-k                   Lock file regions during writes using fcntl()\n");
	fprintf(stream,
		"\t-v                   Verify writes - this is done by doing a buffered\n");
	fprintf(stream,
		"\t                     read() of the data if file io was done, or\n");
	fprintf(stream,
		"\t                     an ssread()of the data if sds io was done\n");
#ifndef CRAY
	fprintf(stream,
		"\t-M                   Data buffer allocation method\n");
	fprintf(stream, "\t                     alloc-type[,type]\n");
#ifdef sgi
	fprintf(stream, "\t			    data:flags\n");
	fprintf(stream, "\t			        p - mpin buffer\n");
	fprintf(stream, "\t			    shmem:shmid:size:flags\n");
	fprintf(stream, "\t			        p - mpin buffer\n");
#else
	fprintf(stream, "\t			    data\n");
	fprintf(stream, "\t			    shmem:shmid:size\n");
#endif /* sgi */
	fprintf(stream, "\t			    mmap:flags:filename\n");
	fprintf(stream, "\t			        p - private\n");
#ifdef sgi
	fprintf(stream, "\t			        s - shared\n");
	fprintf(stream, "\t			        l - local\n");
	fprintf(stream, "\t			        a - autoresrv\n");
	fprintf(stream, "\t			        G - autogrow\n");
#else
	fprintf(stream,
		"\t			        s - shared (shared file must exist\n"),
	    fprintf(stream,
		    "\t			            and have needed length)\n");
#endif
	fprintf(stream,
		"\t			        f - fixed address (not used)\n");
	fprintf(stream,
		"\t			        a - specify address (not used)\n");
	fprintf(stream,
		"\t			        U - Unlink file when done\n");
	fprintf(stream,
		"\t			        The default flag is private\n");
	fprintf(stream, "\n");
#endif /* !CRAY */
	fprintf(stream,
		"\t-m message_interval  Generate a message every 'message_interval'\n");
	fprintf(stream,
		"\t                     requests.  An interval of 0 suppresses\n");
	fprintf(stream,
		"\t                     messages.  The default is 0.\n");
	fprintf(stream, "\t-N tagname           Tag name, for Monster.\n");
	fprintf(stream, "\t-n nprocs            # of processes to start up\n");
	fprintf(stream,
		"\t-r release_interval  Release all memory and close\n");
	fprintf(stream,
		"\t                     files every release_interval operations.\n");
	fprintf(stream,
		"\t                     By default procs never release memory\n");
	fprintf(stream,
		"\t                     or close fds unless they have to.\n");
	fprintf(stream,
		"\t-V validation_ftype  The type of file descriptor to use for doing data\n");
	fprintf(stream,
		"\t                     validation.  validation_ftype may be an octal,\n");
	fprintf(stream,
		"\t                     hex, or decimal number representing the open()\n");
	fprintf(stream,
		"\t                     flags, or may be one of the following strings:\n");
	fprintf(stream,
		"\t                     'buffered' - validate using bufferd read\n");
	fprintf(stream,
		"\t                     'sync'     - validate using O_SYNC read\n");
#ifdef sgi
	fprintf(stream,
		"\t                     'direct    - validate using O_DIRECT read'\n");
#endif
#ifdef CRAY
	fprintf(stream,
		"\t                     'ldraw'    - validate using O_LDRAW read\n");
	fprintf(stream,
		"\t                     'parallel' - validate using O_PARALLEL read\n");
	fprintf(stream,
		"\t                     'raw'      - validate using O_RAW read\n");
#endif
	fprintf(stream, "\t                     By default, 'parallel'\n");
	fprintf(stream,
		"\t                     is used if the write was done with O_PARALLEL\n");
	fprintf(stream,
		"\t                     or 'buffered' for all other writes.\n");
	fprintf(stream,
		"\t-w write_log         File to log file writes to.  The doio_check\n");
	fprintf(stream,
		"\t                     program can reconstruct datafiles using the\n");
	fprintf(stream,
		"\t                     write_log, and detect if a file is corrupt\n");
	fprintf(stream,
		"\t                     after all procs have exited.\n");
	fprintf(stream,
		"\t-U upanic_cond       Comma separated list of conditions that will\n");
	fprintf(stream,
		"\t                     cause a call to upanic(PA_PANIC).\n");
	fprintf(stream,
		"\t                     'corruption' -> upanic on bad data comparisons\n");
	fprintf(stream,
		"\t                     'iosw'     ---> upanic on unexpected async iosw\n");
	fprintf(stream,
		"\t                     'rval'     ---> upanic on unexpected syscall rvals\n");
	fprintf(stream,
		"\t                     'all'      ---> all of the above\n");
	fprintf(stream, "\n");
	fprintf(stream,
		"\tinfile               Input stream - default is stdin - must be a list\n");
	fprintf(stream,
		"\t                     of io_req structures (see doio.h).  Currently\n");
	fprintf(stream,
		"\t                     only the iogen program generates the proper\n");
	fprintf(stream, "\t                     format\n");
}
