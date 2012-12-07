  /*
     Test that the FPE_FLTDIV macro is defined.
   */

#include <signal.h>

#ifndef FPE_FLTDIV
#error FPE_FLTDIV not defined
#endif
