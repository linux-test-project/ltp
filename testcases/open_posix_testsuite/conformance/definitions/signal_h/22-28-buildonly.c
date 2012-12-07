  /*
     Test that the CLD_STOPPED macro is defined.
   */

#include <signal.h>

#ifndef CLD_STOPPED
#error CLD_STOPPED not defined
#endif
