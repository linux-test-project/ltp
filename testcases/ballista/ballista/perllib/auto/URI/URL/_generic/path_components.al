# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

sub path_components {
    my $self = shift;
    my $old = $self->{'path'};
    $old = '' unless defined $old;
    $old = "/$old" if $old !~ m|^/| && defined $self->{'netloc'};
    if (@_) {
	$self->_elem('path',
		     join("/", map {uri_escape($_, $URI::URL::reserved)} @_));
    }
    map { uri_unescape($_) } split("/", $old, -1);
}

1;
