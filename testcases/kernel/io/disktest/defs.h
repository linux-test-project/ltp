#ifndef _DEFS_H
#define _DEFS_H

#include "sys/types.h"

#ifdef WIN32

typedef __int64 OFF_T;
typedef int pid_t;

#else

#define TRUE 1
#define FALSE 0

typedef int BOOL;
typedef void * HANDLE;

/* typedef off_t OFF_T; */
typedef long long int OFF_T;

#endif /* WIN32 */

#endif /* _DEFS_H */

