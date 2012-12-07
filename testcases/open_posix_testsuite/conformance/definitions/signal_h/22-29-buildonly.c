  /*
     Test that the CLD_CONTINUED macro is defined.
   */

#include <signal.h>

#ifndef CLD_CONTINUED
#error CLD_CONTINUED not defined
#endif
