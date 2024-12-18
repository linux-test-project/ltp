#define TST_NO_DEFAULT_MAIN

#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <errno.h>
#include <fcntl.h>
#if HAVE_NUMA_H
#include <numa.h>
#endif
#if HAVE_NUMAIF_H
#include <numaif.h>
#endif
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "mem.h"
#include "numa_helper.h"


/* KSM */

