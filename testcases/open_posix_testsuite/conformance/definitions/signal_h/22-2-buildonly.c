  /*
     Test that the ILL_ILLOPN macro is defined.
   */

#include <signal.h>

#ifndef ILL_ILLOPN
#error ILL_ILLOPN not defined
#endif
