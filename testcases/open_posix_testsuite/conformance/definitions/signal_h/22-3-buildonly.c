  /*
     Test that the ILL_ILLADR macro is defined.
   */

#include <signal.h>

#ifndef ILL_ILLADR
#error ILL_ILLADR not defined
#endif
