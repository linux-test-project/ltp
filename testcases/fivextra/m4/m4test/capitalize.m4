dnl
dnl convert to upper- resp. lowercase
define(`upcase', `translit(`$*', `a-z', `A-Z')')
define(`downcase', `translit(`$*', `A-Z', `a-z')')
upcase(`Convert to upper case')
downcase(`Convert To LOWER Case')
dnl
dnl capitalize a single word
define(`capitalize1', `regexp(`$1', `^\(\w\)\(\w*\)', `upcase(`\1')`'downcase(`\2')')')
define(`capitalize', `patsubst(`$1', `\w+', ``'capitalize1(`\&')')')
capitalize(`This sentence should be capitalized')
