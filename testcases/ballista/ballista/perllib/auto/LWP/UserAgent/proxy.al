# NOTE: Derived from ./blib/lib/LWP/UserAgent.pm.  Changes made here will be lost.
package LWP::UserAgent;

sub proxy
{
    my($self, $key, $proxy) = @_;

    LWP::Debug::trace("$key, $proxy");

    if (!ref($key)) {   # single scalar passed
	my $old = $self->{'proxy'}{$key};
	$self->{'proxy'}{$key} = $proxy;
	return $old;
    } elsif (ref($key) eq 'ARRAY') {
	for(@$key) {    # array passed
	    $self->{'proxy'}{$_} = $proxy;
	}
    }
    return undef;
}

1;
