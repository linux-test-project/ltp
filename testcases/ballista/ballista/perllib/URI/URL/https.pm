package URI::URL::https;
require URI::URL::http;
@ISA = qw(URI::URL::http);

sub default_port { 443 }
1;
