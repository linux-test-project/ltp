  /*
     Test that the ILL_PRVREG macro is defined.
   */

#include <signal.h>

#ifndef ILL_PRVREG
#error ILL_PRVREG not defined
#endif
