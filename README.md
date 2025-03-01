# Some Grid Engine/Son of Grid Engine/Sun Grid Engine
<details>
<summary>Table of contents</summary>
<ol>
	<li><a href='#about-sge'>About SGE</a></li>
	<li><a href='#requirements'>Requirements</a></li>
	<li><a href='#three-different-installation-methods'>Three different installation methods</a></li>
	<li><a href='#quick-test-on-one-machine'>Quick test on one machine</a></li>
	<li><a href='#installation'>Installation</a></li>
	<li><a href='#contribute'>Contribute</a></li>
	<li><a href='#license'>License</a></li>
	<li><a href='#acknowledgments'>Acknowledgments</a></li>
<ol>
</details>

## About SGE

Some Grid Engine is a fork of [Son of Grid Engine](https://en.wikipedia.org/wiki/Oracle_Grid_Engine) at University of Liverpool, with SOME improvement.

We have been using, maintaining this famous software at Michigan Neuroscience Institute, University of Michigan for over a decade. We also test it weekly with the latest version of all major Linux distros. It is stable and good enough for a small HPC cluster. Here share it on GitHub, hoping more peoples can benefit from it.

### Improvements
- Compatible with [musl libc](https://musl.libc.org/) with ported glibc rresvport, tested with [Void Linux musl](https://voidlinux.org/) and [Alpine Linux](https://www.alpinelinux.org).
- Support for submitting jobs via SystemD which also allows to enforce memory/cpu limitations via kernel cgroups. Many thanks to [fretn](https://github.com/fretn/sge) and [ondrejv2](https://github.com/ondrejv2/sge)!
- CMake compiling support. This paved the way for easier maintenance in future. It took 38 seconds to compile in parallel and install on an 8-core old machine, while it took 302 seconds with the legacy SGE way, and 377 seconds with makepkg.
- Fixed a permission error introduced since systemd 241 in 2019 during installation, if SGE is installed as non-root on production system
- Compatible with the latest version of openssl
- All C compling warning are fixed on Arch Linux and Void Linux. Most of them were caused by 'smarter' gcc, new SSL, new GLIBC, obsolete function such 'sigignore', depreciated function such as 'readdir\_r', etc.
- Underscore in port service name 'sge\_qmaster/sge\_execd' is changed to hyphen in all C files and shell scripts, saving us from modifying /etc/services each time
- Supports both runit on Void Linux and systemd on other distros
- Version is changed to the commit version of this github repo
- [5 keystrokes to setup a demo cluster on any Linux machine without root privilege](tests/5-keystrokes-to-setup-a-cluster-without-root-privilege/)
- [Automated weekly tests for a lot of Linux distros](tests/)


## Requirements
### tested with all the Linux distributions below, patched up to the specified date

- **Arch Linux**, 2025-03-01
```
pacman -Sy --needed cmake db gcc git hwloc inetutils m4 make man openmotif pkgconf vi
```

- **Void Linux**, 2025-03-01, x86\_64, glibc/musl
```
xbps-install cmake gcc git hwloc libhwloc-devel libtirpc-devel m4 make motif-devel ncurses-devel openssl-devel pam-devel
```

- **Alpine Linux**, 2025-03-01, x86\_64, Edge
```
apk add cmake db-dev g++ gcc git hwloc-dev libtirpc-dev libxt-dev linux-pam-dev m4 make motif-dev ncurses-dev openssl-dev procps
#Due to the conflict with usr/include/libintl.h owned by both gettext-dev and musl-libintl
apk fetch musl-libintl
tar -C / -xf musl-libintl*.apk usr/include/libintl.h
```

- **AlmaLinux 8.10** and **Rocky 8.10**, 2025-03-01, with SELinux set to permissive
```
dnf group install "Development Tools"
dnf --enablerepo=powertools install cmake hwloc-devel libdb-devel libtirpc-devel motif-devel ncurses-devel openssl-devel pam-devel rsync systemd-devel wget
```

- **AlmaLinux 9.5**, 2025-03-01, with SELinux set to permissive
```
dnf group install "Development Tools"
dnf install cmake hwloc-devel libdb-devel motif-devel ncurses-devel openssl-devel pam-devel rsync systemd-devel wget
dnf install https://repo.almalinux.org/almalinux/9/CRB/x86_64/os/Packages/libtirpc-devel-1.3.3-9.el9.x86_64.rpm
```

- **Rocky 9.5**, 2025-03-01, with SELinux set to permissive
```
dnf group install "Development Tools"
dnf install cmake hwloc-devel libdb-devel motif-devel ncurses-devel openssl-devel pam-devel rsync systemd-devel wget
dnf install https://dl.rockylinux.org/pub/rocky/9/CRB/x86_64/os/Packages/l/libtirpc-devel-1.3.3-9.el9.x86_64.rpm
```

- **Debian Bookworm/Bullseye**, 2025-03-01
```
apt install build-essential cmake git libdb5.3-dev libhwloc-dev libmotif-dev libncurses-dev libpam0g-dev libssl-dev libsystemd-dev libtirpc-dev libxext-dev pkgconf rsync
```

- **Ubuntu Server 24.04, 22.04, 20.04**, 2025-03-01
```
apt-get install build-essential cmake git libdb5.3-dev libhwloc-dev libmotif-dev libncurses-dev libpam0g-dev libssl-dev libsystemd-dev libtirpc-dev libxext-dev pkgconf

```

- **openSUSE Leap**, 2025-03-01
```
zypper -n addrepo http://download.opensuse.org/distribution/leap/15.5/repo/oss/ oss
zypper -n install cmake gcc gcc-c++ git hwloc-devel libdb-4_8-devel libtirpc-devel libXext-devel motif-devel ncurses-devel openssl-devel pam-devel pkgconf rsync systemd-devel wget
```

## Three different installation methods

1) **CMake**, recommended
```
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/sge
cmake --build build -j
sudo cmake --install build
```
Please check [the tested environment below](#environmet) in case of any compiling issue.

2) **Legacy SGE installation on modern Linux distributions**
```
make
sudo make install
```
Please check the original source/README.BUILD for detail

3) **Legacy SGE installation on Arch Linux**
```
cp PKGBUILD.in PKGBUILD
makepkg
sudo pacman -U sge-r*.pkg.tar.zst
```

## Quick test on one machine
- step 1, as root.

```
useradd -r -d /opt/sge sge
chown -R sge /opt/sge
cd /opt/sge
yes "" | ./install_qmaster
yes "" | ./install_execd
source /opt/sge/default/common/settings.sh
qhost -q #you should be able to see five lines of output
qconf -as $HOSTNAME #add this node as submit host
```

- step 2, as a regular account
```
source /opt/sge/default/common/settings.sh
echo hostname | qsub -cwd
watch qstat #check job status
ls STDIN.* #check job output
```

## Installation

All SGE services are running under user **sge** for security reason, as this is production system.

Assuming master node hostname is **master**, and execution nodes hostnames is **node-XX**.
**/etc/hosts** on master and all nodes shoud be like it:
```
10.1.1.1	 master
10.1.1.11	 node-01
10.1.1.12	 node-02
...
10.1.1.1N	 node-0N
```

#### The first - on all nodes as root
```
ping master
ping node-XX
useradd -u <UID> -r -d /opt/sge sge
```
sge UID should be identical on all machines.

#### The second - on master as root

First of all, change option **admin_user** in [bootstrapfile](http://gridscheduler.sourceforge.net/htmlman/htmlman5/bootstrap.html)
```
chown -R sge /opt/sge
cd /opt/sge
yes "" | ./install_qmaster
source /opt/sge/default/common/settings.sh
qconf -as master
```
Next commands it's necessary to run for each nodes in cluster:
```
qconf -ah node-01
qconf -ah node-02
...
qconf -ah node-0N
```

#### The third - on all execution nodes as root
```
mkdir -p /opt/sge/default
chown -R sge /opt/sge/default
scp -pr master:/opt/sge/default/common /opt/sge/default/common
cd /opt/sge
yes "" | ./install_execd
source /opt/sge/default/common/settings.sh
qhost -q
```

#### The fourth - on master as any non-root user
```
source /opt/sge/default/common/settings.sh
echo hostname | qsub -cwd
watch qstat
cat STDIN.*
```

## Contribute

Contributions are always welcome!

## License

Written by [Manhong Dai](mailto:daimh@umich.edu)<br>
Copyright © 2002 University of Michigan.<br>
License [SISSL](https://opensource.org/licenses/sisslpl)<br>
This is free software: you are free to change and redistribute it.<br>
There is NO WARRANTY, to the extent permitted by law.<br>

## Acknowledgments

https://arc.liv.ac.uk/trac/SGE, Sun, and Oracle<br>
[fretn](https://github.com/fretn/sge)<br>
[ondrejv2](https://github.com/ondrejv2/sge)<br>
Ruth Freedman, MPH, former administrator of MNI, UMICH<br>
Thomas Wilson, M.D., Ph.D. Professor of Pathology, UMICH<br>
Huda Akil, Ph.D., Director of MNI, UMICH<br>
Stanley J. Watson, M.D., Ph.D., Director of MNI, UMICH<br>
Fan Meng, Ph.D., Research Associate Professor, Psychiatry, UMICH<br>
