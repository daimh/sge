<img src="mni.png" align="right" />

# Some Grid Engine/Son of Grid Engine/Sun Grid Engine

Some Grid Engine is a fork of Son of Grid Engine at University of Liverpool, with SOME improvement.

We have been using and maintaining this software at Michigan Neuroscience Institute, University of Michigan for over a decade. It is stable and good enough for a small HPC cluster. Here we share it on github, hoping more peoples can benefit from it.

## Improvements

- CMake compiling support. This paved the way for easier maintenance in future. It took 38 seconds to compile in parallel and install on an 8-core old machine, while it took 302 seconds with the legacy SGE way, and 377 seconds with makepkg.
- Fixed a permission error introduced since systemd 241 in 2019 during installation, if SGE is installed as non-root on production system
- Compatible with openssl-1.1.1
- All C compling warning are fixed on Arch Linux and Void Linux. Most of them were caused by 'smarter' gcc, new SSL, new GLIBC, obsolete function such 'sigignore', depreciated function such as 'readdir\_r', etc.
- Underscore in port service name 'sge\_qmaster/sge\_execd' is changed to hyphen in all C files and shell scripts, saving us from modifying /etc/services each time
- Supports both systemd and runit on Void Linux
- Version is changed to the commit version of this github repo

## Three installation methods

- CMake
```
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge
cmake --build build -j
sudo cmake --install build
```
Please check [the tested environment below](#environmet) in case of any compiling issue.

- Legacy SGE installation on modern Linux distributions, check the original source/README.BUILD for detail
```
make
sudo make install
```

- Legacy SGE installation on Arch Linux
```
cp PKGBUILD.in PKGBUILD
makepkg
sudo pacman -U sge-r*.pkg.tar.zst
```

## Quick test on one node
- step 1, as root.
```
cd /opt/sge
yes "" | ./install_qmaster
yes "" | ./install_execd
source /opt/sge/default/common/settings.sh
qhost -q #you should be able to see five lines of output
qconf -as $HOSTNAME #add this node as submit host
```
please make sure there is no SGE process running with 'ps -ef |grep sge' and directory '/opt/sge/default' is removed if you want to run the commands above again

- step 2, as a regular account
```
source /opt/sge/default/common/settings.sh
echo hostname | qsub -cwd
watch qstat #check job status
ls STDIN.* #check job output
```

## Production Installation on a share-nothing two-node system

All SGE services are running under user 'sge' for security reason, as this is production system.

Tested with the latest Arch Linux on Oct 27, 2020, on two nodes created by [daiker](https://github.com/daimh/daiker) with command 'daiker run -PT 22 ...'

Assuming master node hostname is 'master-node', and execution node hostname is 'exec-node'. /etc/hosts on both nodes have these two entries
```
10.1.1.1	master-node
10.1.1.2	exec-node
```

- step 1, on both nodes as root
```
ping master-node
ping exec-node
pacman -Sy --needed git cmake make gcc openmotif hwloc vi inetutils
useradd -r -d /opt/sge sge
mkdir /opt/sge
chown sge /opt/sge
git clone https://github.com/daimh/sge.git
cd sge
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge
cmake --build build -j 
cmake --install build
```

- step 2, on master-node as root
```
cd /opt/sge
yes "" | ./install_qmaster
source /opt/sge/default/common/settings.sh
qconf -ah exec-node
qconf -as exec-node
```

- step 3, on exec-node as root
```
mkdir -p /opt/sge/default
chown -R sge /opt/sge/default
scp -pr master-node:/opt/sge/default/common /opt/sge/default/common
cd /opt/sge
yes "" | ./install_execd
source /opt/sge/default/common/settings.sh
qhost -q
su - sge
```

- step 4, on exec-node as sge
```
source /opt/sge/default/common/settings.sh
echo hostname | qsub -cwd
watch qstat
cat STDIN.*
```

## <a name=environmet></a>CMake building, tested with all the Linux distributions below, patched up to date

- Arch Linux, 2020-10-18
```
pacman -Sy --needed git cmake make gcc openmotif hwloc vi inetutils
```

- Debian Buster, 2020-10-18, with "standard system utilities" checked during installation, and cmake 3.18.4 downloaded from cmake.org
```
apt install git build-essential libhwloc-dev libssl-dev libtirpc-dev libmotif-dev libxext-dev libncurses-dev libdb5.3-dev libpam0g-dev
```

- Ubuntu Server 20.04, 2020-10-18
```
apt install git build-essential libhwloc-dev libssl-dev libtirpc-dev libmotif-dev libxext-dev libncurses-dev libdb5.3-dev libpam0g-dev cmake
```

- CentOS 8.2, 2020-10-18, with SELinux set to permissive, and cmake 3.18.4 downloaded from cmake.org
```
dnf group install "Development Tools"
dnf --enablerepo=PowerTools install hwloc-devel openssl-devel libtirpc-devel motif-devel ncurses-devel libdb-devel pam-devel
```

- Void Linux, 2021-05-26, x86\_64, Glibc
```
xbps-install cmake make gcc openssl-devel motif-devel hwloc libhwloc-devel libtirpc-devel ncurses-devel pam-devel
```

## Contribute

Contributions are always welcome!

## Copyright

Written by [Manhong Dai](mailto:daimh@umich.edu)

Copyright © 2021 University of Michigan. License [SISSL](https://opensource.org/licenses/sisslpl)

This is free software: you are free to change and redistribute it.

There is NO WARRANTY, to the extent permitted by law.

## Acknowledgment

Thomas Wilson, M.D., Ph.D. Professor of Pathology, UMICH

Ruth Freedman, MPH, former administrator of MNI, UMICH

Fan Meng, Ph.D., Research Associate Professor, Psychiatry, UMICH

Huda Akil, Ph.D., Director of MNI, UMICH

Stanley J. Watson, M.D., Ph.D., Director of MNI, UMICH

Also thanks to https://arc.liv.ac.uk/trac/SGE, Sun, and Oracle
