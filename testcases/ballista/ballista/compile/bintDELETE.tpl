name int bint;

parent paramAccess;

includes
[
 {
  #include "values.h"  //for digital unix
  #include "bTypes.h"
 }
]

global_defines
[
 {
  #define filename "btestfile"
 }
]

dials
[
  enum_dial HVAL : ONE, ZERO, NEG1, MAX, MIN, TWO, FOUR, EIGHT, SIXTEEN, THIRTYTWO, SIXTYFOUR, SEVENTEEN, FIFTEEN, THIRTYTHREE, SIXTYFIVE, ONE27, ONE29, TWO55, TWO57, I65535, I65537, INEG64K;
  int_dial  ANYINT;
]

access
[
  INEG64K
    {
       _theVariable=-65536;
    }
  I65537
    {
       _theVariable=65537;
    }
  I65535
    {
       _theVariable=65535;
    }
  TWO57
    {
       _theVariable=257;
    }
  TWO55
    {
       _theVariable=255;
    }
  ONE29
    {
       _theVariable=129;
    }
  ONE27
    {
       _theVariable=127;
    }
  ONE
    {
       _theVariable=1;
    }
 ZERO
    {
       _theVariable=0;
    }
 NEG1
    {
       _theVariable=-1;
    }
 MAX
    {
       _theVariable=MAXINT;
    }
 MIN
    {
       _theVariable=-MAXINT;
    }
 TWO
    {
       _theVariable=2;
    }
 FOUR
    {
       _theVariable=4;
    }
 EIGHT
    {
       _theVariable=8;
    }
 SIXTEEN
    {
       _theVariable=16;
    }
 THIRTYTWO
    {
       _theVariable=32;
    }
 SIXTYFOUR
    {
       _theVariable=64;
    }
 SEVENTEEN
    {
       _theVariable=17;
    }
 FIFTEEN
    {
       _theVariable=15;
    }
 THIRTYTHREE
    {
       _theVariable=33;
    }
 SIXTYFIVE
    {
       _theVariable=65;
    }
  ANYINT
    {
      _theVariable = _ANYINT_INT;
    }
]

commit
[
]

cleanup
[
]
