// b_oflag.tpl : Ballista Datatype Template for open file flags
// Copyright (C) 1998-2001  Carnegie Mellon University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

name int b_oflag;

parent b_int;

includes
[
{
#include <stdio.h>
#include <fcntl.h>
#include "b_int.h"

}
]

global_defines
[
{ 

}
]

dials
[
enum_dial ACCESS : RDONLY_SET, WRONLY_SET, RDWR_SET, NONE;
enum_dial O_APPEND : A_SET, A_CLEAR;
enum_dial O_CREAT :  C_SET, C_CLEAR;
enum_dial O_DSYNC :  D_SET, D_CLEAR;
enum_dial O_EXCL   : E_SET, E_CLEAR;
enum_dial O_NOCTTY : NCT_SET, NCT_CLEAR;
enum_dial O_NONBLOCK : NBLK_SET, NBLK_CLEAR;
enum_dial O_RSYNC    : R_SET, R_CLEAR;
enum_dial O_SYNC     : SYNC_SET, SYNC_CLEAR;
enum_dial O_TRUNC  : T_SET, T_CLEAR;
]

access
[
  {_theVariable = 0;} 
	RDONLY_SET {_theVariable |=  O_RDONLY;   }
	WRONLY_SET {_theVariable |=  O_WRONLY;   }
	RDWR_SET   {_theVariable |=  O_RDWR;     }
	A_SET      {_theVariable |=  O_APPEND;   }
	C_SET      {_theVariable |=  O_CREAT;    }
	D_SET      {_theVariable |=  O_DSYNC;    }
	E_SET      {_theVariable |=  O_EXCL;     }
	NCT_SET    {_theVariable |=  O_NOCTTY;   }
	NBLK_SET   {_theVariable |=  O_NONBLOCK; }
	R_SET      {_theVariable |=  O_RSYNC;    }
	SYNC_SET   {_theVariable |=  O_SYNC;     }
	T_SET      {_theVariable |=  O_TRUNC;    }
]

commit
[
{
}
]

cleanup
[
{
}

]
