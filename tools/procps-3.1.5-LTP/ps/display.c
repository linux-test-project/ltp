/*
 * Copyright 1998-2002 by Albert Cahalan; all rights resered.
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 */                                                                     

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* username lookups */
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

/* major/minor number */
#include <sys/sysmacros.h>

#include <signal.h>   /* catch signals */

#include "common.h"
#include "../proc/wchan.h"
#include "../proc/version.h"
#include "../proc/readproc.h"
#include "../proc/sysinfo.h"

#ifndef SIGCHLD
#define SIGCHLD SIGCLD
#endif

/* just reports a crash */
static void signal_handler(int signo){
  if(signo==SIGPIPE) _exit(0);  /* "ps | head" will cause this */
  /* fprintf() is not reentrant, but we _exit() anyway */
  fprintf(stderr,
    "\n\n"
    "Signal %d caught by ps (%s).\n"
    "Please send bug reports to <acahalan@cs.uml.edu>\n",
    signo,
    procps_version
  );
  _exit(signo+128);
}

/////////////////////////////////////////////////////////////////////////////////////
#undef DEBUG
#ifdef DEBUG
void init_stack_trace(char *prog_name);

#include <ctype.h>

void hex_dump(void *vp){
  char *charlist;
  int i = 0;
  int line = 45;
  char *cp = (char *)vp;

  while(line--){
      printf("%8lx  ", (unsigned long)cp);
      charlist = cp;
      cp += 16;
      for(i=0; i<16; i++){
        if((charlist[i]>31) && (charlist[i]<127)){
          printf("%c", charlist[i]);
        }else{
          printf(".");
        }
      }
      printf(" ");
      for(i=0; i<16; i++) printf(" %2x",(unsigned int)((unsigned char)(charlist[i])));
      printf("\n");
      i=0;
  }
}

static void show_pid(char *s, int n, sel_union *data){
  printf("%s  ", s);
  while(--n){
    printf("%d,", data[n].pid);
  }
  printf("%d\n", data[0].pid);
}

static void show_uid(char *s, int n, sel_union *data){
  struct passwd *pw_data;
  printf("%s  ", s);
  while(--n){
    pw_data = getpwuid(data[n].uid);
    if(pw_data) printf("%s,", pw_data->pw_name);
    else        printf("%d,", data[n].uid);
  }
  pw_data = getpwuid(data[n].uid);
  if(pw_data) printf("%s\n", pw_data->pw_name);
  else        printf("%d\n", data[n].uid);
}

static void show_gid(char *s, int n, sel_union *data){
  struct group *gr_data;
  printf("%s  ", s);
  while(--n){
    gr_data = getgrgid(data[n].gid);
    if(gr_data) printf("%s,", gr_data->gr_name);
    else        printf("%d,", data[n].gid);
  }
  gr_data = getgrgid(data[n].gid);
  if(gr_data) printf("%s\n", gr_data->gr_name);
  else        printf("%d\n", data[n].gid);
}

static void show_tty(char *s, int n, sel_union *data){
  printf("%s  ", s);
  while(--n){
    printf("%d:%d,", (int)major(data[n].tty), (int)minor(data[n].tty));
  }
  printf("%d:%d\n", (int)major(data[n].tty), (int)minor(data[n].tty));
}

static void show_cmd(char *s, int n, sel_union *data){
  printf("%s  ", s);
  while(--n){
    printf("%.8s,", data[n].cmd);
  }
  printf("%.8s\n", data[0].cmd);
}

static void arg_show(void){
  selection_node *walk = selection_list;
  while(walk){
    switch(walk->typecode){
    case SEL_RUID: show_uid("RUID", walk->n, walk->u); break;
    case SEL_EUID: show_uid("EUID", walk->n, walk->u); break;
    case SEL_SUID: show_uid("SUID", walk->n, walk->u); break;
    case SEL_FUID: show_uid("FUID", walk->n, walk->u); break;
    case SEL_RGID: show_gid("RGID", walk->n, walk->u); break;
    case SEL_EGID: show_gid("EGID", walk->n, walk->u); break;
    case SEL_SGID: show_gid("SGID", walk->n, walk->u); break;
    case SEL_FGID: show_gid("FGID", walk->n, walk->u); break;
    case SEL_PGRP: show_pid("PGRP", walk->n, walk->u); break;
    case SEL_PID : show_pid("PID ", walk->n, walk->u); break;
    case SEL_TTY : show_tty("TTY ", walk->n, walk->u); break;
    case SEL_SESS: show_pid("SESS", walk->n, walk->u); break;
    case SEL_COMM: show_cmd("COMM", walk->n, walk->u); break;
    default: printf("Garbage typecode value!\n");
    }
    walk = walk->next;
  }
}

#endif
//////////////////////////////////////////////////////////////////////////


/***** check the header */
/* Unix98: must not print empty header */
static void check_headers(void){
  format_node *walk = format_list;
  int head_normal = 0;
  if(header_type==HEAD_MULTI){
    header_gap = screen_rows-1;  /* true BSD */
    return;
  }
  if(header_type==HEAD_NONE){
    lines_to_next_header = -1;  /* old Linux */
    return;
  }
  while(walk){
    if(!*(walk->name)){
      walk = walk->next;
      continue;
    }
    if(walk->pr){
      head_normal++;
      walk = walk->next;
      continue;
    }
    walk = walk->next;
  }
  if(!head_normal) lines_to_next_header = -1; /* how UNIX does --noheader */
}

static unsigned needs_for_format;
static unsigned needs_for_sort;

/***** check needs */
/* see what files need to be read, etc. */
static void check_needs(void){
  format_node *walk_pr = format_list;
  sort_node   *walk_sr = sort_list;
  /* selection doesn't currently have expensive needs */

  while(walk_pr){
    needs_for_format |= walk_pr->need;
    walk_pr = walk_pr->next;
  }
  if(bsd_e_option) needs_for_format |= PROC_FILLENV;

  while(walk_sr){
    needs_for_sort |= walk_sr->need;
    walk_sr = walk_sr->next;
  }

}

/***** fill in %CPU; not in libproc because of include_dead_children */
/* Note: for sorting, not display, so 0..0x7fffffff would be OK */
static void fill_pcpu(proc_t *buf){
  unsigned long long used_jiffies;
  unsigned long pcpu = 0;
  unsigned long long avail_jiffies;

  used_jiffies = buf->utime + buf->stime;
  if(include_dead_children) used_jiffies += (buf->cutime + buf->cstime);

  avail_jiffies = seconds_since_boot * Hertz - buf->start_time;
  if(avail_jiffies) pcpu = (used_jiffies << 24) / avail_jiffies;

  buf->pcpu = pcpu;  // fits in an int, summing children on 128 CPUs
}

/***** figure out what we need */
static void compute_needs(void){
  if(bsd_c_option){
    needs_for_format &= ~PROC_FILLARG;
    needs_for_sort   &= ~PROC_FILLARG;
  }
  if(!unix_f_option){
    needs_for_format &= ~PROC_FILLCOM;
    needs_for_sort   &= ~PROC_FILLCOM;
  }
}

/***** just display */
static void simple_spew(void){
  proc_t buf;
  PROCTAB* ptp;
  ptp = openproc(needs_for_format | needs_for_sort);
  if(!ptp) {
    fprintf(stderr, "Error: can not access /proc.\n");
    exit(1);
  }
  memset(&buf, '#', sizeof(proc_t));
  /* use "ps_" prefix to catch library mismatch */
  while(ps_readproc(ptp,&buf)){
    if(want_this_proc(&buf)) show_one_proc(&buf);
    if(buf.cmdline) free((void*)*buf.cmdline); // ought to reuse
    if(buf.environ) free((void*)*buf.environ); // ought to reuse
//    memset(&buf, '#', sizeof(proc_t));
  }
  closeproc(ptp);
}

/***** forest output requires sorting by ppid; add start_time by default */
static void prep_forest_sort(void){
  sort_node *tmp_list = sort_list;
  const format_struct *incoming;

  if(!sort_list) {     /* assume start time order */
    incoming = search_format_array("start_time");
    if(!incoming) fprintf(stderr, "Could not find start_time!\n");
    tmp_list = malloc(sizeof(sort_node));
    tmp_list->reverse = 0;
    tmp_list->typecode = '?'; /* what was this for? */
    tmp_list->sr = incoming->sr;
    tmp_list->need = incoming->need;
    tmp_list->next = sort_list;
    sort_list = tmp_list;
  }
  /* this is required for the forest option */
  incoming = search_format_array("ppid");
  if(!incoming) fprintf(stderr, "Could not find ppid!\n");
  tmp_list = malloc(sizeof(sort_node));
  tmp_list->reverse = 0;
  tmp_list->typecode = '?'; /* what was this for? */
  tmp_list->sr = incoming->sr;
  tmp_list->need = incoming->need;
  tmp_list->next = sort_list;
  sort_list = tmp_list;
}

/* we rely on the POSIX requirement for zeroed memory */
static proc_t *processes[98*1024];  // FIXME

/***** compare function for qsort */
static int compare_two_procs(const void *a, const void *b){
  sort_node *tmp_list = sort_list;
  while(tmp_list){
    int result;
    result = (*tmp_list->sr)(*(const proc_t *const*)a, *(const proc_t *const*)b);
    if(result) return (tmp_list->reverse) ? -result : result;
    tmp_list = tmp_list->next;
  }
  return 0; /* no conclusion */
}

/***** show pre-sorted array of process pointers */
static void show_proc_array(int n){
  proc_t **p = processes;
  while(n--){
    show_one_proc(*p);
    /* no point freeing any of this -- won't need more mem */
//    if((*p)->cmdline) free((void*)*(*p)->cmdline);
//    if((*p)->environ) free((void*)*(*p)->environ);
//    memset(*p, '%', sizeof(proc_t)); /* debug */
//    free(*p);
    p++;
  }
}

/***** show tree */
/* this needs some optimization work */
#define ADOPTED(x) 1
static void show_tree(const int self, const int n, const int level, const int have_sibling){
  int i = 0;
  if(level){
    /* add prefix of "+" or "L" */
    if(have_sibling) forest_prefix[level-1] = '+';
    else             forest_prefix[level-1] = 'L';
    forest_prefix[level] = '\0';
  }
  show_one_proc(processes[self]);  /* first show self */
  /* no point freeing any of this -- won't need more mem */
//  if(processes[self]->cmdline) free((void*)*processes[self]->cmdline);
//  if(processes[self]->environ) free((void*)*processes[self]->environ);
  for(;;){  /* look for children */
    if(i >= n) return; /* no children */
    if(processes[i]->ppid == processes[self]->pid) break;
    i++;
  }
  if(level){
    /* change our prefix to "|" or " " for the children */
    if(have_sibling) forest_prefix[level-1] = '|';
    else             forest_prefix[level-1] = ' ';
    forest_prefix[level] = '\0';
  }
  for(;;){
    int self_pid;
    int more_children = 1;
    if(i >= n) break; /* over the edge */
    self_pid=processes[self]->pid;
    if(i+1 >= n)
      more_children = 0;
    else
      if(processes[i+1]->ppid != self_pid) more_children = 0;
    if(self_pid==1 && ADOPTED(processes[i]) && forest_type!='u')
      show_tree(i++, n, level,   more_children);
    else
      show_tree(i++, n, level+1, more_children);
    if(!more_children) break;
  }
  /* chop prefix that children added -- do we need this? */
  forest_prefix[level] = '\0';
//  memset(processes[self], '$', sizeof(proc_t));  /* debug */
}

/***** show forest */
static void show_forest(const int n){
  int i = n;
  int j;
  while(i--){   /* cover whole array looking for trees */
    j = n;
    while(j--){   /* search for parent: if none, i is a tree! */
      if(processes[j]->pid == processes[i]->ppid) goto not_root;
    }
    show_tree(i,n,0,0);
not_root:
    ;
  }
  /* don't free the array because it takes time and ps will exit anyway */
}

/***** sorted or forest */
static void fancy_spew(void){
  proc_t *retbuf = NULL;
  PROCTAB *restrict ptp;
  int n = 0;  /* number of processes & index into array */
  ptp = openproc(needs_for_format | needs_for_sort);
  if(!ptp) {
    fprintf(stderr, "Error: can not access /proc.\n");
    exit(1);
  }
  while((retbuf = ps_readproc(ptp,retbuf))){
    if(want_this_proc(retbuf)){
      fill_pcpu(retbuf); // in case we might sort by %cpu
      processes[n++] = retbuf;
      retbuf = NULL;  /* NULL asks ps_readproc to allocate */
    }
  }
  if(retbuf) free(retbuf);
  closeproc(ptp);
  if(!n) return;  /* no processes */
  if(forest_type) prep_forest_sort();
  qsort(processes, n, sizeof(proc_t*), compare_two_procs);
  if(forest_type) show_forest(n);
  else show_proc_array(n);
}


/***** no comment */
int main(int argc, char *argv[]){
#ifdef DEBUG
  init_stack_trace(argv[0]);
#else
  do {
    struct sigaction sa;
    int i = 32;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigfillset(&sa.sa_mask);
    while(i--) switch(i){
    default:
      sigaction(i,&sa,NULL);
    case 0:
    case SIGINT:   /* ^C */
    case SIGQUIT:  /* ^\ */
    case SIGPROF:  /* profiling */
    case SIGKILL:  /* can not catch */
    case SIGSTOP:  /* can not catch */
    case SIGWINCH: /* don't care if window size changes */
      ;
    }
  } while (0);
#endif

  reset_global();  /* must be before parser */
  arg_parse(argc,argv);

/*  arg_show(); */
  trace("screen is %ux%u\n",screen_cols,screen_rows);
/*  printf("sizeof(proc_t) is %d.\n", sizeof(proc_t)); */
  trace("======= ps output follows =======\n");

  init_output(); /* must be between parser and output */
  check_headers();

  check_needs();
  /* filthy hack -- got to unify some stuff here */
  if( ( (needs_for_format|needs_for_sort) & PROC_FILLWCHAN) && !wchan_is_number)
    if (open_psdb(namelist_file)) wchan_is_number = 1;
  compute_needs();

  if(forest_type || sort_list) fancy_spew(); /* sort or forest */
  else simple_spew(); /* no sort, no forest */
  show_one_proc((proc_t *)-1); /* no output yet? */
  return 0;
}
