#include <termios.h>  //included from calltable include field
#include <signal.h>
ref[1] = (void **) param1.access(params[0]);
structTermiosPtr *temp1 = (structTermiosPtr*) ref[1];
ref[2] = (void **) param2.access(params[1]);
structTermiosPtr *temp2 = (structTermiosPtr*) ref[2];
param1.commit(params[0][0]);
param2.commit(params[1][0]);

struct sigaction setup_action;
sigemptyset(&setup_action.sa_mask);
setup_action.sa_handler = SIG_DFL;
(void) sigaction (SIGINT, &setup_action, NULL);
(void) sigaction (SIGQUIT, &setup_action, NULL);
(void) sigaction (SIGTERM, &setup_action, NULL);
(void) sigaction (SIGABRT, &setup_action, NULL);
(void) sigaction (SIGBUS, &setup_action, NULL);
(void) sigaction (SIGSEGV, &setup_action, NULL);
rval = cfgetispeed(*temp1 , *temp2);
param1.cleanup(params[0][0]);
param2.cleanup(params[1][0]);
	 //done
cout<<"rval:"<<rval<<endl;

//inlining userCatches



