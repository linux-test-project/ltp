# NOTE: Derived from ./blib/lib/LWP/UserAgent.pm.  Changes made here will be lost.
package LWP::UserAgent;

sub clone
{
    my $self = shift;
    my $copy = bless { %$self }, ref $self;  # copy most fields

    # elements that are references must be handled in a special way
    $copy->{'no_proxy'} = [ @{$self->{'no_proxy'}} ];  # copy array

    $copy;
}

1;
