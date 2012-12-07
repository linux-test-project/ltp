  /*
     Test that the SEGV_ACCERR macro is defined.
   */

#include <signal.h>

#ifndef SEGV_ACCERR
#error SEGV_ACCERR not defined
#endif
