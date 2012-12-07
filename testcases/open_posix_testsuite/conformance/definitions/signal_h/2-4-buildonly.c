  /*
     Test the definition of SIG_IGN.
   */

#include <signal.h>

void (*dummy) (int) = SIG_IGN;
