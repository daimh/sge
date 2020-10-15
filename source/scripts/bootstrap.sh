#!/bin/sh

set -e

if [ -f scripts/zerodepend ]; then
csh -f aimk -only-depend "$@" &&
scripts/zerodepend &&
./aimk "$@" depend
else
    echo "Run `basename $0` from the distribution source directory" >&2
    exit 1
fi

echo "Now run ./aimk with any appropriate options (see ./aimk -help)"
