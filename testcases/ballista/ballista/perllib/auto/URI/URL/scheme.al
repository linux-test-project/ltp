# NOTE: Derived from ./blib/lib/URI/URL.pm.  Changes made here will be lost.
package URI::URL;

sub scheme {
    my $self = shift;
    my $old = $self->{'scheme'};
    if (@_) {
	my $new_scheme = shift;
	if (defined($new_scheme) && length($new_scheme)) {
	    # reparse URL with new scheme
	    my $str = $self->as_string;
	    $str =~ s/^[\w+\-.]+://;
	    my $newself = new URI::URL "$new_scheme:$str";
	    %$self = %$newself;
	    bless $self, ref($newself);
	} else {
	    $self->{'scheme'} = undef;
	}
    }
    $old;
}

1;
