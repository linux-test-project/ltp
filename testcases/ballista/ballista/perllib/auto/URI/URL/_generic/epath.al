# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

sub epath {
     my $self = shift;
     my $old = $self->_elem('path', @_);
     return '/' if !defined($old) || !length($old);
     return "/$old" if $old !~ m|^/| && defined $self->{'netloc'};
     $old;
}

1;
