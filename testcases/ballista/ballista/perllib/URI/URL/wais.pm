package URI::URL::wais;
require URI::URL::_generic;
@ISA = qw(URI::URL::_generic);

use URI::Escape;

# RFC 1738 says:
#
#   A WAIS URL takes one of the following forms:
#
#     wais://<host>:<port>/<database>
#     wais://<host>:<port>/<database>?<search>
#     wais://<host>:<port>/<database>/<wtype>/<wpath>
#
#   where <host> and <port> are as described in Section 3.1. If :<port>
#   is omitted, the port defaults to 210.  The first form designates a
#   WAIS database that is available for searching. The second form
#   designates a particular search.  <database> is the name of the WAIS
#   database being queried.


sub default_port { 210 }

sub _parse {
    my($self, $init) = @_;
    $self->URI::URL::_generic::_parse($init, qw(netloc path query frag));
}

# Set the path component with the specified number
sub _path_comp
{
    my $self = shift;
    my $no   = shift;
    my @p = $self->path_components;
    shift(@p) if @p && $p[0] eq '';
    my $old = $p[$no];
    if (@_) {
	$p[$no] = $_[0];
	$self->path_components(@p);
    }
    $old;
}

sub database { shift->_path_comp(0, @_); }
sub wtype    { shift->_path_comp(1, @_); }
sub wpath    { shift->_path_comp(2, @_); }

*params   = \&URI::URL::bad_method;
*eparams  = \&URI::URL::bad_method;
1;
