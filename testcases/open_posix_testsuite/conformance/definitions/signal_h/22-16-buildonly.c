  /*
     Test that the FPE_FLTSUB macro is defined.
   */

#include <signal.h>

#ifndef FPE_FLTSUB
#error FPE_FLTSUB not defined
#endif
