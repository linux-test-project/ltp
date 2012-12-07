  /*
     Test that the FPE_FLTINV macro is defined.
   */

#include <signal.h>

#ifndef FPE_FLTINV
#error FPE_FLTINV not defined
#endif
