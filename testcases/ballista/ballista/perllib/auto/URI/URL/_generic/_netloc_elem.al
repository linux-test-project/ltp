# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

sub _netloc_elem {
    my($self, $elem, @val) = @_;
    my $old = $self->_elem($elem, @val);
    return $old unless @val;

    # update the 'netloc' element
    my $nl = '';
    my $host = $self->{'host'};
    if (defined $host) {  # can't be any netloc without any host
	my $user = $self->{'user'};
	$nl .= uri_escape($user, $URI::URL::reserved) if defined $user;
	$nl .= ":" . uri_escape($self->{'password'}, $URI::URL::reserved)
	  if defined($user) and defined($self->{'password'});
	$nl .= '@' if length $nl;
	$nl .= uri_escape($host, $URI::URL::reserved);
	my $port = $self->{'port'};
	$nl .= ":$port" if defined($port) && $port != $self->default_port;
    }
    $self->{'netloc'} = $nl;
    $self->{'_str'} = '';
    $old;
}

1;
