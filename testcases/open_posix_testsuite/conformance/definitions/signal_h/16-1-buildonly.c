  /*
     Test that the constants below are supported.
   */

#include <signal.h>

static int dummy1 = SA_NOCLDSTOP;
static int dummy2 = SIG_BLOCK;
static int dummy3 = SIG_UNBLOCK;
static int dummy4 = SIG_SETMASK;
static int dummy5 = SA_ONSTACK;
static int dummy6 = SA_RESETHAND;
static int dummy7 = SA_RESTART;
static int dummy8 = SA_SIGINFO;
static int dummy9 = SA_NOCLDWAIT;
static int dummy10 = SA_NODEFER;
static int dummy11 = SS_ONSTACK;
static int dummy12 = SS_DISABLE;
static int dummy13 = MINSIGSTKSZ;
static int dummy14 = SIGSTKSZ;
