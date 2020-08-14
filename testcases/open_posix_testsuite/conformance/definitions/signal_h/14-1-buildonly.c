  /*
     Test that the signals below are supported.
   */

#include <signal.h>

static int dummy1 = SIGABRT;
static int dummy2 = SIGALRM;
static int dummy3 = SIGBUS;
static int dummy4 = SIGCHLD;
static int dummy5 = SIGCONT;
static int dummy6 = SIGFPE;
static int dummy7 = SIGHUP;
static int dummy8 = SIGILL;
static int dummy9 = SIGINT;
static int dummy10 = SIGKILL;
static int dummy11 = SIGPIPE;
static int dummy12 = SIGQUIT;
static int dummy13 = SIGSEGV;
static int dummy14 = SIGSTOP;
static int dummy15 = SIGTERM;
static int dummy16 = SIGTSTP;
static int dummy17 = SIGTTIN;
static int dummy18 = SIGTTOU;
static int dummy19 = SIGUSR1;
static int dummy20 = SIGUSR2;
#ifdef SIGPOLL
static int dummy21 = SIGPOLL;
#endif
#ifdef SIGPROF
static int dummy22 = SIGPROF;
#endif
static int dummy23 = SIGSYS;
static int dummy24 = SIGTRAP;
static int dummy25 = SIGURG;
static int dummy26 = SIGVTALRM;
static int dummy27 = SIGXCPU;
static int dummy28 = SIGXFSZ;
