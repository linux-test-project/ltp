/* serverCommunication.cpp: This file contains the Server_communication class.
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

/************************
 *
 * Description: This file contains the Server_communication class.  
 *              It abstracts the RPC calls to the server away from the 
 *              rest of the program
 *
 ************************/

#include <assert.h>

#include "serverCommunication.h"
#include "ballistaRPC.h"
#include "ballistaUtil.h"

using namespace std;

#ifdef HP
extern "C" CLIENT * clnt_create(char *, u_long, u_long, char*);
extern "C" void clnt_pcreateerror(char *);
extern "C" void clnt_perror(CLIENT *, char *);
void CLNT_DESTROY2(CLIENT *rh){
  //  ((*(rh)->cl_ops->cl_destroy)(rh));
  rh->cl_ops->cl_destroy();
}
#endif


/************************
 *
 * Function: Server_communcation constructor
 * Description: make a copy of the name and create the client.
 * Function Inputs: server_name
 * Global Inputs:
 * Return Values:
 * Global Outputs:
 * Errors:  If the client cannot be created, the program will throw an 
            exception
 * Pre-Conditions:
 * Post-Conditions: clnt is initialized
 * Design:
 * Notes:
 *
 ************************/
#include <iostream>
#include <netdb.h>
#include <fstream>
#include <signal.h>
#include <string.h>

#ifdef SUN

#include <strings.h> 

#endif

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>

//new socket addressing variables for firewalls
  struct   sockaddr_in server_addr, client_addr;
  int sock;
  int Stupid;
  
  struct hostent *hp;

Server_communication::Server_communication(const char *server_name)     
  throw(Ballistic_error)
{
  assert(server_name != NULL);

  safe_strncpy(name,server_name,sizeof(name));
  
  Stupid=1;

/*    
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons(4255);
    sock= socket (PF_INET,SOCK_STREAM,0);
    
    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR, &Stupid,sizeof(int)))
    {
    	perror("Failed to set sock option : ");
    	exit (1);
    }
  
    if (bind(sock,(struct sockaddr *)&client_addr,sizeof(client_addr))==-1)
    {
    	perror ("Error binding socket : ");
    	exit(1);
    }
*/
    
    hp = gethostbyname(name);
    if (hp==NULL)
      {
        cerr<<"Error: could not locate host "<<server_name<<".\n";
        assert(0);
      }


    memcpy((char *)&(server_addr.sin_addr.s_addr),(const char *)hp->h_addr,hp->h_length);


    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons (4255);
     
    
    //if (connect(sock,(struct sockaddr *)&server_addr,sizeof(client_addr))==-1)
    //{
    //	throw Ballistic_error ("Error connecting socket : ");
    //}
 
 //   close(sock);
    sock = RPC_ANYSOCK;
    clnt = clnttcp_create(&server_addr,BALLISTA, BALLISTAVERS,&sock,0,0);
    if (clnt == NULL) {
      clnt_pcreateerror(name);
      throw Ballistic_error("Failed to create TCP socket for client\n");
     }

  
  
  //Try and create a connection to the host, we are using tcp here
  //because it can support larger datatypes and is much more reliable
  //then udp, plus it segfaults if you even try to use udp

/*   Old client create code
  clnt = clnt_create(name, BALLISTA, BALLISTAVERS, "tcp");
  if (clnt == NULL) {
    clnt_pcreateerror(name);
    throw Ballistic_error("Error: Cannot initiate RPC with server.");
  }
  */
}


/************************
 *
 * Function: get_name
 * Description: copies the name of the server into server_name
 * Function Inputs: server_name, max_server_name
 * Global Inputs:
 * Return Values: server_name
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Conditions: name of the server is copied into server_name
 * Design:
 * Notes:
 *
 ************************/
void Server_communication::get_name(char *server_name,int max_server_name)
{
  assert(server_name != NULL);

  safe_strncpy(server_name,name,max_server_name);
}


/************************
 *
 * Function: get_test_set
 * Description: this function gets the test set from the server
 * Function Inputs: param_file_data
 * Global Inputs:
 * Return Values:  return_info
 * Global Outputs:
 * Errors: If the RPC call failed, the method will throw an exception
 * Pre-Conditions:
 * Post-Contidions:
 * Design: call the RPC call.  If it fails, exit the program.
 * Notes:
 *
 ************************/
return_info *Server_communication::get_test_set(info *param_file_data)
  throw(Ballistic_error)
{
  assert(param_file_data != NULL);

  return_info *result_temp = get_array_3(param_file_data, clnt);
  if (result_temp == NULL) {
    clnt_perror(clnt, "call failed:");
    throw(Ballistic_error("Error: Cannot get test set from server."));
  }

#ifdef UNIT_TEST
  FILE *fptr = fopen("server.txt","a");
  if(fptr) {
    fprintf(fptr,"received tests from %s\n",name);
    fprintf(fptr,"total = %d\n",result_temp->total);
    int i,j,k;
    two_dim *td;
    for(i=0;i<result_temp->total;i++) {
      td = &result_temp->my_three_dim.three_dim_val[i];
      for(j=0;j<td->two_dim_len;j++) {
      for(k=0;k<td->two_dim_val[j].one_dim_len;k++) {
	fprintf(fptr,"%s\t",td->two_dim_val[j].one_dim_val[k]);
      }
      fprintf(fptr,"\n");
      }
      fprintf(fptr,"---\n");
    }
    fclose(fptr);
  }
#endif
  return result_temp;
}

/************************
 *
 * Function: send_test_results
 * Description: send the results of a single test to the server
 * Function Inputs: data_from_client representing the results of the last test
 * Global Inputs:
 * Return Values: none
 * Global Outputs:
 * Errors: If the RPC call failed, the program will throw an exception
 * Pre-Conditions:
 * Post-Contidions: server has received the data
 * Design: call the RPC call.  If it fails, throw an exception
 * Notes:
 *
 ************************/
void Server_communication::send_test_results(struct data_from_client *data)
  throw(Ballistic_error)
{
  int *result;

  assert(data != NULL);

  result = send_data_3(data,clnt);
  if (result == NULL) {
    clnt_perror(clnt, "call failed:");
    throw(Ballistic_error("Error: Cannot pass test results to server."));
  }
}


/************************
 *
 * Function: ~Server_communication
 * Description: destructor for the class
 * Function Inputs:
 * Global Inputs:
 * Return Values:
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Contidions: client is destroyed
 * Design: destroy the client
 * Notes:
 *
 ************************/
Server_communication::~Server_communication()
{
#ifdef  HP
  CLNT_DESTROY2(clnt);
#else
  clnt_destroy( clnt );
#endif
  
}
