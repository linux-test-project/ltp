#include <stdio.h>
#include <utmp.h>

const char * utTypeName(short int foo) {
    switch (foo) {
      case EMPTY:
	return "EMPTY";
      case RUN_LVL:
	return "RUN_LVL";
      case BOOT_TIME:
	return "BOOT_TIME";
      case NEW_TIME:
	return "NEW_TIME";
      case OLD_TIME:
	return "OLD_TIME";
      case INIT_PROCESS:
	return "INIT_PROCESS";
      case LOGIN_PROCESS:
	return "LOGIN_PROCESS";
      case USER_PROCESS:
	return "USER_PROCESS";
      case DEAD_PROCESS:
	return "DEAD_PROCESS";
      case ACCOUNTING:
	return "ACCOUNTING";
    }

    return "(unknown)";
}

int main() {
    struct utmp * ut;

    setutent();
    while ((ut = getutent())) {
	if (ut->ut_type == DEAD_PROCESS) continue;

	printf("%-13s %5d /dev/%-5s %-8s %-15s\n", 
		utTypeName(ut->ut_type), ut->ut_pid, ut->ut_line,
		*ut->ut_user ? ut->ut_user : "(user)", ut->ut_host);
    }

    return 0;
}
