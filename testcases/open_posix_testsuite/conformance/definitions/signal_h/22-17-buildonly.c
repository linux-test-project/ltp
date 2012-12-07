  /*
     Test that the SEGV_MAPERR macro is defined.
   */

#include <signal.h>

#ifndef SEGV_MAPERR
#error SEGV_MAPERR not defined
#endif
