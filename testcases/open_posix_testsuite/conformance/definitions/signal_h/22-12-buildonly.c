  /*
     Test that the FPE_FLTOVF macro is defined.
   */

#include <signal.h>

#ifndef FPE_FLTOVF
#error FPE_FLTOVF not defined
#endif
