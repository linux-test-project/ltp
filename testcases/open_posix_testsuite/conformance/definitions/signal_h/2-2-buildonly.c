  /*
     Test the definition of SIG_ERR.
   */

#include <signal.h>

void (*dummy) (int) = SIG_ERR;
