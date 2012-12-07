  /*
     Test that the CLD_KILLED macro is defined.
   */

#include <signal.h>

#ifndef CLD_KILLED
#error CLD_KILLED not defined
#endif
