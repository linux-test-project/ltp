  /*
     Test that the TRAP_BRKPT macro is defined.
   */

#include <signal.h>

#ifndef TRAP_BRKPT
#error TRAP_BRKPT not defined
#endif
