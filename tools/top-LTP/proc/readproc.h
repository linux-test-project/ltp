#ifndef PROCPS_PROC_READPROC_H
#define PROCPS_PROC_READPROC_H
/*
 * New Interface to Process Table -- PROCTAB Stream (a la Directory streams)
 * Copyright 1996 Charles L. Blake.
 * Copyright 1998 Michael K. Johnson
 * Copyright 1998-2002 Albert Cahalan
 * May be distributed under the terms of the
 * GNU Library General Public License, a copy of which is provided
 * in the file COPYING
 */

#include "procps.h"

#define SIGNAL_STRING

#ifdef FLASK_LINUX
#include <fs_secure.h>
#endif

EXTERN_C_BEGIN

/*
 ld	cutime, cstime, priority, nice, timeout, it_real_value, rss,
 c	state,
 d	ppid, pgrp, session, tty, tpgid,
 s	signal, blocked, sigignore, sigcatch,
 lu	flags, min_flt, cmin_flt, maj_flt, cmaj_flt, utime, stime,
 lu	rss_rlim, start_code, end_code, start_stack, kstk_esp, kstk_eip,
 lu	start_time, vsize, wchan, nswap, cnswap,
*/

/* Basic data structure which holds all information we can get about a process.
 * (unless otherwise specified, fields are read from /proc/#/stat)
 *
 * Most of it comes from task_struct in linux/sched.h
 */
typedef struct proc_t {
// 1st 16 bytes
    int
        pid,		/* process id */
    	ppid;		/* pid of parent process */
    unsigned
        pcpu;           /* %CPU usage (is not filled in by readproc!!!) */
    char
    	state,		/* single-char code for process state (S=sleeping) */
    	pad_1,		/* padding */
    	pad_2,		/* padding */
    	pad_3;		/* padding */
// 2nd 16 bytes
    unsigned long long
	utime,		/* user-mode CPU time accumulated by process */
	stime,		/* kernel-mode CPU time accumulated by process */
// and so on...
	cutime,		/* cumulative utime of process and reaped children */
	cstime,		/* cumulative stime of process and reaped children */
	start_time;	/* start time of process -- seconds since 1-1-70 */
#ifdef SIGNAL_STRING
    char
	/* Linux 2.1.7x and up have 64 signals. Allow 64, plus '\0' and padding. */
	signal[18],	/* mask of pending signals */
	blocked[18],	/* mask of blocked signals */
	sigignore[18],	/* mask of ignored signals */
	sigcatch[18];	/* mask of caught  signals */
#else
    long long
	/* Linux 2.1.7x and up have 64 signals. */
	signal,		/* mask of pending signals */
	blocked,	/* mask of blocked signals */
	sigignore,	/* mask of ignored signals */
	sigcatch;	/* mask of caught  signals */
#endif
    long
	priority,	/* kernel scheduling priority */
	timeout,	/* ? */
	nice,		/* standard unix nice level of process */
	rss,		/* resident set size from /proc/#/stat (pages) */
	it_real_value,	/* ? */
    /* the next 7 members come from /proc/#/statm */
	size,		/* total # of pages of memory */
	resident,	/* number of resident set (non-swapped) pages (4k) */
	share,		/* number of pages of shared (mmap'd) memory */
	trs,		/* text resident set size */
	lrs,		/* shared-lib resident set size */
	drs,		/* data resident set size */
	dt;		/* dirty pages */
    unsigned long
	vm_size,        /* same as vsize in kb */
	vm_lock,        /* locked pages in kb */
	vm_rss,         /* same as rss in kb */
	vm_data,        /* data size */
	vm_stack,       /* stack size */
	vm_exe,         /* executable size */
	vm_lib,         /* library size (all pages, not just used ones) */
	rtprio,		/* real-time priority */
	sched,		/* scheduling class */
	vsize,		/* number of pages of virtual memory ... */
	rss_rlim,	/* resident set size limit? */
	flags,		/* kernel flags for the process */
	min_flt,	/* number of minor page faults since process start */
	maj_flt,	/* number of major page faults since process start */
	cmin_flt,	/* cumulative min_flt of process and child processes */
	cmaj_flt,	/* cumulative maj_flt of process and child processes */
	nswap,		/* ? */
	cnswap,		/* cumulative nswap ? */
	start_code,	/* address of beginning of code segment */
	end_code,	/* address of end of code segment */
	start_stack,	/* address of the bottom of stack for the process */
	kstk_esp,	/* kernel stack pointer */
	kstk_eip,	/* kernel instruction pointer */
	wchan;		/* address of kernel wait channel proc is sleeping in */
    char
	**environ,	/* environment string vector (/proc/#/environ) */
	**cmdline;	/* command line string vector (/proc/#/cmdline) */
    char
	/* Be compatible: Digital allows 16 and NT allows 14 ??? */
    	ruser[16],	/* real user name */
    	euser[16],	/* effective user name */
    	suser[16],	/* saved user name */
    	fuser[16],	/* filesystem user name */
    	rgroup[16],	/* real group name */
    	egroup[16],	/* effective group name */
    	sgroup[16],	/* saved group name */
    	fgroup[16],	/* filesystem group name */
    	cmd[16];	/* basename of executable file in call to exec(2) */
    int
        ruid, rgid,     /* real      */
        euid, egid,     /* effective */
        suid, sgid,     /* saved     */
        fuid, fgid,     /* fs (used for file access only) */
	pgrp,		/* process group id */
	session,	/* session id */
	tty,		/* full device number of controlling terminal */
	tpgid,		/* terminal process group id */
	exit_signal,	/* might not be SIGCHLD */
	processor;      /* current (or most recent?) CPU */
#ifdef FLASK_LINUX
	security_id_t secsid;
#endif
} proc_t;

/* PROCTAB: data structure holding the persistent information readproc needs
 * from openproc().  The setup is intentionally similar to the dirent interface
 * and other system table interfaces (utmp+wtmp come to mind).
 */
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
typedef struct PROCTAB {
    DIR*	procfs;
    int		flags;
    pid_t*	pids;	/* pids of the procs */
    uid_t*	uids;	/* uids of procs */
    int		nuid;	/* cannot really sentinel-terminate unsigned short[] */
#ifdef FLASK_LINUX
    security_id_t* sids; /* SIDs of the procs */
#endif
} PROCTAB;

/* initialize a PROCTAB structure holding needed call-to-call persistent data
 */
extern PROCTAB* openproc(int flags, ... /* pid_t*|uid_t*|dev_t*|char* [, int n] */ );


/* Convenient wrapper around openproc and readproc to slurp in the whole process
 * table subset satisfying the constraints of flags and the optional PID list.
 * Free allocated memory with freeproctab().  Access via tab[N]->member.  The
 * pointer list is NULL terminated.
 */
extern proc_t** readproctab(int flags, ... /* same as openproc */ );

/* clean-up open files, etc from the openproc()
 */
extern void closeproc(PROCTAB* PT);

/* retrieve the next process matching the criteria set by the openproc()
 */
extern proc_t* readproc(PROCTAB* PT, proc_t* return_buf);
extern proc_t* ps_readproc(PROCTAB* PT, proc_t* return_buf);

// warning: interface may change
extern int read_cmdline(char *restrict const dst, unsigned sz, unsigned pid);

extern void look_up_our_self(proc_t *p);

/* deallocate space allocated by readproc
 */
extern void freeproc(proc_t* p);

/* openproc/readproctab:
 *   
 * Return PROCTAB* / *proc_t[] or NULL on error ((probably) "/proc" cannot be
 * opened.)  By default readproc will consider all processes as valid to parse
 * and return, but not actually fill in the cmdline, environ, and /proc/#/statm
 * derived memory fields.
 *
 * `flags' (a bitwise-or of PROC_* below) modifies the default behavior.  The
 * "fill" options will cause more of the proc_t to be filled in.  The "filter"
 * options all use the second argument as the pointer to a list of objects:
 * process status', process id's, user id's.  The third
 * argument is the length of the list (currently only used for lists of user
 * id's since uid_t supports no convenient termination sentinel.)
 */
#define PROC_FILLMEM    0x0001 /* read statm */
#define PROC_FILLCOM    0x0002 /* alloc and fill in `cmdline' */
#define PROC_FILLENV    0x0004 /* alloc and fill in `environ' */
#define PROC_FILLUSR    0x0008 /* resolve user id number -> user name */
#define PROC_FILLGRP    0x0010 /* resolve group id number -> group name */
#define PROC_FILLSTATUS 0x0020 /* read status -- currently unconditional */
#define PROC_FILLSTAT   0x0040 /* read stat -- currently unconditional */
#define PROC_FILLWCHAN  0x0080 /* look up WCHAN name */
#define PROC_FILLARG    0x0100 /* alloc and fill in `cmdline' */

#define PROC_FILLBUG    0x0fff /* No idea what we need */
#define PROC_FILLANY    0x0000 /* either stat or status will do */

/* Obsolete, consider only processes with one of the passed: */
#define PROC_PID     0x1000  /* process id numbers ( 0   terminated) */
#define PROC_UID     0x4000  /* user id numbers    ( length needed ) */

// it helps to give app code a few spare bits
#define PROC_SPARE_1 0x01000000
#define PROC_SPARE_2 0x02000000
#define PROC_SPARE_3 0x04000000
#define PROC_SPARE_4 0x08000000

EXTERN_C_END
#endif
