<img src="mni.png" align="right" />

# Some Grid Engine/Son of Grid Engine/Sun Grid Engine

A fork of Son of Grid Engine at University of Liverpool, with SOME improvement.

We have been using and maintaining this software at Michigan Neuroscience Institute, University of Michigan for over a decade. It is stable and good enough for a small HPC cluster. Here we share it on github, hoping more peoples can benefit from it.

## Improvements

- Underscore in port service name 'sge\_qmaster/sge\_execd' is changed to hyphen in all C files and shell scripts, saving us from modifying /etc/services each time
- Systemd support
- Version is changed to the commit version of this github repo
- All warning are fixed on Arch Linux. Most of them were caused by 'smarter' gcc, new SSL, new GLIBC, obsolete function such 'sigignore', depreciated function such as 'readdir\_r'.
- CMake compiling support. This paved the way for easier maintenance in future. It took 38 seconds to compile in parallel and install on an 8-core old machine, while it took 302 seconds with the legacy SGE way, and 377 seconds with makepkg.

## Three installation methods

- CMake
```
$ cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge
$ cmake --build build -j 
$ sudo cmake --install build 
```
Please check the tested building environment below in case of any compiling issue.

- Legacy SGE installation on modern Linux distributions, check the original source/README.BUILD for detail
```
$ make
$ sudo make install
```

- Legacy SGE installation on Arch Linux
```
$ cp PKGBUILD.in PKGBUILD
$ makepkg
$ sudo pacman -U sge-r*.pkg.tar.zst
```

## Quick test on one node
- step 1, as root. 
```
$ cd /opt/sge
$ yes "" | ./install_qmaster
$ yes "" | ./install_execd
$ . /opt/sge/default/common/settings.sh 
$ qhost -q #you should be able to see five lines of output
$ qconf -as $(hostname -s) #add this node as submit host
```
please make sure there is no SGE process running with 'ps -ef |grep sge' and directory '/opt/sge/default' is removed if you want to run the commands above again

- step 2, as a regular account
```
$ . /opt/sge/default/common/settings.sh 
$ echo hostname | qsub -cwd
$ qstat #check job status
$ ls STDIN.* #check job output
```

## CMake building, tested with all the below Linux distributions patched up to date, on 2020-10-18

- Arch Linux
```
$ pacman -Sy --needed git cmake make gcc openmotif hwloc
```

- Debian Buster, with "standard system utilities" checked during installation, and cmake 3.18.4 downloaded from cmake.org
```
$ apt install git build-essential libhwloc-dev libssl-dev libtirpc-dev libmotif-dev libxext-dev libncurses-dev libdb5.3-dev libpam0g-dev
```

- Ubuntu Server 20.04
```
$ apt install git build-essential libhwloc-dev libssl-dev libtirpc-dev libmotif-dev libxext-dev libncurses-dev libdb5.3-dev libpam0g-dev cmake
```

- CentOS 8.2 with SELinux set to permissive, and cmake 3.18.4 downloaded from cmake.org
```
$ dnf group install "Development Tools"
$ dnf --enablerepo=PowerTools install hwloc-devel openssl-devel libtirpc-devel motif-devel ncurses-devel libdb-devel pam-devel
```

## Contribute

Contributions are always welcome!

## Copyright

Developed by [Manhong Dai](mailto:daimh@umich.edu)

Copyright © 2020 University of Michigan. License [GPLv3+](https://gnu.org/licenses/gpl.html): GNU GPL version 3 or later 

This is free software: you are free to change and redistribute it.

There is NO WARRANTY, to the extent permitted by law.

## Acknowledgment

Thomas Wilson, M.D., Ph.D. Professor of Pathology, UMICH

Ruth Freedman, MPH, former administrator of MNI, UMICH

Fan Meng, Ph.D., Research Associate Professor, Psychiatry, UMICH

Huda Akil, Ph.D., Director of MNI, UMICH

Stanley J. Watson, M.D., Ph.D., Director of MNI, UMICH

Also thanks to https://arc.liv.ac.uk/trac/SGE and Sun company. 

no thanks to Oracle though, :)
