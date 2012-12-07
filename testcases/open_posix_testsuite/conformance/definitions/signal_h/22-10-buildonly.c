  /*
     Test that the FPE_INTOVF macro is defined.
   */

#include <signal.h>

#ifndef FPE_INTOVF
#error FPE_INTOVF not defined
#endif
