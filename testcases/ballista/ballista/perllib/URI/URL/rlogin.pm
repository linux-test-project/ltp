package URI::URL::rlogin;
require URI::URL::_login;
@ISA = qw(URI::URL::_login);

sub default_port { 513 }

1;
