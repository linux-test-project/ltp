// b_ptr_termios.tpl : Ballista Datatype Template for termios pointer
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

name structTermiosPtr b_ptr_termios;

parent b_ptr_buf;

includes
[
{
#include <termios.h>
#include "b_ptr_buf.h"
#define structTermiosPtr struct termios*
}
]

global_defines
[
{
static struct termios termios_temp;
}
]

dials
[
  enum_dial C_IFLAG : BRKINT_SET, PARMRK_SET, I_MAXINT,I_NEGONE, I_ONE, I_ZERO;
  enum_dial C_OFLAG : OPOST_SET, O_MAXINT, O_NEGONE, O_ONE, O_ZERO;
  enum_dial C_CFLAG : CLOCAL_SET, PARENB_SET, PARODD_SET, C_MAXINT,C_NEGONE, C_ONE, C_ZERO;
  enum_dial C_LFLAG : ECHOK_SET, ICANON_SET, IEXTEN_SET, L_MAXINT,L_NEGONE, L_ONE, L_ZERO;
  enum_dial C_CC : CC_UNSET, CC_VKILL, CC_VMIN; 
  enum_dial BAUD    : ZERO, THREE_HUNDRED, THREE8400, NEG1, UNSET;

  // complete list too big for ballista - valid values were stripped
//  enum_dial C_IFLAG : BRKINT_SET, ICRNL_SET, IGNBRK_SET, IGNCR_SET, IGNPAR_SET, INLCR_SET, INPCK_SET, ISTRIP_SET, IXOFF_SET, IXON_SET,PARMRK_SET,
//                      I_MAXINT,I_NEGONE, I_ONE, I_ZERO;
//  enum_dial C_OFLAG : OPOST_SET, O_MAXINT, O_NEGONE, O_ONE, O_ZERO;
//  enum_dial C_CFLAG : CLOCAL_SET, CREAD_SET, CSIZE_SET, CS5_SET, CS6_SET, CS7_SET, CS8_SET, CSTOPB_SET, HUPCL_SET, PARENB_SET, PARODD_SET,
//                      C_MAXINT,C_NEGONE, C_ONE, C_ZERO;
//  enum_dial C_LFLAG : ECHO_SET, ECHOE_SET, ECHOK_SET, ECHONL_SET, ICANON_SET, IEXTEN_SET, ISIG_SET, NOFLSH_SET, TOSTOP_SET, L_MAXINT,L_NEGONE,
//                      L_ONE, L_ZERO;
//  enum_dial C_CC : CC_UNSET, CC_VKILL, CC_VMIN; 
//  enum_dial BAUD    : ZERO, THREE_HUNDRED, THREE8400, NEG1, UNSET;

]

access
[ 
{
termios_temp.c_iflag = 0;
termios_temp.c_oflag = 0;
termios_temp.c_cflag = 0;
termios_temp.c_lflag = 0;
}
	BRKINT_SET	{termios_temp.c_iflag |=  BRKINT; }
//	ICRNL_SET	{termios_temp.c_iflag |=  ICRNL; }
//	IGNBRK_SET	{termios_temp.c_iflag |= IGNBRK; }
//	IGNCR_SET	{termios_temp.c_iflag |= IGNCR; }
//	IGNPAR_SET	{termios_temp.c_iflag |= IGNPAR; }
//	INLCR_SET	{termios_temp.c_iflag |= INLCR; }
//	INPCK_SET	{termios_temp.c_iflag |= INPCK; }
//	ISTRIP_SET	{termios_temp.c_iflag |= ISTRIP; }
//	IXOFF_SET	{termios_temp.c_iflag |=  IXOFF; }
//	IXON_SET	{termios_temp.c_iflag |=  IXON;	}
	PARMRK_SET	{termios_temp.c_iflag |=  PARMRK; }
        I_MAXINT	{termios_temp.c_iflag == MAXINT; }
        I_ZERO		{termios_temp.c_iflag == 0; }
        I_ONE		{termios_temp.c_iflag == 1; }
        I_NEGONE	{termios_temp.c_iflag == -1; }

	OPOST_SET	{termios_temp.c_oflag |= OPOST;	}
        O_MAXINT	{termios_temp.c_oflag == MAXINT; }
        O_ZERO		{termios_temp.c_oflag == 0; }
        O_ONE		{termios_temp.c_oflag == 1; }
        O_NEGONE	{termios_temp.c_oflag == -1; }

	CLOCAL_SET	{termios_temp.c_cflag |= CLOCAL; }
//	CREAD_SET	{termios_temp.c_cflag |=CREAD ;	}
//	CSIZE_SET	{termios_temp.c_cflag |= CSIZE;	}
//	CS5_SET	{termios_temp.c_cflag |= CS5; }
//	CS6_SET	{termios_temp.c_cflag |= CS6; }
//	CS7_SET	{termios_temp.c_cflag |=CS7 ; }
//	CS8_SET	{termios_temp.c_cflag |= CS8; }
//	CSTOPB_SET	{termios_temp.c_cflag |= CSTOPB; }
//	HUPCL_SET	{termios_temp.c_cflag |= HUPCL;	}
	PARENB_SET	{termios_temp.c_cflag |= PARENB; }
	PARODD_SET	{termios_temp.c_cflag |= PARODD; }
        C_MAXINT	{termios_temp.c_cflag == MAXINT; }
        C_ZERO		{termios_temp.c_cflag == 0; }
        C_ONE		{termios_temp.c_cflag == 1; }
        C_NEGONE	{termios_temp.c_cflag == -1; }

//	ECHO_SET	{termios_temp.c_lflag |= ECHO;	}
//	ECHOE_SET	{termios_temp.c_lflag |= ECHOE;	}
	ECHOK_SET	{termios_temp.c_lflag |= ECHOK;	}
//	ECHONL_SET	{termios_temp.c_lflag |= ECHONL; }
	ICANON_SET	{termios_temp.c_lflag |= ICANON; }
	IEXTEN_SET	{termios_temp.c_lflag |= IEXTEN; }
//	ISIG_SET	{termios_temp.c_lflag |= ISIG; }
//	NOFLSH_SET	{termios_temp.c_lflag |= NOFLSH; }
//	TOSTOP_SET	{termios_temp.c_lflag |= TOSTOP; }
        L_MAXINT	{termios_temp.c_lflag == MAXINT; }
        L_ZERO		{termios_temp.c_lflag == 0; }
        L_ONE		{termios_temp.c_lflag == 1; }
        L_NEGONE	{termios_temp.c_lflag == -1; }
	CC_VKILL	{termios_temp.c_cc[0] = VKILL; }
	CC_VMIN		{termios_temp.c_cc[0] = VMIN; } 

	ZERO	
	{if ((cfsetospeed(&termios_temp, 0))!=0){
		perror("set BAUD to 0 failed");
		exit(99);
		}
	}
	THREE_HUNDRED
	{
	if ((cfsetospeed(&termios_temp, 300))!=0){
		perror("set BAUD to 300 failed");
		exit(99);
		}
	}
	THREE8400
	{
	if ((cfsetospeed(&termios_temp, 38400))!=0){
		perror("set BAUD to 38400 failed");
		exit(99);
		}
	}
	NEG1
	{
	if ((cfsetospeed(&termios_temp, -1))!=0){
		perror("set BAUD to -1 failed");
		exit(99);
		}
	}
{
  _theVariable = &termios_temp;
}
]


commit
[
]

cleanup
[
]
