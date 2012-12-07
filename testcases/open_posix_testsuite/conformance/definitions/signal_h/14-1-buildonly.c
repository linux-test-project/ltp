  /*
     Test that the signals below are supported.
   */

#include <signal.h>

int dummy1 = SIGABRT;
int dummy2 = SIGALRM;
int dummy3 = SIGBUS;
int dummy4 = SIGCHLD;
int dummy5 = SIGCONT;
int dummy6 = SIGFPE;
int dummy7 = SIGHUP;
int dummy8 = SIGILL;
int dummy9 = SIGINT;
int dummy10 = SIGKILL;
int dummy11 = SIGPIPE;
int dummy12 = SIGQUIT;
int dummy13 = SIGSEGV;
int dummy14 = SIGSTOP;
int dummy15 = SIGTERM;
int dummy16 = SIGTSTP;
int dummy17 = SIGTTIN;
int dummy18 = SIGTTOU;
int dummy19 = SIGUSR1;
int dummy20 = SIGUSR2;
#ifdef SIGPOLL
int dummy21 = SIGPOLL;
#endif
#ifdef SIGPROF
int dummy22 = SIGPROF;
#endif
int dummy23 = SIGSYS;
int dummy24 = SIGTRAP;
int dummy25 = SIGURG;
int dummy26 = SIGVTALRM;
int dummy27 = SIGXCPU;
int dummy28 = SIGXFSZ;
