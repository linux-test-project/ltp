# NOTE: Derived from ./blib/lib/URI/URL.pm.  Changes made here will be lost.
package URI::URL;

# Compare two URLs, subclasses will provide a more correct implementation
sub eq {
    my($self, $other) = @_;
    $other = URI::URL->new($other, $self) unless ref $other;
    ref($self) eq ref($other) &&
      $self->scheme eq $other->scheme &&
      $self->as_string eq $other->as_string;  # Case-sensitive
}

1;
