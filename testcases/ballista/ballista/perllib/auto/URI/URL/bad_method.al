# NOTE: Derived from ./blib/lib/URI/URL.pm.  Changes made here will be lost.
package URI::URL;

# This is set up as an alias for various methods
sub bad_method {
    my $self = shift;
    my $scheme = $self->scheme;
    Carp::croak("Illegal method called for $scheme: URL")
	if $Strict_URL;
    # Carp::carp("Illegal method called for $scheme: URL")
    #     if $^W;
    undef;
}

1;
