/*
 *
 * Copyright 1994 Charles Blake and Michael K. Johnson
 * This file is a part of procps, which is distributable
 * under the conditions of the GNU Library General Public License.
 * See the file COPYING for details.
 *
 * Copyright 2002 Albert Cahalan
 */

#include <string.h>		/* for strcmp */
#include <stdio.h>		/* for parse error output */

#include "readproc.h"	/* for proc_t */

#include "compare.h"	/* for this code */


/*
  This module was written by Charles Blake for procps.

mult_lvl_cmp:
    slick general purpose multi-level compare function I invented.
sort_depth:
    the number of levels of functions *to use*.  This means many more levels
    can be defined than mult_lvl_cmp tres out.  If this is 1 then mult_lvl_cmp
    is just a trivial wrapper around (*sort_function[0]).
sort_direction:
    multiplicative factor for the output of cmp_whatever.
    1 ==> default order, -1 ==> reverse order, 0 ==> forced equality
    The 0 bit is the neat part.  Since a value of zero is the code for equality
    multiplying the output of cmp_foo(a,b) forces a==b to be true.  This is a
    convenient way to turn sorting off in middle levels of a multi-level sort.
    If time is a problem, reforming the whole sort_function array to not include
    these unsorted middle levels will be faster since then cmp_foo won't even
    be called.  It might simplify some code depending upon how you organize it.
sort_function[]:
    array of function pointers that points to our family of comparison functions
    (I have named them cmp_* but mult_lvl_cmp doesn't care what they're named).
    This may be declared and initialized like so:
       int (*sort_function[])(void* a, void* b)={&cmp_foo, &cmp_bar, &cmp_hiho};
    You could also use my command line '-O' parser below.

Note that we only descend levels until the order is determined.  If we descend
all levels, that means that the items are equal at all levels, so we return 0.
Otherwise we return whatever the level's cmp_foo function would have returned.
This allows whatever default behavior you want for cmp_foo.  sort_direction[]
reverses this default behavior, but mult_lvl_cmp doesn't decide that ascending
or descending is the default.  That is the job of your cmp_foo's.
*/

/* the only reason these are global is because qsort(3) likes it that way.
   It's also a little more efficient if mult_lvl_cmp() is called many times.
*/

static int sort_depth = 0;
static int sort_direction[10];     /* storage for 10 levels, but 4 would be plenty!*/
static int (*sort_function[10])(void* a, void* b);

int mult_lvl_cmp(void* a, void* b) {
    int i, cmp_val;
    for (i = 0; i < sort_depth; i++) {
        cmp_val = sort_direction[i] * (*sort_function[i])(a,b);
        if (cmp_val != 0)
            return cmp_val;
    }
    return 0;
}

/* qsort(3) compliant comparison functions for all members of the ps_proc
   structure (in the same order in which they appear in the proc_t declaration)
   return is {-1,0,1} as {a<b, a==b, a>b}
   default ordering is ascending for all members. (flip 1,-1 to reverse)
*/
/* pre-processor macros to cut down on source size (and typing!)
   Note the use of the string concatenation operator ##
*/
#define CMP_STR(NAME) \
static int cmp_ ## NAME(proc_t** P, proc_t** Q) { \
    return strcmp((*P)->NAME, (*Q)->NAME); \
}

#define CMP_INT(NAME) \
static int cmp_ ## NAME (proc_t** P, proc_t** Q) { \
    if ((*P)->NAME < (*Q)->NAME) return -1; \
    if ((*P)->NAME > (*Q)->NAME) return  1; \
    return 0; \
}

/* Define the (46!) cmp_ functions with the above macros for every element
   of proc_t.  If the binary gets too big, we could nuke inessentials.
*/

/* CMP_STR(cmdline) */
/* CMP_STR(ruser) */
CMP_STR(euser)
CMP_STR(cmd)
/* CMP_INT(state) */
/* CMP_STR(ttyc) */
CMP_INT(euid)
CMP_INT(pid)
CMP_INT(ppid)
CMP_INT(pgrp)
CMP_INT(session)
CMP_INT(tty)
CMP_INT(tpgid)
CMP_INT(utime)
CMP_INT(stime)
CMP_INT(cutime)
CMP_INT(cstime)
/* CMP_INT(priority) */
CMP_INT(nice)
CMP_INT(start_time)
/* CMP_INT(signal) */
/* CMP_INT(blocked) */
/* CMP_INT(sigignore) */
/* CMP_INT(sigcatch) */
CMP_INT(flags)
CMP_INT(min_flt)
CMP_INT(cmin_flt)
CMP_INT(maj_flt)
CMP_INT(cmaj_flt)
/* CMP_INT(timeout) */
CMP_INT(vsize)
CMP_INT(rss)
/* CMP_INT(rss_rlim) */
/* CMP_INT(start_code) */
/* CMP_INT(end_code) */
/* CMP_INT(start_stack) */
/* CMP_INT(kstk_esp) */
/* CMP_INT(kstk_eip) */
/* CMP_INT(wchan) */
CMP_INT(pcpu)
CMP_INT(size)
CMP_INT(resident)
CMP_INT(share)
/* CMP_INT(trs) */
/* CMP_INT(lrs) */
/* CMP_INT(drs) */
/* CMP_INT(dt) */

/* define user interface to sort keys.  Fairly self-explanatory. */

static struct cmp_fun_struct {
    char letter;                           /* single option-letter for key */
    char name[15];                             /* long option name for key */
    int (*fun)(proc_t**, proc_t**);  /* pointer to cmp_key */
} cmp[] = {
/*  { '?', "cmdline",       &cmp_cmdline       }, */
    { 'u', "user",          &cmp_euser         },
/*  { '?', "ruser",         &cmp_ruser         }, */
    { 'c', "cmd",           &cmp_cmd           },
/*  { '?', "state",         &cmp_state         }, */
/*  { '?', "ttyc",          &cmp_ttyc          }, */
    { 'U', "uid",           &cmp_euid          },
    { 'p', "pid",           &cmp_pid           },
    { 'P', "ppid",          &cmp_ppid          },
    { 'g', "pgrp",          &cmp_pgrp          },
    { 'o', "session",       &cmp_session       },
    { 't', "tty",           &cmp_tty           },
    { 'G', "tpgid",         &cmp_tpgid         },
    { 'k', "utime",         &cmp_utime         },
    { 'K', "stime",         &cmp_stime         },
    { 'j', "cutime",        &cmp_cutime        },
    { 'J', "cstime",        &cmp_cstime        },
/*  { '?', "counter",       &cmp_counter       }, */
    { 'y', "priority",      &cmp_nice          },
    { 'T', "start_time",    &cmp_start_time    },
/*  { '?', "signal",        &cmp_signal        }, */
/*  { '?', "blocked",       &cmp_blocked       }, */
/*  { '?', "sigignore",     &cmp_sigignore     }, */
/*  { '?', "sigcatch",      &cmp_sigcatch      }, */
    { 'f', "flags",         &cmp_flags         },
    { 'm', "min_flt",       &cmp_min_flt       },
    { 'n', "cmin_flt",      &cmp_cmin_flt      },
    { 'M', "maj_flt",       &cmp_maj_flt       },
    { 'N', "cmaj_flt",      &cmp_cmaj_flt      },
/*  { 'C', "timeout",       &cmp_timeout       }, */
    { 'v', "vsize",         &cmp_vsize         },
    { 'r', "rss",           &cmp_rss           },
/*  { '?', "rss_rlim",      &cmp_rss_rlim      }, */
/*  { '?', "start_code",    &cmp_start_code    }, */
/*  { '?', "end_code",      &cmp_end_code      }, */
/*  { '?', "start_stack",   &cmp_start_stack   }, */
/*  { '?', "kstk_esp",      &cmp_kstk_esp      }, */
/*  { '?', "kstk_eip",      &cmp_kstk_eip      }, */
/*  { '?', "wchan",         &cmp_wchan         }, */
    { 'C', "pcpu",          &cmp_pcpu          },
    { 's', "size",          &cmp_size          },
    { 'R', "resident",      &cmp_resident      },
    { 'S', "share",         &cmp_share         },
/*  { '?', "trs",           &cmp_trs           }, */
/*  { '?', "lrs",           &cmp_lrs           }, */
/*  { '?', "drs",           &cmp_drs           }, */
/*  { '?', "dt",            &cmp_dt            }, */
    { '\0',"terminator",    NULL               }
};

/* command line option parsing.  Assign sort_{depth,direction[],function[]}
   based upon a string of the form:
        [+-]a[+-]b[+-]c...
   with a,b,c,... being letter flags corresponding to a particular sort
   key and the optional '-' specifying a reverse sort on that key.  + doesn't
   mean anything, but it keeps things looking balanced...
*/
const char *parse_sort_opt(const char* opt) {
    int i, next_dir=1;
    for (; *opt ; ++opt) {
        if (*opt == '-' || *opt == '+') {
            if (*opt == '-')
                next_dir = -1;
	    opt++;
            continue;
        }
        for (i = 0; cmp[i].letter; i++)
            if (*opt == cmp[i].letter)
                break;
        if (!cmp[i].letter) { /* failed, clear and return */
            sort_depth=0;
            for (i=0;i<10;i++) {
              sort_direction[i]=0;
              sort_function[i]=(cmp_t)NULL;
            }
            return "Unknown sort key.";
        } else {
#ifdef DEBUG
	    fprintf(stderr,
		    "sort level %d: key %s, direction % d\n",
		    sort_depth, cmp[i].name, next_dir);
#endif
            sort_function[sort_depth] = (cmp_t)cmp[i].fun;
            sort_direction[sort_depth++] = next_dir;
            next_dir = 1;
        }
    }
    return NULL;
}

const char *parse_long_sort(const char* opt) {
    char* comma;
    int i, more_keys, next_dir=1;
    do {
        if (*opt == '-' || *opt == '+') {
            if (*opt == '-')
                next_dir = -1;
            more_keys = 1;
            opt++;
            continue;
        }
        more_keys = ((comma=index(opt,',')) != NULL);
	                      /* keys are ',' delimited */
        if (more_keys)
            *comma='\0';      /* terminate for strcmp() */
        for (i = 0; cmp[i].letter; ++i)
            if (strcmp(opt, cmp[i].name) == 0)
                break;
        if (!cmp[i].letter) { /* failed, clear and return */
            sort_depth=0;
            for (i=0;i<10;i++) {
              sort_direction[i]=0;
              sort_function[i]=(cmp_t)NULL;
            }
            return "Unknown sort key.";
        } else {
#ifdef DEBUG
	    fprintf(stderr,
		    "sort level %d: key %s, direction % d\n",
		    sort_depth, cmp[i].name, next_dir);
#endif
            sort_function[sort_depth] = (cmp_t)cmp[i].fun;
            sort_direction[sort_depth++] = next_dir;
            next_dir = 1;
        }
        opt = comma + 1; /* do next loop on next key, if more keys, else done*/
    } while (more_keys);
    return NULL;
}

void reset_sort_options (void)
{
  int i;

  sort_depth=0;
  for (i=0;i<10;i++) {
    sort_direction[i]=0;
    sort_function[i]=(cmp_t)NULL;
  }
}

void register_sort_function (int dir, cmp_t func)
{
    sort_function[sort_depth] = func;
    sort_direction[sort_depth++] = dir;
}