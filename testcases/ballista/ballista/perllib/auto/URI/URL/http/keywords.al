# NOTE: Derived from ./blib/lib/URI/URL/http.pm.  Changes made here will be lost.
package URI::URL::http;

# Handle ...?dog+bones type of query
sub keywords {
    my $self = shift;
    $old = $self->{'query'};
    if (@_) {
	# Try to set query string
	$self->equery(join('+', map { URI::Escape::uri_escape($_, $URI::URL::reserved) } @_));
    }
    return if !defined($old) || !defined(wantarray);

    Carp::croak("Query is not keywords") if $old =~ /=/;
    map { URI::Escape::uri_unescape($_) } split(/\+/, $old);
}

1;
