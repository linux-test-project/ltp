// Copyright (C) 1992-1998 by Michael K. Johnson, johnsonm@redhat.com
// Copyright 1998-2002 Albert Cahalan
//
// This file is placed under the conditions of the GNU Library
// General Public License, version 2, or any later version.
// See file COPYING for information on distribution conditions.

/* File for parsing top-level /proc entities. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#include <unistd.h>
#include <fcntl.h>
#include "version.h"
#include "sysinfo.h" /* include self to verify prototypes */

#ifndef HZ
#include <netinet/in.h>  /* htons */
#endif

long smp_num_cpus;     /* number of CPUs */

#define BAD_OPEN_MESSAGE					\
"Error: /proc must be mounted\n"				\
"  To mount /proc at boot you need an /etc/fstab line like:\n"	\
"      /proc   /proc   proc    defaults\n"			\
"  In the meantime, mount /proc /proc -t proc\n"

#define STAT_FILE    "/proc/stat"
static int stat_fd = -1;
#define UPTIME_FILE  "/proc/uptime"
static int uptime_fd = -1;
#define LOADAVG_FILE "/proc/loadavg"
static int loadavg_fd = -1;
#define MEMINFO_FILE "/proc/meminfo"
static int meminfo_fd = -1;
#define VMINFO_FILE "/proc/vmstat"
static int vminfo_fd = -1;

static char buf[1024];

/* This macro opens filename only if necessary and seeks to 0 so
 * that successive calls to the functions are more efficient.
 * It also reads the current contents of the file into the global buf.
 */
#define FILE_TO_BUF(filename, fd) do{				\
    static int local_n;						\
    if (fd == -1 && (fd = open(filename, O_RDONLY)) == -1) {	\
	fprintf(stderr, BAD_OPEN_MESSAGE);			\
	fflush(NULL);						\
	_exit(102);						\
    }								\
    lseek(fd, 0L, SEEK_SET);					\
    if ((local_n = read(fd, buf, sizeof buf - 1)) < 0) {	\
	perror(filename);					\
	fflush(NULL);						\
	_exit(103);						\
    }								\
    buf[local_n] = '\0';					\
}while(0)

/* evals 'x' twice */
#define SET_IF_DESIRED(x,y) do{  if(x) *(x) = (y); }while(0)


/***********************************************************************/
int uptime(double *restrict uptime_secs, double *restrict idle_secs) {
    double up=0, idle=0;
    char *restrict savelocale;

    FILE_TO_BUF(UPTIME_FILE,uptime_fd);
    savelocale = setlocale(LC_NUMERIC, NULL);
    setlocale(LC_NUMERIC,"C");
    if (sscanf(buf, "%lf %lf", &up, &idle) < 2) {
        setlocale(LC_NUMERIC,savelocale);
        fprintf(stderr, "bad data in " UPTIME_FILE "\n");
	    return 0;
    }
    setlocale(LC_NUMERIC,savelocale);
    SET_IF_DESIRED(uptime_secs, up);
    SET_IF_DESIRED(idle_secs, idle);
    return up;	/* assume never be zero seconds in practice */
}

/***********************************************************************
 * Some values in /proc are expressed in units of 1/HZ seconds, where HZ
 * is the kernel clock tick rate. One of these units is called a jiffy.
 * The HZ value used in the kernel may vary according to hacker desire.
 * According to Linus Torvalds, this is not true. He considers the values
 * in /proc as being in architecture-dependant units that have no relation
 * to the kernel clock tick rate. Examination of the kernel source code
 * reveals that opinion as wishful thinking.
 *
 * In any case, we need the HZ constant as used in /proc. (the real HZ value
 * may differ, but we don't care) There are several ways we could get HZ:
 *
 * 1. Include the kernel header file. If it changes, recompile this library.
 * 2. Use the sysconf() function. When HZ changes, recompile the C library!
 * 3. Ask the kernel. This is obviously correct...
 *
 * Linus Torvalds won't let us ask the kernel, because he thinks we should
 * not know the HZ value. Oh well, we don't have to listen to him.
 * Someone smuggled out the HZ value. :-)
 *
 * This code should work fine, even if Linus fixes the kernel to match his
 * stated behavior. The code only fails in case of a partial conversion.
 *
 * Recent update: on some architectures, the 2.4 kernel provides an
 * ELF note to indicate HZ. This may be for ARM or user-mode Linux
 * support. This ought to be investigated. Note that sysconf() is still
 * unreliable, because it doesn't return an error code when it is
 * used with a kernel that doesn't support the ELF note. On some other
 * architectures there may be a system call or sysctl() that will work.
 */

unsigned long long Hertz;

static void old_Hertz_hack(void){
  unsigned long long user_j, nice_j, sys_j, other_j;  /* jiffies (clock ticks) */
  double up_1, up_2, seconds;
  unsigned long long jiffies;
  unsigned h;
  char *restrict savelocale;

  savelocale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  do{
    FILE_TO_BUF(UPTIME_FILE,uptime_fd);  sscanf(buf, "%lf", &up_1);
    /* uptime(&up_1, NULL); */
    FILE_TO_BUF(STAT_FILE,stat_fd);
    sscanf(buf, "cpu %Lu %Lu %Lu %Lu", &user_j, &nice_j, &sys_j, &other_j);
    FILE_TO_BUF(UPTIME_FILE,uptime_fd);  sscanf(buf, "%lf", &up_2);
    /* uptime(&up_2, NULL); */
  } while((long long)( (up_2-up_1)*1000.0/up_1 )); /* want under 0.1% error */
  setlocale(LC_NUMERIC, savelocale);
  jiffies = user_j + nice_j + sys_j + other_j;
  seconds = (up_1 + up_2) / 2;
  h = (unsigned)( (double)jiffies/seconds/smp_num_cpus );
  /* actual values used by 2.4 kernels: 32 64 100 128 1000 1024 1200 */
  switch(h){
  case    9 ...   11 :  Hertz =   10; break; /* S/390 (sometimes) */
  case   18 ...   22 :  Hertz =   20; break; /* user-mode Linux */
  case   30 ...   34 :  Hertz =   32; break; /* ia64 emulator */
  case   48 ...   52 :  Hertz =   50; break;
  case   58 ...   61 :  Hertz =   60; break;
  case   62 ...   65 :  Hertz =   64; break; /* StrongARM /Shark */
  case   95 ...  105 :  Hertz =  100; break; /* normal Linux */
  case  124 ...  132 :  Hertz =  128; break; /* MIPS, ARM */
  case  195 ...  204 :  Hertz =  200; break; /* normal << 1 */
  case  253 ...  260 :  Hertz =  256; break;
  case  393 ...  408 :  Hertz =  400; break; /* normal << 2 */
  case  790 ...  808 :  Hertz =  800; break; /* normal << 3 */
  case  990 ... 1010 :  Hertz = 1000; break; /* ARM */
  case 1015 ... 1035 :  Hertz = 1024; break; /* Alpha, ia64 */
  case 1180 ... 1220 :  Hertz = 1200; break; /* Alpha */
  default:
#ifdef HZ
    Hertz = (unsigned long long)HZ;    /* <asm/param.h> */
#else
    /* If 32-bit or big-endian (not Alpha or ia64), assume HZ is 100. */
    Hertz = (sizeof(long)==sizeof(int) || htons(999)==999) ? 100UL : 1024UL;
#endif
    fprintf(stderr, "Unknown HZ value! (%d) Assume %Ld.\n", h, Hertz);
  }
}

#ifndef AT_CLKTCK
#define AT_CLKTCK       17    /* frequency of times() */
#endif

extern char** environ;

/* for ELF executables, notes are pushed before environment and args */
static unsigned long find_elf_note(unsigned long findme){
  unsigned long *ep = (unsigned long *)environ;
  while(*ep++);
  while(*ep){
    if(ep[0]==findme) return ep[1];
    ep+=2;
  }
  return 42;
}

static void init_libproc(void) __attribute__((constructor));
static void init_libproc(void){
  /* ought to count CPUs in /proc/stat instead of relying
   * on glibc, which foolishly tries to parse /proc/cpuinfo
   */
  smp_num_cpus = sysconf(_SC_NPROCESSORS_CONF); // or _SC_NPROCESSORS_ONLN
  if(smp_num_cpus<1) smp_num_cpus=1; /* SPARC glibc is buggy */

  if(linux_version_code > LINUX_VERSION(2, 4, 0)){ 
    Hertz = find_elf_note(AT_CLKTCK);
    if(Hertz!=42) return;
    fprintf(stderr, "2.4 kernel w/o ELF notes? -- report to albert@users.sf.net\n");
  }
  old_Hertz_hack();
}

/***********************************************************************
 * The /proc filesystem calculates idle=jiffies-(user+nice+sys) and we
 * recover jiffies by adding up the 4 or 5 numbers we are given. SMP kernels
 * (as of pre-2.4 era) can report idle time going backwards, perhaps due
 * to non-atomic reads and updates. There is no locking for these values.
 */
#ifndef NAN
#define NAN (-0.0)
#endif
#define JT unsigned long long
void five_cpu_numbers(double *restrict uret, double *restrict nret, double *restrict sret, double *restrict iret, double *restrict wret){
    double tmp_u, tmp_n, tmp_s, tmp_i, tmp_w;
    double scale;  /* scale values to % */
    static JT old_u, old_n, old_s, old_i, old_w;
    JT new_u, new_n, new_s, new_i, new_w;
    JT ticks_past; /* avoid div-by-0 by not calling too often :-( */

    tmp_w = 0.0;
    new_w = 0;
 
    FILE_TO_BUF(STAT_FILE,stat_fd);
    sscanf(buf, "cpu %Lu %Lu %Lu %Lu %Lu", &new_u, &new_n, &new_s, &new_i, &new_w);
    ticks_past = (new_u+new_n+new_s+new_i+new_w)-(old_u+old_n+old_s+old_i+old_w);
    if(ticks_past){
      scale = 100.0 / (double)ticks_past;
      tmp_u = ( (double)new_u - (double)old_u ) * scale;
      tmp_n = ( (double)new_n - (double)old_n ) * scale;
      tmp_s = ( (double)new_s - (double)old_s ) * scale;
      tmp_i = ( (double)new_i - (double)old_i ) * scale;
      tmp_w = ( (double)new_w - (double)old_w ) * scale;
    }else{
      tmp_u = NAN;
      tmp_n = NAN;
      tmp_s = NAN;
      tmp_i = NAN;
      tmp_w = NAN;
    }
    SET_IF_DESIRED(uret, tmp_u);
    SET_IF_DESIRED(nret, tmp_n);
    SET_IF_DESIRED(sret, tmp_s);
    SET_IF_DESIRED(iret, tmp_i);
    SET_IF_DESIRED(wret, tmp_w);
    old_u=new_u;
    old_n=new_n;
    old_s=new_s;
    old_i=new_i;
    old_w=new_w;
}
#undef JT

/***********************************************************************/
void loadavg(double *restrict av1, double *restrict av5, double *restrict av15) {
    double avg_1=0, avg_5=0, avg_15=0;
    char *restrict savelocale;
    
    FILE_TO_BUF(LOADAVG_FILE,loadavg_fd);
    savelocale = setlocale(LC_NUMERIC, NULL);
    setlocale(LC_NUMERIC, "C");
    if (sscanf(buf, "%lf %lf %lf", &avg_1, &avg_5, &avg_15) < 3) {
	fprintf(stderr, "bad data in " LOADAVG_FILE "\n");
	exit(1);
    }
    setlocale(LC_NUMERIC, savelocale);
    SET_IF_DESIRED(av1,  avg_1);
    SET_IF_DESIRED(av5,  avg_5);
    SET_IF_DESIRED(av15, avg_15);
}

/***********************************************************************/
/*
 * Copyright 1999 by Albert Cahalan; all rights reserved.
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 */

typedef struct mem_table_struct {
  const char *name;     /* memory type name */
  unsigned *slot; /* slot in return struct */
} mem_table_struct;

static int compare_mem_table_structs(const void *a, const void *b){
  return strcmp(((const mem_table_struct*)a)->name,((const mem_table_struct*)b)->name);
}

/* example data, following junk, with comments added:
 *
 * MemTotal:        61768 kB    old
 * MemFree:          1436 kB    old
 * MemShared:           0 kB    old (now always zero; not calculated)
 * Buffers:          1312 kB    old
 * Cached:          20932 kB    old
 * Active:          12464 kB    new
 * Inact_dirty:      7772 kB    new
 * Inact_clean:      2008 kB    new
 * Inact_target:        0 kB    new
 * Inact_laundry:       0 kB    new, and might be missing too
 * HighTotal:           0 kB
 * HighFree:            0 kB
 * LowTotal:        61768 kB
 * LowFree:          1436 kB
 * SwapTotal:      122580 kB    old
 * SwapFree:        60352 kB    old
 * Inactive:        20420 kB    2.5.41+
 * Dirty:               0 kB    2.5.41+
 * Writeback:           0 kB    2.5.41+
 * Mapped:           9792 kB    2.5.41+
 * Slab:             4564 kB    2.5.41+
 * Committed_AS:     8440 kB    2.5.41+
 * PageTables:        304 kB    2.5.41+
 * ReverseMaps:      5738       2.5.41+
 */

/* obsolete */
unsigned kb_main_shared;
/* old but still kicking -- the important stuff */
unsigned kb_main_buffers;
unsigned kb_main_cached;
unsigned kb_main_free;
unsigned kb_main_total;
unsigned kb_swap_free;
unsigned kb_swap_total;
/* recently introduced */
unsigned kb_high_free;
unsigned kb_high_total;
unsigned kb_low_free;
unsigned kb_low_total;
/* 2.4.xx era */
unsigned kb_active;
unsigned kb_inact_laundry;
unsigned kb_inact_dirty;
unsigned kb_inact_clean;
unsigned kb_inact_target;
unsigned kb_swap_cached;  /* late 2.4 only */
/* derived values */
unsigned kb_swap_used;
unsigned kb_main_used;
/* 2.5.41+ */
unsigned kb_writeback;
unsigned kb_slab;
unsigned nr_reversemaps;
unsigned kb_committed_as;
unsigned kb_dirty;
unsigned kb_inactive;
unsigned kb_mapped;
unsigned kb_pagetables;

void meminfo(void){
  char namebuf[16]; /* big enough to hold any row name */
  mem_table_struct findme = { namebuf, NULL};
  mem_table_struct *found;
  char *head;
  char *tail;
  static const mem_table_struct mem_table[] = {
  {"Active",       &kb_active},
  {"Buffers",      &kb_main_buffers},
  {"Cached",       &kb_main_cached},
  {"Committed_AS", &kb_committed_as},
  {"Dirty",        &kb_dirty},
  {"HighFree",     &kb_high_free},
  {"HighTotal",    &kb_high_total},
  {"Inact_clean",  &kb_inact_clean},
  {"Inact_dirty",  &kb_inact_dirty},
  {"Inact_laundry",&kb_inact_laundry},
  {"Inact_target", &kb_inact_target},
  {"Inactive",     &kb_inactive},
  {"LowFree",      &kb_low_free},
  {"LowTotal",     &kb_low_total},
  {"Mapped",       &kb_mapped},
  {"MemFree",      &kb_main_free},
  {"MemShared",    &kb_main_shared},
  {"MemTotal",     &kb_main_total},
  {"PageTables",   &kb_pagetables},
  {"ReverseMaps",  &nr_reversemaps},
  {"Slab",         &kb_slab},
  {"SwapCached",   &kb_swap_cached},
  {"SwapFree",     &kb_swap_free},
  {"SwapTotal",    &kb_swap_total},
  {"Writeback",    &kb_writeback}
  };
  const int mem_table_count = sizeof(mem_table)/sizeof(mem_table_struct);

  FILE_TO_BUF(MEMINFO_FILE,meminfo_fd);

  kb_inactive = ~0U;

  head = buf;
  for(;;){
    tail = strchr(head, ':');
    if(!tail) break;
    *tail = '\0';
    if(strlen(head) >= sizeof(namebuf)){
      head = tail+1;
      goto nextline;
    }
    strcpy(namebuf,head);
    found = bsearch(&findme, mem_table, mem_table_count,
        sizeof(mem_table_struct), compare_mem_table_structs
    );
    head = tail+1;
    if(!found) goto nextline;
    *(found->slot) = strtoul(head,&tail,10);
nextline:
    tail = strchr(head, '\n');
    if(!tail) break;
    head = tail+1;
  }
  if(!kb_low_total){  /* low==main except with large-memory support */
    kb_low_total = kb_main_total;
    kb_low_free  = kb_main_free;
  }
  if(kb_inactive==~0U){
    kb_inactive = kb_inact_dirty + kb_inact_clean + kb_inact_laundry;
  }
  kb_swap_used = kb_swap_total - kb_swap_free;
  kb_main_used = kb_main_total - kb_main_free;
}

/*****************************************************************/

/* read /proc/vminfo only for 2.5.41 and above */

typedef struct vm_table_struct {
  const char *name;     /* VM statistic name */
  unsigned *slot;       /* slot in return struct */
} vm_table_struct;

static int compare_vm_table_structs(const void *a, const void *b){
  return strcmp(((const vm_table_struct*)a)->name,((const vm_table_struct*)b)->name);
}

unsigned vm_nr_dirty;           // dirty writable pages
unsigned vm_nr_writeback;       // pages under writeback
unsigned vm_nr_pagecache;       // pages in pagecache
unsigned vm_nr_page_table_pages;// pages used for pagetables
unsigned vm_nr_reverse_maps;    // includes PageDirect
unsigned vm_nr_mapped;          // mapped into pagetables
unsigned vm_nr_slab;            // in slab
unsigned vm_pgpgin;             // disk reads  (same as 1st num on /proc/stat page line)
unsigned vm_pgpgout;            // disk writes (same as 2nd num on /proc/stat page line)
unsigned vm_pswpin;             // swap reads  (same as 1st num on /proc/stat swap line)
unsigned vm_pswpout;            // swap writes (same as 2nd num on /proc/stat swap line)
unsigned vm_pgalloc;            // page allocations
unsigned vm_pgfree;             // page freeings
unsigned vm_pgactivate;         // pages moved inactive -> active
unsigned vm_pgdeactivate;       // pages moved active -> inactive
unsigned vm_pgfault;           // total faults (major+minor)
unsigned vm_pgmajfault;       // major faults
unsigned vm_pgscan;          // pages scanned by page reclaim
unsigned vm_pgrefill;       // inspected by refill_inactive_zone
unsigned vm_pgsteal;       // total pages reclaimed
unsigned vm_kswapd_steal; // pages reclaimed by kswapd
// next 3 as defined by the 2.5.52 kernel
unsigned vm_pageoutrun;  // times kswapd ran page reclaim
unsigned vm_allocstall; // times a page allocator ran direct reclaim
unsigned vm_pgrotated; // pages rotated to the tail of the LRU for immediate reclaim

void vminfo(void){
  char namebuf[16]; /* big enough to hold any row name */
  vm_table_struct findme = { namebuf, NULL};
  vm_table_struct *found;
  char *head;
  char *tail;
  static const vm_table_struct vm_table[] = {
  {"allocstall",          &vm_allocstall},
  {"kswapd_steal",        &vm_kswapd_steal},
  {"nr_dirty",            &vm_nr_dirty},
  {"nr_mapped",           &vm_nr_mapped},
  {"nr_page_table_pages", &vm_nr_page_table_pages},
  {"nr_pagecache",        &vm_nr_pagecache},
  {"nr_reverse_maps",     &vm_nr_reverse_maps},
  {"nr_slab",             &vm_nr_slab},
  {"nr_writeback",        &vm_nr_writeback},
  {"pageoutrun",          &vm_pageoutrun},
  {"pgactivate",          &vm_pgactivate},
  {"pgalloc",             &vm_pgalloc},
  {"pgdeactivate",        &vm_pgdeactivate},
  {"pgfault",             &vm_pgfault},
  {"pgfree",              &vm_pgfree},
  {"pgmajfault",          &vm_pgmajfault},
  {"pgpgin",              &vm_pgpgin},
  {"pgpgout",             &vm_pgpgout},
  {"pgrefill",            &vm_pgrefill},
  {"pgrotated",           &vm_pgrotated},
  {"pgscan",              &vm_pgscan},
  {"pgsteal",             &vm_pgsteal},
  {"pswpin",              &vm_pswpin},
  {"pswpout",             &vm_pswpout}
  };
  const int vm_table_count = sizeof(vm_table)/sizeof(vm_table_struct);

  FILE_TO_BUF(VMINFO_FILE,vminfo_fd);

  head = buf;
  for(;;){
    tail = strchr(head, ' ');
    if(!tail) break;
    *tail = '\0';
    if(strlen(head) >= sizeof(namebuf)){
      head = tail+1;
      goto nextline;
    }
    strcpy(namebuf,head);
    found = bsearch(&findme, vm_table, vm_table_count,
        sizeof(vm_table_struct), compare_vm_table_structs
    );
    head = tail+1;
    if(!found) goto nextline;
    *(found->slot) = strtoul(head,&tail,10);
nextline:

//if(found) fprintf(stderr,"%s=%d\n",found->name,*(found->slot));
//else      fprintf(stderr,"%s not found\n",findme.name);

    tail = strchr(head, '\n');
    if(!tail) break;
    head = tail+1;
  }
}
/*****************************************************************/
