/* testManager.h: Class definitions for virtual class Test_manager
   Copyright (C) 1998-2001  Carnegie Mellon University

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _TESTMANAGER_H
#define _TESTMANAGER_H

#include "ballista.h"


/************************
 *
 * Function:        class Test_manager
 * Description:     Test_manager class definition - base class for all
 *                  Test_manager classes for different Ballista client
 *                  implementations
 * Function Inputs: N/A
 * Global Inputs:   N/A
 * Return Values:   N/A
 * Global Outputs:  N/A
 * Errors:          N/A
 * Pre-Conditions:  N/A
 * Post-Contidions: N/A
 * Design:          N/A
 * Notes:           N/A
 *
 ************************/

class Test_manager {
    public:

    virtual int manage_test(MARSHAL_DATA_TYPE arguments,
			    long timeout_value,
			    char *pass_status, int max_pass_status,
			    int *call_ret,
			    char *sys_err, int max_sys_err,
			    char *mut_return,
			    int max_mut_return) = 0;

};

#endif // _TESTMANAGER_H
