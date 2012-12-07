  /*
     Test that the ILL_PRVOPC macro is defined.
   */

#include <signal.h>

#ifndef ILL_PRVOPC
#error ILL_PRVOPC not defined
#endif
