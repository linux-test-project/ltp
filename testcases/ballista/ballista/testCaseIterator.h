/* testCaseIterator.h: class that will iterate through all test cases
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


#ifndef _TEST_CASE_ITERATOR_
#define _TEST_CASE_ITERATOR_


#include "ballistaRPC.h"
#include <vector>
#include "line.h"
#include "serverCommunication.h"

using namespace std;

class Test_case_iterator {
  three_dim  *result_1;
  info get_array_1_arg;
  int total;
  int index;
  double sample_size;
  Server_communication *server;
 public:
  Test_case_iterator(Server_communication *server_com, double sample_size_val, 
		     int NumArg, vector<line> vec[MAXP]);
  two_dim *next();
  virtual ~Test_case_iterator();
 private:

  void send_request_to_server(info *data);
};



#endif // _TEST_CASE_ITERATOR_
