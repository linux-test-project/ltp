  /*
     Test that the POLL_PRI macro is defined.
   */

#include <signal.h>

#ifndef POLL_PRI
#error POLL_PRI not defined
#endif
