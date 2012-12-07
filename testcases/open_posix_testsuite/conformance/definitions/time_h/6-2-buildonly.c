  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   @pt:TMR
   Test that TIMER_ABSTIME is defined
   */

#include <time.h>

#ifndef TIMER_ABSTIME
#error TIMER_ABSTIME not defined
#endif
