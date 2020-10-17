<img src="mni.png" align="right" />

# Some Grid Engine/Son of Grid Engine/Sun Grid Engine

A fork of Son of Grid Engine at University of Liverpool, with SOME improvement.

We have been using and maintaining this software at Michigan Neuroscience Institute, University of Michigan for over a decade. It is stable and good enough for a small HPC cluster. Here we share it on github, hoping more peoples can benefit from it.

## Three installation methods

- CMake, this will be always supported
```
$ cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge
$ cmake --build build -j 
$ sudo cmake --install build 
```

- Standard SGE installation on modern Linux distributions, check the original source/README.BUILD for detail
```
$ make
$ sudo make install
```

- Standard SGE installation on Arch Linux
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

## Improvements

- Underscore in port service name 'sge\_qmaster/sge\_execd' is changed to hyphen in all C files and shell scripts, saving us from modifying /etc/services each time
- Systemd support
- Version is changed to the commit version of this github repo
- All warning are fixed. Most of them were caused by 'smarter' gcc, new SSL, new GLIBC, obsolete function such 'sigignore', depreciated function such as 'readdir\_r'.
- CMake compiling support. This paved the way for easier maintenance in future. It took 38 seconds to compile in parallel and install on an 8-core old machine, while it took 302 seconds with the standard SGE way, and 377 seconds with makepkg.

## Contribute

Contributions are always welcome!

## License

GPLv3

To the extent possible under law, [Manhong Dai](mailto:daimh@umich.edu) has waived all copyright and related or neighboring rights to this work.

## Acknowledge

Ruth Freedman, MPH, former administrator of MNI, UMICH

Fan Meng, Ph.D., Researach Associate Professor, Psychiatry, UMICH

Huda Akil, Ph.D., Director of MNI, UMICH

Stanley J. Watson, M.D., Ph.D., Director of MNI, UMICH

Also thanks to https://arc.liv.ac.uk/trac/SGE and Sun company. 

no thanks to Oracle though, :)
