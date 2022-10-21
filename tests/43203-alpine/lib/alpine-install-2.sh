set -fuEexo pipefail
echo -e "http://dl-cdn.alpinelinux.org/alpine/edge/main\nhttp://dl-cdn.alpinelinux.org/alpine/edge/community" > /etc/apk/repositories
apk update
apk upgrade
apk add rsync bash
adduser -D -s /bin/bash sge
echo -e "10.0.2.15\tsge-alpine" >> /etc/hosts
echo -e "SomeGridEngine\nSomeGridEngine" | passwd
sed -ie "s/# LBU_BACKUPDIR=/LBU_BACKUPDIR=/" /etc/lbu/lbu.conf
mkdir /root/config-backups
lbu ci
poweroff
