  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   Test the following symbols are defined by pthread.h
   */

#include <pthread.h>

#ifndef PTHREAD_CANCEL_ASYNCHRONOUS
#error PTHREAD_CANCEL_ASYNCHRONOUS not defined
#endif

#ifndef PTHREAD_CANCEL_ENABLE
#error PTHREAD_CANCEL_ENABLE not defined
#endif

#ifndef PTHREAD_CANCEL_DEFERRED
#error PTHREAD_CANCEL_DEFERRED not defined
#endif

#ifndef PTHREAD_CANCEL_DISABLE
#error PTHREAD_CANCEL_DISABLE not defined
#endif

#ifndef PTHREAD_CANCELED
#error PTHREAD_CANCELED not defined
#endif

#ifndef PTHREAD_COND_INITIALIZER
#error PTHREAD_COND_INTIALIZER not defined
#endif

#ifndef PTHREAD_CREATE_DETACHED
#error PTHREAD_CREATE_DETACHED not defined
#endif

#ifndef PTHREAD_CREATE_JOINABLE
#error PTHREAD_CREATE_JOINABLE not defined
#endif

#ifndef PTHREAD_EXPLICIT_SCHED
#error PTHREAD_EXPLICIT_SCHED not defined
#endif

#ifndef PTHREAD_INHERIT_SCHED
#error PTHREAD_INHERIT_SCHED not defined
#endif

#ifndef PTHREAD_MUTEX_INITIALIZER
#error PTHREAD_MUTEX_INITIALIZED not defined
#endif

#ifndef PTHREAD_ONCE_INIT
#error PTHREAD_ONCE_INIT not defined
#endif

#ifndef PTHREAD_PROCESS_SHARED
#error PTHREAD_PROCESS_SHARED not defined
#endif

#ifndef PTHREAD_PROCESS_PRIVATE
#error PTHREAD_PROCESS_PRIVATE not defined
#endif
