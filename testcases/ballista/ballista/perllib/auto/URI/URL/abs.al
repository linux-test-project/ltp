# NOTE: Derived from ./blib/lib/URI/URL.pm.  Changes made here will be lost.
package URI::URL;

# These are overridden by _generic (this is just a noop for those schemes that
# do not wish to be a subclass of URI::URL::_generic)
sub abs { shift->clone; }
1;
