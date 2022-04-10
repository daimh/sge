#!/usr/bin/env sh
set -e
cd $1/bin/lx-amd64
ln -fs qalter qhold
ln -fs qalter qresub
ln -fs qalter qrls
ln -fs qsh qlogin
ln -fs qsh qrsh
ln -fs qstat qselect
echo -e "#!/usr/bin/env bash\necho 'qmake is retired due to issue #18 at github. we can revisit this issue if needed'" > qmake 
chmod +x qmake
