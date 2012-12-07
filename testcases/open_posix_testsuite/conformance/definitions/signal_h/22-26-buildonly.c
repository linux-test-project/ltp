  /*
     Test that the CLD_DUMPED macro is defined.
   */

#include <signal.h>

#ifndef CLD_DUMPED
#error CLD_DUMPED not defined
#endif
