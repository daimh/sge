#!/bin/sh

# Generate Emacs TAGS file for C and Java files in the current directory
# and below it.

# (Can't assume we have -print0.)
# Ignore the 3rparty stuff if run in source.
rm -f TAGS
find common clients daemons libs security utilbin -name \*.[ch] -o -name \*.java |
  xargs etags --regex='{c}/\(NAME\|LIST\)DEF\(([A-Za-z0-0_]+)\)/\2/' -a
mkid -i C common clients daemons libs security utilbin
