package URI::URL::telnet;
require URI::URL::_login;
@ISA = qw(URI::URL::_login);

sub default_port { 23 }

1;
