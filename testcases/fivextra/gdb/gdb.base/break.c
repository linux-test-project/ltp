#ifdef vxworks

#  include <stdio.h>

/* VxWorks does not supply atoi.  */
static int
atoi (z)
     char *z;
{
  int i = 0;

  while (*z >= '0' && *z <= '9')
    i = i * 10 + (*z++ - '0');
  return i;
}

/* I don't know of any way to pass an array to VxWorks.  This function
   can be called directly from gdb.  */

vxmain (arg)
char *arg;
{
  char *argv[2];

  argv[0] = "";
  argv[1] = arg;
  main (2, argv, (char **) 0);
}

#else /* ! vxworks */
#  include <stdio.h>
#  include <stdlib.h>
#endif /* ! vxworks */

/*
 * The following functions do nothing useful.  They are included simply
 * as places to try setting breakpoints at.  They are explicitly
 * "one-line functions" to verify that this case works (some versions
 * of gcc have or have had problems with this).
 */

#ifdef PROTOTYPES
int marker1 (void) { return (0); }
int marker2 (int a) { return (1); }
void marker3 (char *a, char *b) {}
void marker4 (long d) {}
#else
int marker1 () { return (0); }
int marker2 (a) int a; { return (1); }
void marker3 (a, b) char *a, *b; {}
void marker4 (d) long d; {}
#endif

/*
 *	This simple classical example of recursion is useful for
 *	testing stack backtraces and such.
 */

#ifdef PROTOTYPES
int factorial(int);

int
main (int argc, char **argv, char **envp)
#else
int
main (argc, argv, envp)
int argc;
char *argv[], **envp;
#endif
{
#ifdef usestubs
    set_debug_traps();
    breakpoint();
#endif
    if (argc == 12345) {  /* an unlikely value < 2^16, in case uninited */
	fprintf (stderr, "usage:  factorial <number>\n");
	return 1;
    }
    printf ("%d\n", factorial (atoi ("6")));

    marker1 ();
    marker2 (43);
    marker3 ("stack", "trace");
    marker4 (177601976L);
    argc = (argc == 12345); /* This is silly, but we can step off of it */
    return argc;
}

#ifdef PROTOTYPES
int factorial (int value)
#else
int factorial (value)
int value;
#endif
{
    if (value > 1) {
	value *= factorial (value - 1);
    }
    return (value);
}

#ifdef PROTOTYPES
int multi_line_if_conditional (int a, int b, int c)
#else
int multi_line_if_conditional (a, b, c)
  int a, b, c;
#endif
{
  if (a
      && b
      && c)
    return 0;
  else
    return 1;
}

#ifdef PROTOTYPES
int multi_line_while_conditional (int a, int b, int c)
#else
int multi_line_while_conditional (a, b, c)
  int a, b, c;
#endif
{
  while (a
      && b
      && c)
    {
      a--, b--, c--;
    }
  return 0;
}
