  /*
     Test that the POLL_HUP macro is defined.
   */

#include <signal.h>

#ifndef POLL_HUP
#error POLL_HUP not defined
#endif
