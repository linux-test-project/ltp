  /*
     Test that the FPE_FLTUND macro is defined.
   */

#include <signal.h>

#ifndef FPE_FLTUND
#error FPE_FLTUND not defined
#endif
