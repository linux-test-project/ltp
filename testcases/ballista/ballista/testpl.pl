#!/usr/bin/perl


my $option1default = "n"; # default answer for question 1
my $option1; # the final answer for question1
my $timeoutdefault = "5"; # seconds to wait
 
  
# start
$SIG{ALRM} = sub { die "timeout" };
 print "Enter \n";
 eval {

 alarm( $timeoutdefault );

# get the input

 $option1 = <STDIN>;

alarm( 0 );

 };

 if ( $@ ) {

 if ( $@ =~ /timeout/i ) {

 $option1 = $option1default;
 print "\n";

 } else {

 alarm( 0 );
 die;

 }

 }

 chomp( $option1 );
 print "Answer to question 1: $option1\n";

 exit;
