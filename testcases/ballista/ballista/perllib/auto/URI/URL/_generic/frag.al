# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

# No efrag method because the fragment is always stored unescaped
sub frag     { shift->_elem('frag', @_); }

1;
