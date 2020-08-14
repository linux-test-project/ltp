  /*
     @pt:CX

     Test the definition of SIG_HOLD.
   */

#include <signal.h>

static void (*dummy) (int) = SIG_HOLD;
