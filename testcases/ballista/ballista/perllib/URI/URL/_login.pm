package URI::URL::_login;
require URI::URL::_generic;
@ISA = qw(URI::URL::_generic);


# Generic terminal logins.  This is used as a base class for 'telnet',
# 'tn3270', and 'rlogin' URL schemes.


sub _parse {
    my($self, $init) = @_;
    # All we want from _generic is the 'netloc' handling.
    $self->URI::URL::_generic::_parse($init, 'netloc');
}


*path      = \&URI::URL::bad_method;
*epath     = \&URI::URL::bad_method;
*query     = \&URI::URL::bad_method;
*equery    = \&URI::URL::bad_method;
*params    = \&URI::URL::bad_method;
*eparams   = \&URI::URL::bad_method;
*frag      = \&URI::URL::bad_method;

1;
