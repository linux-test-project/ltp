  /*
     Test that the ILL_BADSTK macro is defined.
   */

#include <signal.h>

#ifndef ILL_BADSTK
#error ILL_BADSTK not defined
#endif
