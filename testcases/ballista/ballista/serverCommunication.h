/* serverCommunication.h: This file contains the Server_communication class.
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

#ifndef _SERVER_COMMUNICATION_H_
#define _SERVER_COMMUNICATION_H_

#include "ballistaRPC.h"
#include "ballistaError.h"


class Server_communication 
{
private:
  CLIENT *clnt;
  char name[255];

public:
  Server_communication(const char *server_name) 
    throw(Ballistic_error);
  virtual ~Server_communication();
  void get_name(char *server_name, int max_server_name);
  void send_test_results(struct data_from_client *data) 
    throw(Ballistic_error);
  return_info *get_test_set(info *param_file_data)
    throw(Ballistic_error);
};


#endif // _SERVER_COMMUNICATION_H_
