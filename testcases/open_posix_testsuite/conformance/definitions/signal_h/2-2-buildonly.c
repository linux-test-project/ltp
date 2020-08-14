  /*
     Test the definition of SIG_ERR.
   */

#include <signal.h>

static void (*dummy) (int) = SIG_ERR;
