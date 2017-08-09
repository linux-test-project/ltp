#include <signal.h>

int main(void)
{
	raise(SIGSEGV);
	return 0;
}
