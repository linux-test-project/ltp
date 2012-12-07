  /*
     Test that the TRAP_TRACE macro is defined.
   */

#include <signal.h>

#ifndef TRAP_TRACE
#error TRAP_TRACE not defined
#endif
