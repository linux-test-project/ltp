
  /*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

  Test the following symbols are defined by pthread.h
  */

#include <pthread.h>

/* BAR */
#ifndef PTHREAD_BARRIER_SERIAL_THREAD
#error PTHREAD_BARRIER_SERIAL_THREAD not defined
#endif

/* XSI */
#ifndef PTHREAD_MUTEX_DEFAULT
#error PTHREAD_MUTEX_DEFAULT not defined
#endif

/* XSI */
#ifndef PTHREAD_MUTEX_ERRORCHECK
#error PTHREAD_MUTEX_ERRORCHECK not defined
#endif

/* XSI */
#ifndef PTHREAD_MUTEX_NORMAL
#error PTHREAD_MUTEX_NORMAL not defined
#endif

/* XSI */
#ifndef PTHREAD_MUTEX_RECURSIVE
#error PTHREAD_MUTEX_RECURSIVE not defined
#endif 

/* TPP|TPI */
#ifndef PTHREAD_PRIO_INHERIT
#error PTHREAD_PRIO_INHERIT not defined
#endif

/* TPP|TPI */
#ifndef PTHREAD_PRIO_NONE
#error PTHREAD_PRIO_NONE not defined
#endif

/* TPP|TPI */
#ifndef PTHREAD_PRIO_PROTECT
#error PTHREAD_PRIO_PROTECT not defined
#endif

/* TPS */
#ifndef PTHREAD_SCOPE_PROCESS
#error PTHREAD_SCOPE_PROCESS not defined
#endif

/* TPS */
#ifndef PTHREAD_SCOPE_SYSTEM
#error PTHREAD_SCOPE_SYSTEM not defined
#endif

