  /*
     Test the definition of siginfo_t.
   */

#include <signal.h>
#include <sys/types.h>

static siginfo_t this_type_should_exist, t;
static int tsigno;
static int terrno;
static int tcode;
static pid_t tpid;
static uid_t tuid;
static void *taddr;
static int tstatus;
static long tband;
static union sigval tvalue;

static int dummyfcn(void)
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
