# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

sub params {
    my $self = shift;
    my $old = $self->_elem('params', map {uri_escape($_,$URI::URL::reserved_no_form)} @_);
    return uri_unescape($old) if defined $old;
    undef;
}

1;
