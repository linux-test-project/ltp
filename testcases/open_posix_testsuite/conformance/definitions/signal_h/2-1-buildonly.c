  /*
     Test the definition of SIG_DFL.
   */

#include <signal.h>

static void (*dummy) (int) = SIG_DFL;
