# NOTE: Derived from ./blib/lib/URI/URL.pm.  Changes made here will be lost.
package URI::URL;

sub strict
{
    return $Strict_URL unless @_;
    my $old = $Strict_URL;
    $Strict_URL = $_[0];
    $old;
}

1;
