// b_mode_t.tpl : Ballista Datatype Template for mode_t datatype
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

name mode_t b_mode_t;

parent b_int;

includes
[
{
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <sys/types.h>
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
  // Note removed U_RW, U_RX, U_WX, G_RW, G_RX, G_WX, O_RW, O_RX, O_WX, G_RWX, O_RWX to short
  // run time
  enum_dial OWNER : U_READ, U_WRITE, U_EXEC, U_RWX, U_NONE;
  enum_dial GROUP : G_READ, G_WRITE, G_EXEC, G_NONE;
  enum_dial OTHER : O_READ, O_WRITE, O_EXEC, O_NONE;
  enum_dial S_ISUID : UID_SET, UID_CLEAR;
  enum_dial S_ISGID : GID_SET, GID_CLEAR;
]

access
[
{
_theVariable = 0;
}
	U_READ
	{//owner read permission
	_theVariable = (_theVariable | S_IRUSR);
	}
	U_WRITE
	{//owner write permission
	_theVariable = (_theVariable | S_IWUSR);
	}
	U_EXEC
	{//owner exec permission
	_theVariable = (_theVariable | S_IXUSR);
	}
//	U_RW
//	{//owner r/w permission
//	_theVariable = (_theVariable | S_IRUSR | S_IWUSR);
//	}
//	U_RX
//	{//owner r/x permission
//	_theVariable = (_theVariable | S_IRUSR | S_IXUSR);
//	}
//	U_WX
//	{//owner x/w permission
//	_theVariable = (_theVariable | S_IXUSR | S_IWUSR);
//	}
	U_RWX
	{//owner r/w/x permission
	_theVariable = (_theVariable | S_IRUSR | S_IWUSR | S_IXUSR);
	}


	G_READ
	{//owner read permission
	_theVariable = (_theVariable | S_IRGRP);
	}
	G_WRITE
	{//owner write permission
	_theVariable = (_theVariable | S_IWGRP);
	}
	G_EXEC
	{//owner exec permission
	_theVariable = (_theVariable | S_IXGRP);
	}
//	G_RW
//	{//owner r/w permission
//	_theVariable = (_theVariable | S_IRGRP | S_IWGRP);
//	}
//	G_RX
//	{//owner r/x permission
//	_theVariable = (_theVariable | S_IRGRP | S_IXGRP);
//	}
//	G_WX
//	{//owner x/w permission
//	_theVariable = (_theVariable | S_IXGRP | S_IWGRP);
//	}
//	G_RWX
//	{//owner r/w/x permission
//	_theVariable = (_theVariable | S_IRGRP | S_IWGRP | S_IXGRP);
//	}

	O_READ
	{//owner read permission
	_theVariable = (_theVariable | S_IROTH);
	}
	O_WRITE
	{//owner write permission
	_theVariable = (_theVariable | S_IWOTH);
	}
	O_EXEC
	{//owner exec permission
	_theVariable = (_theVariable | S_IXOTH);
	}
//	O_RW
//	{//owner r/w permission
//	_theVariable = (_theVariable | S_IROTH | S_IWOTH);
//	}
//	O_RX
//	{//owner r/x permission
//	_theVariable = (_theVariable | S_IROTH | S_IXOTH);
//	}
//	O_WX
//	{//owner x/w permission
//	_theVariable = (_theVariable | S_IXOTH | S_IWOTH);
//	}
//	O_RWX
//	{//owner r/w/x permission
//	_theVariable = (_theVariable | S_IROTH | S_IWOTH | S_IXOTH);
//	}

	UID_SET
	{
	_theVariable = (_theVariable | S_ISUID);
	}
	GID_SET
	{
	_theVariable = (_theVariable | S_ISGID);
	}
]

commit
[
]

cleanup
[
]
