#include <signal.h>  //included from calltable include field
#include <signal.h>
ref[1] = (void **) param1.access(params[0]);
sigset_t* *temp1 = (sigset_t**) ref[1];
ref[2] = (void **) param2.access(params[1]);
siginfo_t* *temp2 = (siginfo_t**) ref[2];
ref[3] = (void **) param3.access(params[2]);
structtimespecptr *temp3 = (structtimespecptr*) ref[3];
ref[4] = (void **) param4.access(params[3]);
structtimespecptr *temp4 = (structtimespecptr*) ref[4];
param1.commit(params[0][0]);
param2.commit(params[1][0]);
param3.commit(params[2][0]);
param4.commit(params[3][0]);

struct sigaction setup_action;
sigemptyset(&setup_action.sa_mask);
setup_action.sa_handler = SIG_DFL;
(void) sigaction (SIGINT, &setup_action, NULL);
(void) sigaction (SIGQUIT, &setup_action, NULL);
(void) sigaction (SIGTERM, &setup_action, NULL);
(void) sigaction (SIGABRT, &setup_action, NULL);
(void) sigaction (SIGBUS, &setup_action, NULL);
(void) sigaction (SIGSEGV, &setup_action, NULL);
rval = sigtimedwait(*temp1 , *temp2 , *temp3 , *temp4);
param1.cleanup(params[0][0]);
param2.cleanup(params[1][0]);
param3.cleanup(params[2][0]);
param4.cleanup(params[3][0]);
	 //done
cout<<"rval:"<<rval<<endl;

//inlining userCatches



