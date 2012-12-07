  /*
     Test that the CLD_EXITED macro is defined.
   */

#include <signal.h>

#ifndef CLD_EXITED
#error CLD_EXITED not defined
#endif
