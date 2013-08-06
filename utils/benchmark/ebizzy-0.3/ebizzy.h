/* Portability stuff */

#ifndef EBIZZY_H
#define EBIZZY_H

#ifndef _freebsd
#include <malloc.h>
#endif

/*
 * Solaris and FreeBSD compatibility stuff
 */
#if defined(_solaris) || defined(_freebsd)
#define	MAP_ANONYMOUS	MAP_ANON
#define	M_MMAP_MAX	(-4)
#endif

/*
 * FreeBSD compatibility stuff
 */
#if defined(_freebsd) || defined(__UCLIBC__)
#define mallopt(arg1, arg2) do { } while (0);
#endif

/*
 * HP-UX compatibility stuff
 */
#ifdef _HPUX_SOURCE
#define _SC_NPROCESSORS_ONLN pthread_num_processors_np()
#endif



#endif /* EBIZZY_H */
