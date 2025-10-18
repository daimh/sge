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

Some Grid Engine is a fork of [Son of Grid Engine](https://en.wikipedia.org/wiki/Oracle_Grid_Engine) from the University of Liverpool, with several improvements.  
Many thanks to the Wikipedia editor who included this repository in the [Comparison of cluster software](https://en.wikipedia.org/wiki/Comparison_of_cluster_software).  

We have been using and maintaining this software at the **Michigan Neuroscience Institute, University of Michigan** for over a decade.  We also have been testing it weekly against the latest versions of all major Linux distributions.  
It has proven to be stable and well-suited for small HPC clusters.  

We are sharing it here on GitHub in the hope that more people can benefit from it.  

---

### Improvements

- **musl libc compatibility**  
  Ported `glibc rresvport` to work with [musl libc](https://musl.libc.org/).  
  Tested on [Void Linux (musl)](https://voidlinux.org/) and [Alpine Linux](https://www.alpinelinux.org/).  

- **SystemD job submission support**  
  Jobs can now be submitted via SystemD, allowing enforcement of memory and CPU limits via kernel cgroups.  
  Thanks to [fretn](https://github.com/fretn/sge) and [ondrejv2](https://github.com/ondrejv2/sge)!  

- **CMake build support**  
  Added CMake for easier long-term maintenance.  
  On an 15-year old 8-core machine:  
  - 38 seconds with CMake (parallel compile & install)  
  - 302 seconds with legacy SGE build  
  - 377 seconds with `makepkg`  

- **Systemd installation fix**  
  Fixed a permission error (introduced with systemd 241 in 2019) when installing SGE as a non-root user.  

- **OpenSSL compatibility**  
  Updated for the latest OpenSSL versions.  

- **Compiler warnings resolved**  
  All C compilation warnings fixed on Arch Linux and Void Linux.  
  Issues were caused by stricter GCC, new SSL/GLIBC, and obsolete/deprecated functions (`sigignore`, `readdir_r`, etc.).  

- **Service name cleanup**  
  Changed underscores to hyphens in service names (`sge_qmaster/sge_execd` → `sge-qmaster/sge-execd`).  
  This avoids modifying `/etc/services` manually.  

- **Init system support**  
  Supports both `runit` (Void Linux) and `systemd` (other distros).  

- **Versioning**  
  Version numbers now correspond to the commit hash of this GitHub repository.  

- **[5 keystrokes to setup a demo cluster on any Linux machine without root privilege](tests/5-keystrokes-to-setup-a-cluster-without-root-privilege/)**

- **[Automated weekly tests for a lot of Linux distros](tests/)**


## Requirements
### tested with all the Linux distributions below, patched up to the specified date

- **Arch Linux**, 2025-10-18
```
pacman -Sy --needed cmake db gcc git hwloc inetutils m4 make man pkgconf vi
```

- **Void Linux**, 2025-10-18, x86\_64, glibc/musl
```
xbps-install cmake gcc git hwloc libhwloc-devel libtirpc-devel m4 make ncurses-devel openssl-devel pam-devel
```

- **Alpine Linux**, 2025-10-18, x86\_64, Edge
```
apk add cmake db-dev g++ gcc git hwloc-dev libtirpc-dev libxt-dev linux-pam-dev m4 make ncurses-dev openssl-dev procps
#Due to the conflict with usr/include/libintl.h owned by both gettext-dev and musl-libintl
apk fetch musl-libintl
tar -C / -xf musl-libintl*.apk usr/include/libintl.h
```

- **AlmaLinux 10.0**, 2025-10-18, with SELinux set to permissive
```
dnf group install "Development Tools"
dnf install cmake hwloc-devel ncurses-devel openssl-devel pam-devel rsync systemd-devel wget
dnf install https://dl.fedoraproject.org/pub/epel/10/Everything/x86_64/Packages/l/libdb-5.3.28-64.el10_0.x86_64.rpm
dnf install https://dl.fedoraproject.org/pub/epel/10/Everything/x86_64/Packages/l/libdb-devel-5.3.28-64.el10_0.x86_64.rpm
dnf install https://repo.almalinux.org/almalinux/10/CRB/x86_64/os/Packages/libtirpc-devel-1.3.5-1.el10.x86_64.rpm
```

- **Rocky 10.0**, 2025-10-18, with SELinux set to permissive
```
dnf group install "Development Tools"
dnf install cmake hwloc-devel ncurses-devel openssl-devel pam-devel rsync systemd-devel wget
dnf install https://dl.fedoraproject.org/pub/epel/10/Everything/x86_64/Packages/l/libdb-5.3.28-64.el10_0.x86_64.rpm
dnf install https://dl.fedoraproject.org/pub/epel/10/Everything/x86_64/Packages/l/libdb-devel-5.3.28-64.el10_0.x86_64.rpm
dnf install https://dl.rockylinux.org/pub/rocky/10/CRB/x86_64/os/Packages/l/libtirpc-devel-1.3.5-1.el10.x86_64.rpm
```

- **AlmaLinux 9.6**, 2025-10-18, with SELinux set to permissive
```
dnf group install "Development Tools"
dnf install cmake hwloc-devel libdb-devel ncurses-devel openssl-devel pam-devel rsync systemd-devel wget
dnf install https://repo.almalinux.org/almalinux/9/CRB/x86_64/os/Packages/libtirpc-devel-1.3.3-9.el9.x86_64.rpm
```

- **Rocky 9.6**, 2025-10-18, with SELinux set to permissive
```
dnf group install "Development Tools"
dnf install cmake hwloc-devel libdb-devel ncurses-devel openssl-devel pam-devel rsync systemd-devel wget
dnf install https://dl.rockylinux.org/pub/rocky/9/CRB/x86_64/os/Packages/l/libtirpc-devel-1.3.3-9.el9.x86_64.rpm
```

- **AlmaLinux 8.10** and **Rocky 8.10**, 2025-10-18, with SELinux set to permissive
```
dnf group install "Development Tools"
dnf --enablerepo=powertools install cmake hwloc-devel libdb-devel libtirpc-devel ncurses-devel openssl-devel pam-devel rsync systemd-devel wget
```

- **Debian Trixie/Bookworm/Bullseye**, 2025-10-18
```
apt install build-essential cmake git libdb5.3-dev libhwloc-dev libncurses-dev libpam0g-dev libssl-dev libsystemd-dev libtirpc-dev libxext-dev pkgconf rsync
```

- **Ubuntu Server 24.04, 22.04, 20.04**, 2025-10-18
```
apt-get install build-essential cmake git libdb5.3-dev libhwloc-dev libncurses-dev libpam0g-dev libssl-dev libsystemd-dev libtirpc-dev libxext-dev pkgconf

```

- **openSUSE Leap**, 2025-10-18
```
zypper -n addrepo http://download.opensuse.org/distribution/leap/15.6/repo/oss/ oss
zypper -n install cmake gcc gcc-c++ git hwloc-devel libdb-4_8-devel libtirpc-devel libXext-devel m4 ncurses-devel openssl-devel pam-devel pkgconf rsync systemd-devel wget
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

Written by [Manhong Dai](mailto:daimh@umich.edu)
Copyright © 2002-2022 University of Michigan.
License [SISSL](https://opensource.org/licenses/sisslpl)
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

This fork will be free in both "*gratis*" and "*libre*".
For other active forks that have commercial support, please look into
- [Open Cluster Scheduler](https://github.com/hpc-gridware/clusterscheduler)
- [Altair Grid Engine](https://altair.com/grid-engine)

## Acknowledgments

https://arc.liv.ac.uk/trac/SGE, Sun, and Oracle  
[fretn](https://github.com/fretn/sge)  
[ondrejv2](https://github.com/ondrejv2/sge)  
Ruth Freedman, MPH, former administrator of MNI, UMICH  
Thomas Wilson, M.D., Ph.D. Professor of Pathology, UMICH  
Huda Akil, Ph.D., Director of MNI, UMICH  
Stanley J. Watson, M.D., Ph.D., Director of MNI, UMICH  
Fan Meng, Ph.D., Research Associate Professor, Psychiatry, UMICH  
