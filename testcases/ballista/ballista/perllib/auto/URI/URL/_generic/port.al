# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

sub port {
    my $self = shift;
    my $old = $self->_netloc_elem('port', @_);
    defined($old) ? $old : $self->default_port;
}

1;
