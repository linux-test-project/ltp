/*
 * New Interface to Process Table -- PROCTAB Stream (a la Directory streams)
 * Copyright (C) 1996 Charles L. Blake.
 * Copyright (C) 1998 Michael K. Johnson
 * Copyright 1998-2002 Albert Cahalan
 * May be distributed under the conditions of the
 * GNU Library General Public License; a copy is in COPYING
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "version.h"
#include "readproc.h"
#include "alloc.h"
#include "pwcache.h"
#include "devname.h"
#include "procps.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef FLASK_LINUX
#include <fs_secure.h>
#endif

/* initiate a process table scan
 */
PROCTAB* openproc(int flags, ...) {
    va_list ap;
    PROCTAB* PT = xmalloc(sizeof(PROCTAB));
    
    if (flags & PROC_PID)
      PT->procfs = NULL;
    else if (!(PT->procfs = opendir("/proc")))
      return NULL;
    PT->flags = flags;
    va_start(ap, flags);		/*  Init args list */
    if (flags & PROC_PID)
    	PT->pids = va_arg(ap, pid_t*);
    else if (flags & PROC_UID) {
    	PT->uids = va_arg(ap, uid_t*);
	PT->nuid = va_arg(ap, int);
    }
    va_end(ap);				/*  Clean up args list */
    return PT;
}

/* terminate a process table scan
 */
void closeproc(PROCTAB* PT) {
    if (PT){
        if (PT->procfs) closedir(PT->procfs);
        free(PT);
    }
}

/* deallocate the space allocated by readproc if the passed rbuf was NULL
 */
void freeproc(proc_t* p) {
    if (!p)	/* in case p is NULL */
	return;
    /* ptrs are after strings to avoid copying memory when building them. */
    /* so free is called on the address of the address of strvec[0]. */
    if (p->cmdline)
	free((void*)*p->cmdline);
    if (p->environ)
	free((void*)*p->environ);
    free(p);
}


// 2.5.xx looks like:
//
// "State:\t%s\n"
// "Tgid:\t%d\n"
// "Pid:\t%d\n"
// "PPid:\t%d\n"
// "TracerPid:\t%d\n"

static void status2proc(const char *S, proc_t *restrict P){
    char* tmp;
    unsigned i;

    // The cmd is escaped, with \\ and \n for backslash and newline.
    // It certainly may contain "VmSize:" and similar crap.
    if(unlikely(strncmp("Name:\t",S,6))) fprintf(stderr, "Internal error!\n");
    S += 6;
    i = 0;
    while(i < sizeof P->cmd - 1){
      int c = *S++;
      if(unlikely(c=='\n')) break;
      if(unlikely(c=='\0')) return; // should never happen
      if(unlikely(c=='\\')){
        c = *S++;
        if(c=='\n') break; // should never happen
        if(!c) break; // should never happen
        if(c=='n') c='\n'; // else we assume it is '\\'
      }
      P->cmd[i++] = c;
    }
    P->cmd[i] = '\0';

    tmp = strstr (S,"State:\t");
    if(likely(tmp)) P->state = tmp[7];
    else fprintf(stderr, "Internal error!\n");

    tmp = strstr (S,"PPid:");
    if(likely(tmp)) sscanf (tmp,
        "PPid:\t%d\n",
        &P->ppid
    );
    else fprintf(stderr, "Internal error!\n");

    tmp = strstr (S,"Uid:");
    if(likely(tmp)) sscanf (tmp,
        "Uid:\t%d\t%d\t%d\t%d",
        &P->ruid, &P->euid, &P->suid, &P->fuid
    );
    else fprintf(stderr, "Internal error!\n");

    tmp = strstr (S,"Gid:");
    if(likely(tmp)) sscanf (tmp,
        "Gid:\t%d\t%d\t%d\t%d",
        &P->rgid, &P->egid, &P->sgid, &P->fgid
    );
    else fprintf(stderr, "Internal error!\n");

    tmp = strstr (S,"VmSize:");
    if(likely(tmp)) sscanf (tmp,
        "VmSize: %lu kB\n"
        "VmLck: %lu kB\n"
        "VmRSS: %lu kB\n"
        "VmData: %lu kB\n"
        "VmStk: %lu kB\n"
        "VmExe: %lu kB\n"
        "VmLib: %lu kB\n",
        &P->vm_size, &P->vm_lock, &P->vm_rss, &P->vm_data,
        &P->vm_stack, &P->vm_exe, &P->vm_lib
    );
    else /* looks like an annoying kernel thread */
    {
        P->vm_size  = 0;
        P->vm_lock  = 0;
        P->vm_rss   = 0;
        P->vm_data  = 0;
        P->vm_stack = 0;
        P->vm_exe   = 0;
        P->vm_lib   = 0;
    }

    tmp = strstr (S,"SigPnd:");
    if(likely(tmp)) sscanf (tmp,
#ifdef SIGNAL_STRING
        "SigPnd: %s SigBlk: %s SigIgn: %s %*s %s",
        P->signal, P->blocked, P->sigignore, P->sigcatch
#else
        "SigPnd: %Lx SigBlk: %Lx SigIgn: %Lx %*s %Lx",
        &P->signal, &P->blocked, &P->sigignore, &P->sigcatch
#endif
    );
    else fprintf(stderr, "Internal error!\n");
}



// Reads /proc/*/stat files, being careful not to trip over processes with
// names like ":-) 1 2 3 4 5 6".
static void stat2proc(const char* S, proc_t *restrict P) {
    unsigned num;
    char* tmp;

    /* fill in default values for older kernels */
    P->exit_signal = SIGCHLD;
    P->processor = 0;
    P->rtprio = -1;
    P->sched = -1;

    S = strchr(S, '(') + 1;
    tmp = strrchr(S, ')');
    num = tmp - S;
    if(unlikely(num >= sizeof P->cmd)) num = sizeof P->cmd - 1;
    memcpy(P->cmd, S, num);
    P->cmd[num] = '\0';
    S = tmp + 2;                 // skip ") "

    num = sscanf(S,
       "%c "
       "%d %d %d %d %d "
       "%lu %lu %lu %lu %lu "
       "%Lu %Lu %Lu %Lu "  /* utime stime cutime cstime */
       "%ld %ld %ld %ld "
       "%Lu "  /* start_time */
       "%lu "
       "%ld "
       "%lu %lu %lu %lu %lu %lu "
       "%*s %*s %*s %*s " /* discard, no RT signals & Linux 2.1 used hex */
       "%lu %lu %lu "
       "%d %d "
       "%lu %lu",
       &P->state,
       &P->ppid, &P->pgrp, &P->session, &P->tty, &P->tpgid,
       &P->flags, &P->min_flt, &P->cmin_flt, &P->maj_flt, &P->cmaj_flt,
       &P->utime, &P->stime, &P->cutime, &P->cstime,
       &P->priority, &P->nice, &P->timeout, &P->it_real_value,
       &P->start_time,
       &P->vsize,
       &P->rss,
       &P->rss_rlim, &P->start_code, &P->end_code, &P->start_stack, &P->kstk_esp, &P->kstk_eip,
/*     P->signal, P->blocked, P->sigignore, P->sigcatch,   */ /* can't use */
       &P->wchan, &P->nswap, &P->cnswap,
/* -- Linux 2.0.35 ends here -- */
       &P->exit_signal, &P->processor,  /* 2.2.1 ends with "exit_signal" */
/* -- Linux 2.2.8 to 2.5.17 end here -- */
       &P->rtprio, &P->sched  /* both added to 2.5.18 */
    );
}

static void statm2proc(const char* s, proc_t *restrict P) {
    int num;
    num = sscanf(s, "%ld %ld %ld %ld %ld %ld %ld",
	   &P->size, &P->resident, &P->share,
	   &P->trs, &P->lrs, &P->drs, &P->dt);
/*    fprintf(stderr, "statm2proc converted %d fields.\n",num); */
}

static int file2str(const char *directory, const char *what, char *ret, int cap) {
    static char filename[80];
    int fd, num_read;

    sprintf(filename, "%s/%s", directory, what);
    fd = open(filename, O_RDONLY, 0);
    if(unlikely(fd==-1)) return -1;
    num_read = read(fd, ret, cap - 1);
    if(unlikely(num_read<=0)) num_read = -1;
    else ret[num_read] = 0;
    close(fd);
    return num_read;
}

static char** file2strvec(const char* directory, const char* what) {
    char buf[2048];	/* read buf bytes at a time */
    char *p, *rbuf = 0, *endbuf, **q, **ret;
    int fd, tot = 0, n, c, end_of_file = 0;
    int align;

    sprintf(buf, "%s/%s", directory, what);
    fd = open(buf, O_RDONLY, 0);
    if(fd==-1) return NULL;

    /* read whole file into a memory buffer, allocating as we go */
    while ((n = read(fd, buf, sizeof buf - 1)) > 0) {
	if (n < (int)(sizeof buf - 1))
	    end_of_file = 1;
	if (n == 0 && rbuf == 0)
	    return NULL;	/* process died between our open and read */
	if (n < 0) {
	    if (rbuf)
		free(rbuf);
	    return NULL;	/* read error */
	}
	if (end_of_file && buf[n-1])		/* last read char not null */
	    buf[n++] = '\0';			/* so append null-terminator */
	rbuf = xrealloc(rbuf, tot + n);		/* allocate more memory */
	memcpy(rbuf + tot, buf, n);		/* copy buffer into it */
	tot += n;				/* increment total byte ctr */
	if (end_of_file)
	    break;
    }
    close(fd);
    if (n <= 0 && !end_of_file) {
	if (rbuf) free(rbuf);
	return NULL;		/* read error */
    }
    endbuf = rbuf + tot;			/* count space for pointers */
    align = (sizeof(char*)-1) - ((tot + sizeof(char*)-1) & (sizeof(char*)-1));
    for (c = 0, p = rbuf; p < endbuf; p++)
    	if (!*p)
	    c += sizeof(char*);
    c += sizeof(char*);				/* one extra for NULL term */

    rbuf = xrealloc(rbuf, tot + c + align);	/* make room for ptrs AT END */
    endbuf = rbuf + tot;			/* addr just past data buf */
    q = ret = (char**) (endbuf+align);		/* ==> free(*ret) to dealloc */
    *q++ = p = rbuf;				/* point ptrs to the strings */
    endbuf--;					/* do not traverse final NUL */
    while (++p < endbuf) 
    	if (!*p)				/* NUL char implies that */
	    *q++ = p+1;				/* next string -> next char */

    *q = 0;					/* null ptr list terminator */
    return ret;
}

// warning: interface may change
int read_cmdline(char *restrict const dst, unsigned sz, unsigned pid){
    char name[32];
    int fd;
    unsigned n = 0;
    dst[0] = '\0';
    snprintf(name, sizeof name, "/proc/%u/cmdline", pid);
    fd = open(name, O_RDONLY);
    if(fd==-1) return 0;
    for(;;){
        ssize_t r = read(fd,dst+n,sz-n);
        if(r==-1){
            if(errno==EINTR) continue;
            break;
        }
        n += r;
        if(n==sz) break; // filled the buffer
        if(r==0) break;  // EOF
    }
    if(n){
        int i;
        if(n==sz) n--;
        dst[n] = '\0';
        i=n;
        while(i--){
          int c = dst[i];
          if(c<' ' || c>'~') dst[i]=' ';
        }
    }
    return n;
}

/* These are some nice GNU C expression subscope "inline" functions.
 * The can be used with arbitrary types and evaluate their arguments
 * exactly once.
 */

/* Test if item X of type T is present in the 0 terminated list L */
#   define XinL(T, X, L) ( {			\
	    T  x = (X), *l = (L);		\
	    while (*l && *l != x) l++;		\
	    *l == x;				\
	} )

/* Test if item X of type T is present in the list L of length N */
#   define XinLN(T, X, L, N) ( {		\
	    T x = (X), *l = (L);		\
	    int i = 0, n = (N);			\
	    while (i < n && l[i] != x) i++;	\
	    i < n && l[i] == x;			\
	} )

/* readproc: return a pointer to a proc_t filled with requested info about the
 * next process available matching the restriction set.  If no more such
 * processes are available, return a null pointer (boolean false).  Use the
 * passed buffer instead of allocating space if it is non-NULL.  */

/* This is optimized so that if a PID list is given, only those files are
 * searched for in /proc.  If other lists are given in addition to the PID list,
 * the same logic can follow through as for the no-PID list case.  This is
 * fairly complex, but it does try to not to do any unnecessary work.
 */
proc_t* readproc(PROCTAB* PT, proc_t* p) {
    static struct direct *ent;		/* dirent handle */
    static struct stat sb;		/* stat buffer */
    static char path[32], sbuf[1024];	/* bufs for stat,statm */
#ifdef FLASK_LINUX
    security_id_t secsid;
#endif
    pid_t pid;  // saved until we have a proc_t allocated for sure

    /* loop until a proc matching restrictions is found or no more processes */
    /* I know this could be a while loop -- this way is easier to indent ;-) */
next_proc:				/* get next PID for consideration */

/*printf("PT->flags is 0x%08x\n", PT->flags);*/
#define flags (PT->flags)

    if (flags & PROC_PID) {
        pid = *(PT->pids)++;
	if (unlikely(!pid)) return NULL;
	snprintf(path, sizeof path, "/proc/%d", pid);
    } else {					/* get next numeric /proc ent */
	for (;;) {
	    ent = readdir(PT->procfs);
	    if(unlikely(unlikely(!ent) || unlikely(!ent->d_name))) return NULL;
	    if(likely( likely(*ent->d_name > '0') && likely(*ent->d_name <= '9') )) break;
	}
	pid = strtoul(ent->d_name, NULL, 10);
	memcpy(path, "/proc/", 6);
	strcpy(path+6, ent->d_name);  // trust /proc to not contain evil top-level entries
//	snprintf(path, sizeof path, "/proc/%s", ent->d_name);
    }
#ifdef FLASK_LINUX
    if ( stat_secure(path, &sb, &secsid) == -1 ) /* no such dirent (anymore) */
#else
    if (unlikely(stat(path, &sb) == -1))	/* no such dirent (anymore) */
#endif
	goto next_proc;

    if ((flags & PROC_UID) && !XinLN(uid_t, sb.st_uid, PT->uids, PT->nuid))
	goto next_proc;			/* not one of the requested uids */

    if (!p)
	p = xcalloc(p, sizeof *p); /* passed buf or alloced mem */

    p->euid = sb.st_uid;			/* need a way to get real uid */
#ifdef FLASK_LINUX
    p->secsid = secsid;
#endif
    p->pid  = pid;

    if (flags & PROC_FILLSTAT) {         /* read, parse /proc/#/stat */
	if (unlikely( file2str(path, "stat", sbuf, sizeof sbuf) == -1 ))
	    goto next_proc;			/* error reading /proc/#/stat */
	stat2proc(sbuf, p);				/* parse /proc/#/stat */
    }

    if (unlikely(flags & PROC_FILLMEM)) {				/* read, parse /proc/#/statm */
	if (likely( file2str(path, "statm", sbuf, sizeof sbuf) != -1 ))
	    statm2proc(sbuf, p);		/* ignore statm errors here */
    }						/* statm fields just zero */

    if (flags & PROC_FILLSTATUS) {         /* read, parse /proc/#/status */
       if (likely( file2str(path, "status", sbuf, sizeof sbuf) != -1 )){
           status2proc(sbuf, p);
       }
    }

    /* some number->text resolving which is time consuming */
    if (flags & PROC_FILLUSR){
	strncpy(p->euser,   user_from_uid(p->euid), sizeof p->euser);
        if(flags & PROC_FILLSTATUS) {
            strncpy(p->ruser,   user_from_uid(p->ruid), sizeof p->ruser);
            strncpy(p->suser,   user_from_uid(p->suid), sizeof p->suser);
            strncpy(p->fuser,   user_from_uid(p->fuid), sizeof p->fuser);
        }
    }

    /* some number->text resolving which is time consuming */
    if (flags & PROC_FILLGRP){
        strncpy(p->egroup, group_from_gid(p->egid), sizeof p->egroup);
        if(flags & PROC_FILLSTATUS) {
            strncpy(p->rgroup, group_from_gid(p->rgid), sizeof p->rgroup);
            strncpy(p->sgroup, group_from_gid(p->sgid), sizeof p->sgroup);
            strncpy(p->fgroup, group_from_gid(p->fgid), sizeof p->fgroup);
        }
    }

    if ((flags & PROC_FILLCOM) || (flags & PROC_FILLARG))	/* read+parse /proc/#/cmdline */
	p->cmdline = file2strvec(path, "cmdline");
    else
        p->cmdline = NULL;

    if (unlikely(flags & PROC_FILLENV))			/* read+parse /proc/#/environ */
	p->environ = file2strvec(path, "environ");
    else
        p->environ = NULL;
    
    return p;
}
#undef flags

/* ps_readproc: return a pointer to a proc_t filled with requested info about the
 * next process available matching the restriction set.  If no more such
 * processes are available, return a null pointer (boolean false).  Use the
 * passed buffer instead of allocating space if it is non-NULL.  */

/* This is optimized so that if a PID list is given, only those files are
 * searched for in /proc.  If other lists are given in addition to the PID list,
 * the same logic can follow through as for the no-PID list case.  This is
 * fairly complex, but it does try to not to do any unnecessary work.
 */
proc_t* ps_readproc(PROCTAB* PT, proc_t* p) {
    static struct direct *ent;		/* dirent handle */
    static struct stat sb;		/* stat buffer */
    static char path[32], sbuf[1024];	/* bufs for stat,statm */
#ifdef FLASK_LINUX
    security_id_t secsid;
#endif
    pid_t pid;  // saved until we have a proc_t allocated for sure

    /* loop until a proc matching restrictions is found or no more processes */
    /* I know this could be a while loop -- this way is easier to indent ;-) */
next_proc:				/* get next PID for consideration */

/*printf("PT->flags is 0x%08x\n", PT->flags);*/
#define flags (PT->flags)

    for (;;) {
	ent = readdir(PT->procfs);
	if(unlikely(unlikely(!ent) || unlikely(!ent->d_name))) return NULL;
	if(likely( likely(*ent->d_name > '0') && likely(*ent->d_name <= '9') )) break;
    }
    pid = strtoul(ent->d_name, NULL, 10);
    memcpy(path, "/proc/", 6);
    strcpy(path+6, ent->d_name);  // trust /proc to not contain evil top-level entries
//  snprintf(path, sizeof path, "/proc/%s", ent->d_name);

#ifdef FLASK_LINUX
    if (stat_secure(path, &sb, &secsid) == -1) /* no such dirent (anymore) */
#else
    if (stat(path, &sb) == -1)		/* no such dirent (anymore) */
#endif
	goto next_proc;

    if (!p)
	p = xcalloc(p, sizeof *p); /* passed buf or alloced mem */

    p->euid = sb.st_uid;			/* need a way to get real uid */
#ifdef FLASK_LINUX
    p->secsid = secsid;
#endif
    p->pid  = pid;

    if ((file2str(path, "stat", sbuf, sizeof sbuf)) == -1)
	goto next_proc;			/* error reading /proc/#/stat */
    stat2proc(sbuf, p);				/* parse /proc/#/stat */

    if (flags & PROC_FILLMEM) {				/* read, parse /proc/#/statm */
	if ((file2str(path, "statm", sbuf, sizeof sbuf)) != -1 )
	    statm2proc(sbuf, p);		/* ignore statm errors here */
    }						/* statm fields just zero */

  /*  if (flags & PROC_FILLSTATUS) { */        /* read, parse /proc/#/status */
       if ((file2str(path, "status", sbuf, sizeof sbuf)) != -1 ){
           status2proc(sbuf, p);
       }
/*    }*/

    /* some number->text resolving which is time consuming */
    if (flags & PROC_FILLUSR){
	strncpy(p->euser,   user_from_uid(p->euid), sizeof p->euser);
/*        if(flags & PROC_FILLSTATUS) { */
            strncpy(p->ruser,   user_from_uid(p->ruid), sizeof p->ruser);
            strncpy(p->suser,   user_from_uid(p->suid), sizeof p->suser);
            strncpy(p->fuser,   user_from_uid(p->fuid), sizeof p->fuser);
/*        }*/
    }

    /* some number->text resolving which is time consuming */
    if (flags & PROC_FILLGRP){
        strncpy(p->egroup, group_from_gid(p->egid), sizeof p->egroup);
/*        if(flags & PROC_FILLSTATUS) { */
            strncpy(p->rgroup, group_from_gid(p->rgid), sizeof p->rgroup);
            strncpy(p->sgroup, group_from_gid(p->sgid), sizeof p->sgroup);
            strncpy(p->fgroup, group_from_gid(p->fgid), sizeof p->fgroup);
/*        }*/
    }

    if ((flags & PROC_FILLCOM) || (flags & PROC_FILLARG))	/* read+parse /proc/#/cmdline */
	p->cmdline = file2strvec(path, "cmdline");
    else
        p->cmdline = NULL;

    if (flags & PROC_FILLENV)			/* read+parse /proc/#/environ */
	p->environ = file2strvec(path, "environ");
    else
        p->environ = NULL;
    
    return p;
}
#undef flags


void look_up_our_self(proc_t *p) {
    static char path[32], sbuf[1024];	/* bufs for stat,statm */
    sprintf(path, "/proc/%d", getpid());
    file2str(path, "stat", sbuf, sizeof sbuf);
    stat2proc(sbuf, p);				/* parse /proc/#/stat */
    file2str(path, "statm", sbuf, sizeof sbuf);
    statm2proc(sbuf, p);		/* ignore statm errors here */
    file2str(path, "status", sbuf, sizeof sbuf);
    status2proc(sbuf, p);
}


/* Convenient wrapper around openproc and readproc to slurp in the whole process
 * table subset satisfying the constraints of flags and the optional PID list.
 * Free allocated memory with freeproctab().  Access via tab[N]->member.  The
 * pointer list is NULL terminated.
 */
proc_t** readproctab(int flags, ...) {
    PROCTAB* PT = NULL;
    proc_t** tab = NULL;
    int n = 0;
    va_list ap;

    va_start(ap, flags);		/* pass through args to openproc */
    if (flags & PROC_UID) {
	/* temporary variables to ensure that va_arg() instances
	 * are called in the right order
	 */
	uid_t* u;
	int i;

	u = va_arg(ap, uid_t*);
	i = va_arg(ap, int);
	PT = openproc(flags, u, i);
    }
    else if (flags & PROC_PID)
	PT = openproc(flags, va_arg(ap, void*)); /* assume ptr sizes same */
    else
	PT = openproc(flags);
    va_end(ap);
    do {					/* read table: */
	tab = xrealloc(tab, (n+1)*sizeof(proc_t*));/* realloc as we go, using */
	tab[n] = readproc(PT, NULL);		  /* final null to terminate */
    } while (tab[n++]);				  /* stop when NULL reached */
    closeproc(PT);
    return tab;
}
