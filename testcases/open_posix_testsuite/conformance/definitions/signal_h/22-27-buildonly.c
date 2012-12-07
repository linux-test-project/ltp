  /*
     Test that the CLD_TRAPPED macro is defined.
   */

#include <signal.h>

#ifndef CLD_TRAPPED
#error CLD_TRAPPED not defined
#endif
