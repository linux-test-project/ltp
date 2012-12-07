  /*
     @pt:CX

     Test the definition of SIG_HOLD.
   */

#include <signal.h>

void (*dummy) (int) = SIG_HOLD;
