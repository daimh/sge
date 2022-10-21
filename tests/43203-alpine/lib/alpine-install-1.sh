cat > answer << EOF
KEYMAPOPTS="us us"
HOSTNAMEOPTS=sge-alpine
DEVDOPTS=mdev
INTERFACESOPTS="auto lo
iface lo inet loopback
auto eth0
iface eth0 inet dhcp
	hostname sge-alpine
"
TIMEZONEOPTS=none
PROXYOPTS=none
APKREPOSOPTS="-1"
#USEROPTS="-a -u sge"
USEROPTS=none
SSHDOPTS=openssh
ROOTSSHKEY="http://10.0.2.2:8000/id_ed25519.pub"
NTPOPTS=none
DISKOPTS="-m sys /dev/sda"
#LBUOPTS="LABEL=APKOVL"
LBUOPTS=none
#APKCACHEOPTS="/media/LABEL=APKOVL/cache"
APKCACHEOPTS=none
EOF
echo y | setup-alpine -ef answer
poweroff
