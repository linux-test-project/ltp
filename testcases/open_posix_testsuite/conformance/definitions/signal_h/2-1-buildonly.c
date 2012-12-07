  /*
     Test the definition of SIG_DFL.
   */

#include <signal.h>

void (*dummy) (int) = SIG_DFL;
