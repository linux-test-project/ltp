# NOTE: Derived from ./blib/lib/LWP/UserAgent.pm.  Changes made here will be lost.
package LWP::UserAgent;

# Private method which returns the URL of the Proxy configured for this
# URL, or undefined if none is configured.
sub _need_proxy
{
    my($self, $url) = @_;

    $url = new URI::URL($url) unless ref $url;

    LWP::Debug::trace("($url)");

    # check the list of noproxies

    if (@{ $self->{'no_proxy'} }) {
	my $host = $url->host;
	return undef unless defined $host;
	my $domain;
	for $domain (@{ $self->{'no_proxy'} }) {
	    if ($host =~ /$domain$/) {
		LWP::Debug::trace("no_proxy configured");
		return undef;
	    }
	}
    }

    # Currently configured per scheme.
    # Eventually want finer granularity

    my $scheme = $url->scheme;
    if (exists $self->{'proxy'}{$scheme}) {

	LWP::Debug::debug('Proxied');
	return new URI::URL($self->{'proxy'}{$scheme});
    }

    LWP::Debug::debug('Not proxied');
    undef;
}

1;

1;
