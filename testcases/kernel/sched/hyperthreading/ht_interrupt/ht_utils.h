
#ifndef _HTUTILS_H_
#define _HTUTILS_H_

#include <stdlib.h>
#include <sys/types.h>

int is_cmdline_para(const char *para);

// return 0 means Pass,
// return 1 means ht is not enabled,
// return 2 means CPU is not support ht,
// return 3 mean kernel is not support ht.
int check_ht_capability();

extern char buf[];

int get_cpu_count();
int get_current_cpu(pid_t pid);

#endif
