#!/usr/bin/env sh
set -eu
cd $1/bin/lx-amd64
ln -fs qalter qhold
ln -fs qalter qresub
ln -fs qalter qrls
ln -fs qsh qlogin
ln -fs qsh qrsh
ln -fs qstat qselect
cd $1/qmon/PIXMAPS/small
mv *.xpm ../
