#!/usr/bin/perl
#

# Test whether the interfaces on 10.53.0.* are up.

require 5.001;

use Socket;
use Getopt::Long;

my $port = 0;
GetOptions("p=i" => \$port);

#print " port is $port";

for ($id = 1 ; $id < 6 ; $id++) {
        my $addr = pack("C4", 10, 53, 0, $id);
	my $sa = pack_sockaddr_in($port, $addr);
	socket(SOCK, PF_INET, SOCK_STREAM, getprotobyname("tcp"))
      		or die "$0: socket: $!\n";
      #  print " addr is $addr, sa is $sa";
	bind(SOCK, $sa) 
        #	 or  die sprintf("$0: bind(%s, %d): $!\n",
	#     		       inet_ntoa($addr), $port) ;
                 or  exit 1;
        
        
        close(SOCK);
	sleep (1);
}
