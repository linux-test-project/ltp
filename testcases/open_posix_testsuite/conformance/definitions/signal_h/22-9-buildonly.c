  /*
     Test that the FPE_INTDIV macro is defined.
   */

#include <signal.h>

#ifndef FPE_INTDIV
#error FPE_INTDIV not defined
#endif
