# NOTE: Derived from ./blib/lib/LWP/UserAgent.pm.  Changes made here will be lost.
package LWP::UserAgent;

sub no_proxy {
    my($self, @no) = @_;
    if (@no) {
	push(@{ $self->{'no_proxy'} }, @no);
    }
    else {
	$self->{'no_proxy'} = [];
    }
}

1;
