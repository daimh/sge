# Five keystrokes to set up a cluster without root privilege

This directory sets up a demo cluster with one 'make' command on any Linux machine. No root privileges are needed. The default root password is 'SomeGridEngine' on all nodes.

It sets up three nodes. One is a firewall with two NIC devices, one is public-facing, the other is for the internal network only. The master node and all compute nodes connect to the internal network. In addition, the compute nodes boot up via network PXE and memory file system, no local storage is needed. Their maintenance is minimal as all the compute nodes use the same image file stored on the firewall.

Further, the ISO file this demo creates can be applied to a real-world cluster with little modification. 'dd' the ISO file to a USB, then set up the firewall node and the master node with the USB. No installation is needed for those compute nodes as they will boot up via network.

In this demo, ens3 on the firewall node connects to the Internet, with its SSH daemon listens at port 12345, ens4 is 192.168.234.1/24. The master node is 192.168.234.2, while the compute nodes are 192.168.234.100-109. If you don't have X window, replace 'gtk' with 'vnc' in Makefile line 3.

Here are [two videos](https://youtube.com/playlist?list=PLcUreuc9RezIPip5ShBr3Wg1RrnrAv45X) recorded on 2022-02-27 with the latest Arch Linux.
