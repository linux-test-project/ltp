
/* timer_create */
#ifndef __NR_timer_create
#if defined(__i386__)
#define __NR_timer_create 259
#elif defined(__ppc__)
#define __NR_timer_create 240
#elif defined(__ppc64__)
#define __NR_timer_create 240
#elif defined(__x86_64__)
#define __NR_timer_create 222
#elif defined(__s390__)
#define __NR_timer_create 254
#elif defined(__ia64__)
#define __NR_timer_create 1248
#elif defined(__h8300__)
#define __NR_timer_create 259
#elif defined(__parisc__)
#define __NR_HPUX_timer_create 348
#endif
#endif
#pragma weak timer_create

/* timer_settime */
#ifndef __NR_timer_settime
#if defined(__parisc__)
#define __NR_timer_settime 350
#else
#define __NR_timer_settime (__NR_timer_create+1)
#endif
#endif
#pragma weak timer_settime

