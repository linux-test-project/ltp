  /*
     Test that the constants below are supported.
   */

#include <signal.h>

int dummy1 = SA_NOCLDSTOP;
int dummy2 = SIG_BLOCK;
int dummy3 = SIG_UNBLOCK;
int dummy4 = SIG_SETMASK;
int dummy5 = SA_ONSTACK;
int dummy6 = SA_RESETHAND;
int dummy7 = SA_RESTART;
int dummy8 = SA_SIGINFO;
int dummy9 = SA_NOCLDWAIT;
int dummy10 = SA_NODEFER;
int dummy11 = SS_ONSTACK;
int dummy12 = SS_DISABLE;
int dummy13 = MINSIGSTKSZ;
int dummy14 = SIGSTKSZ;
