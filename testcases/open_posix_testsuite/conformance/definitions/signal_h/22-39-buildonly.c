  /*
     Test that the SI_ASYNCIO macro is defined.
   */

#include <signal.h>

#ifndef SI_ASYNCIO
#error SI_ASYNCIO not defined
#endif
