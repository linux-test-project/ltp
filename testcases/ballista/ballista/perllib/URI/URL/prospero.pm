package URI::URL::prospero;
require URI::URL::_generic;
@ISA = qw(URI::URL::_generic);

# RFC 1738 says:
#
#   A prospero URLs takes the form:
#
#      prospero://<host>:<port>/<hsoname>;<field>=<value>
#
#   where <host> and <port> are as described in Section 3.1. If :<port>
#   is omitted, the port defaults to 1525. No username or password is
#   allowed.
#
#   The <hsoname> is the host-specific object name in the Prospero
#   protocol, suitably encoded.  This name is opaque and interpreted by
#   the Prospero server.  The semicolon ";" is reserved and may not
#   appear without quoting in the <hsoname>.

sub default_port { 1525 }       # says rfc1738, section 3.11

sub _parse {
    my($self, $init) = @_;
    $self->URI::URL::_generic::_parse($init, qw(netloc path params frag));
}

1;
