# NOTE: Derived from ./blib/lib/LWP/UserAgent.pm.  Changes made here will be lost.
package LWP::UserAgent;

sub is_protocol_supported
{
    my($self, $scheme) = @_;
    if (ref $scheme) {
	# assume we got a reference to an URI::URL object
	$scheme = $scheme->abs->scheme;
    } else {
	Carp::croak("Illeal scheme '$scheme' passed to is_protocol_supported")
	    if $scheme =~ /\W/;
	$scheme = lc $scheme;
    }
    return LWP::Protocol::implementor($scheme);
}

1;
