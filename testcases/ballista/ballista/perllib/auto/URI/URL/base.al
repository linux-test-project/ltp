# NOTE: Derived from ./blib/lib/URI/URL.pm.  Changes made here will be lost.
package URI::URL;

# Access some attributes of a URL object:
sub base {
    my $self = shift;
    my $base  = $self->{'_base'};

    if (@_) { # set
	my $new_base = shift;
	$new_base = $new_base->abs if ref($new_base);  # unsure absoluteness
	$self->{_base} = $new_base;
    }
    return unless defined wantarray;

    # The base attribute supports 'lazy' conversion from URL strings
    # to URL objects. Strings may be stored but when a string is
    # fetched it will automatically be converted to a URL object.
    # The main benefit is to make it much cheaper to say:
    #   new URI::URL $random_url_string, 'http:'
    if (defined($base) && !ref($base)) {
	$base = new URI::URL $base;
	$self->_elem('_base', $base); # set new object
    }
    $base;
}

1;
