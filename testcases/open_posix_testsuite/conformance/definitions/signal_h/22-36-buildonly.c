  /*
     Test that the SI_USER macro is defined.
   */

#include <signal.h>

#ifndef SI_USER
#error SI_USER not defined
#endif
