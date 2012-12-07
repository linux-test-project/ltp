  /*
     Test the definition of siginfo_t.
   */

#include <signal.h>
#include <sys/types.h>

siginfo_t this_type_should_exist, t;
int tsigno;
int terrno;
int tcode;
pid_t tpid;
uid_t tuid;
void *taddr;
int tstatus;
long tband;
union sigval tvalue;

int dummyfcn(void)
{
	tsigno = t.si_signo;
	terrno = t.si_errno;
	tcode = t.si_code;
	tpid = t.si_pid;
	tuid = t.si_uid;
	taddr = t.si_addr;
	tstatus = t.si_status;
	tband = t.si_band;
	tvalue = t.si_value;

	return 0;
}
