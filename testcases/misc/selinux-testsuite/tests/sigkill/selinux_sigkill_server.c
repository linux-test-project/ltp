#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void handler(int sig)
{
	return;
}

int main(void) 
{
  struct sigaction sa;
  int i;

  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  for (i = 0; i < 32; i++) {
	  sigaction(i, &sa, NULL);
  }
	
  while (1)
	  ;
}
