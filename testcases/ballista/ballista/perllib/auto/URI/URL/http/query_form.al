# NOTE: Derived from ./blib/lib/URI/URL/http.pm.  Changes made here will be lost.
package URI::URL::http;

# Handle ...?foo=bar&bar=foo type of query
sub query_form {
    my $self = shift;
    $old = $self->{'query'};
    if (@_) {
	# Try to set query string
	my @query;
	my($key,$vals);
        my $esc = $URI::URL::reserved . $URI::URL::unsafe;
	while (($key,$vals) = splice(@_, 0, 2)) {
	    $key = '' unless defined $key;
	    $key =  URI::Escape::uri_escape($key, $esc);
	    $vals = [$vals] unless ref($vals) eq 'ARRAY';
	    my $val;
	    for $val (@$vals) {
		$val = '' unless defined $val;
		$val = URI::Escape::uri_escape($val, $esc);
		push(@query, "$key=$val");
	    }
	}
	$self->equery(join('&', @query));
    }
    return if !defined($old) || length($old) == 0 || !defined(wantarray);
    Carp::croak("Query is not a form") unless $old =~ /=/;
    map { s/\+/ /g; URI::Escape::uri_unescape($_) }
	 map { /=/ ? split(/=/, $_, 2) : ($_ => '')} split(/&/, $old);
}

1;
1;
