/* free.c - a /proc implementation of free */
/* Dec14/92 by Brian Edmonds */
/* Thanks to Rafal Maszkowski for the Total line */

#include "proc/sysinfo.h"
#include "proc/version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#define S(X) ( ((unsigned long long)(X) << 10) >> byteshift)

static int byteshift = 10;
static int total = 0;

int main(int argc, char *argv[]){
    int i;
    int old_fmt = 0;
    int rtime = 0;

    /* check startup flags */
    while( (i = getopt(argc, argv, "bkmos:tV") ) != -1 )
        switch (i) {
        case 'b': byteshift = 0;  break;
        case 'k': byteshift = 10; break;
        case 'm': byteshift = 20; break;
        case 'o': old_fmt = 1; break;
        case 's': rtime = 1000000 * atof(optarg); break;
        case 't': total = 1; break;
	case 'V': display_version(); exit(0);
        default:
	  fprintf(stderr, "usage: %s [-b|-k|-m] [-o] [-s delay] [-t] [-V]\n", argv[0]);
	  return 1;
    }

    do {
        meminfo();
        printf("             total       used       free     shared    buffers     cached\n");
        printf(
            "%-7s %10Ld %10Ld %10Ld %10Ld %10Ld %10Ld\n", "Mem:",
            S(kb_main_total),
            S(kb_main_used),
            S(kb_main_free),
            S(kb_main_shared),
            S(kb_main_buffers),
            S(kb_main_cached)
        );
        if(!old_fmt){
            printf(
                "-/+ buffers/cache: %10Ld %10Ld\n", 
                S(kb_main_used-kb_main_buffers-kb_main_cached),
                S(kb_main_free+kb_main_buffers+kb_main_cached)
            );
        }
        printf(
            "%-7s %10Ld %10Ld %10Ld\n", "Swap:",
            S(kb_swap_total),
            S(kb_swap_used),
            S(kb_swap_free)
        );
        if(total == 1){
            printf(
                "%-7s %10Ld %10Ld %10Ld\n", "Total:",
                S(kb_main_total + kb_swap_total),
                S(kb_main_used  + kb_swap_used),
                S(kb_main_free  + kb_swap_free)
            );
        }
        if(rtime){
	    fputc('\n', stdout);
	    fflush(stdout);
	    usleep(rtime);
	}
    } while(rtime);

    return 0;
}
