  /*
     Test that the FPE_FLTRES macro is defined.
   */

#include <signal.h>

#ifndef FPE_FLTRES
#error FPE_FLTRES not defined
#endif
