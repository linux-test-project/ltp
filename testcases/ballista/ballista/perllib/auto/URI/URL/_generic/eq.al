# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

# Compare two URLs
sub eq {
    my($self, $other) = @_;
    local($^W) = 0; # avoid warnings if we compare undef values
    $other = URI::URL->new($other, $self) unless ref $other;

    # Compare scheme and netloc
    return 0 if ref($self) ne ref($other);                # must be same class
    return 0 if $self->scheme ne $other->scheme;          # Always lower case
    return 0 if lc($self->netloc) ne lc($other->netloc);  # Case-insensitive

    # Compare full_path:
    # According to <draft-ietf-http-v11-spec-05>:
    # Characters other than those in the "reserved" and "unsafe" sets
    # are equivalent to their %XX encodings.
    my $fp1 = $self->full_path;
    my $fp2 = $other->full_path;
    for ($fp1, $fp2) {
	s,%([\dA-Fa-f]{2}),
	  my $x = $1;
	  my $c = chr(hex($x));
	  $c =~ /^[;\/?:\@&=+\"\#%<>\0-\040\177]/ ? "%\L$x" : $c;
	,eg;
    }
    return 0 if $fp1 ne $fp2;
    return 0 if $self->frag ne $other->frag;
    1;
}

1;
1;
