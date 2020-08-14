  /*
     Test the definition of SIG_IGN.
   */

#include <signal.h>

static void (*dummy) (int) = SIG_IGN;
