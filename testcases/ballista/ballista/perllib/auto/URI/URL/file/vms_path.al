# NOTE: Derived from ./blib/lib/URI/URL/file.pm.  Changes made here will be lost.
package URI::URL::file;

sub vms_path
{
    # ????? Can some VMS people please redo this function ??????

    # This is implemented based on what RFC1738 (sec 3.10) says in the
    # VMS file example:
    #
    #  DISK$USER:[MY.NOTES]NOTE123456.TXT
    #
    #      that might become
    #
    #  file:/disk$user/my/notes/note12345.txt
    #
    # BEWARE: I don't have a VMS machine myself so this is pure guesswork!!!

    my $self = shift;
    my @p = $self->path_components;
    my $abs = 0;
    if (@p && $p[0] eq '') {
	shift @p;
	$abs = 1;
    }
    # First I assume there must be a dollar in a disk spesification
    my $p = '';
    $p = uc(shift(@p)) . ":"  if @p && $p[0] =~ /\$/;
    my $file = pop(@p);
    $p .= "[" . join(".", map{uc($_)} @p) . "]" if @p;
    $p .= uc $file;
    # XXX: How is an absolute path different from a relative one??
    $p =~ s/\[/[./ unless $abs;
    # XXX: How is a directory denoted??
    $p;
}

1;
1;
