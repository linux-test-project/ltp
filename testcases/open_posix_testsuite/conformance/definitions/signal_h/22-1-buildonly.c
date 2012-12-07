  /*
     Test that the ILL_ILLOPC macro is defined.
   */

#include <signal.h>

#ifndef ILL_ILLOPC
#error ILL_ILLOPC not defined
#endif
