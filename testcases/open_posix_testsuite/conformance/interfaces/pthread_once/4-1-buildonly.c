  /*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

  Test pthread_once()
  
  *The constant PTHREAD_ONCE_INIT is defined in the pthread.h header.
  */

#include <pthread.h>

pthread_once_t dummy = PTHREAD_ONCE_INIT;


