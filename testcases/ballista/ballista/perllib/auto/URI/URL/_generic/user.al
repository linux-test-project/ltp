# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

# Fields derived from generic netloc:
sub user     { shift->_netloc_elem('user',    @_); }
1;
