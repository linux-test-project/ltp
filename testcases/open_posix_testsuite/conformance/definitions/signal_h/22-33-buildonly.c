  /*
     Test that the POLL_ERR macro is defined.
   */

#include <signal.h>

#ifndef POLL_ERR
#error POLL_ERR not defined
#endif
