#ifndef PROC_SYSINFO_H
#define PROC_SYSINFO_H

#include "procps.h"

EXTERN_C_BEGIN

extern unsigned long long Hertz;   /* clock tick frequency */
extern long smp_num_cpus;     /* number of CPUs */

#define JT double
extern void five_cpu_numbers(JT *uret, JT *nret, JT *sret, JT *iret, JT *wret);
#undef JT

extern int        uptime (double *uptime_secs, double *idle_secs);
extern void       loadavg(double *av1, double *av5, double *av15);


/* obsolete */
extern unsigned kb_main_shared;
/* old but still kicking -- the important stuff */
extern unsigned kb_main_buffers;
extern unsigned kb_main_cached;
extern unsigned kb_main_free;
extern unsigned kb_main_total;
extern unsigned kb_swap_free;
extern unsigned kb_swap_total;
/* recently introduced */
extern unsigned kb_high_free;
extern unsigned kb_high_total;
extern unsigned kb_low_free;
extern unsigned kb_low_total;
/* 2.4.xx era */
extern unsigned kb_active;
extern unsigned kb_inact_laundry;  // grrr...
extern unsigned kb_inact_dirty;
extern unsigned kb_inact_clean;
extern unsigned kb_inact_target;
extern unsigned kb_swap_cached;  /* late 2.4+ */
/* derived values */
extern unsigned kb_swap_used;
extern unsigned kb_main_used;
/* 2.5.41+ */
extern unsigned kb_writeback;
extern unsigned kb_slab;
extern unsigned nr_reversemaps;
extern unsigned kb_committed_as;
extern unsigned kb_dirty;
extern unsigned kb_inactive;
extern unsigned kb_mapped;
extern unsigned kb_pagetables;

extern void meminfo(void);


extern unsigned vm_nr_dirty;
extern unsigned vm_nr_writeback;
extern unsigned vm_nr_pagecache;
extern unsigned vm_nr_page_table_pages;
extern unsigned vm_nr_reverse_maps;
extern unsigned vm_nr_mapped;
extern unsigned vm_nr_slab;
extern unsigned vm_pgpgin;
extern unsigned vm_pgpgout;
extern unsigned vm_pswpin;
extern unsigned vm_pswpout;
extern unsigned vm_pgalloc;
extern unsigned vm_pgfree;
extern unsigned vm_pgactivate;
extern unsigned vm_pgdeactivate;
extern unsigned vm_pgfault;
extern unsigned vm_pgmajfault;
extern unsigned vm_pgscan;
extern unsigned vm_pgrefill;
extern unsigned vm_pgsteal;
extern unsigned vm_kswapd_steal;
extern unsigned vm_pageoutrun;
extern unsigned vm_allocstall;

extern void vminfo(void);

EXTERN_C_END
#endif /* SYSINFO_H */
