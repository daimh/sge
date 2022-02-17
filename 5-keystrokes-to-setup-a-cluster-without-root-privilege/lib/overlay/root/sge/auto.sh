#!/usr/bin/env bash
set -eux
set -o pipefail
if grep -q " sge-" /proc/cmdline
then
	Node=$(sed -e "s/ /\n/g" /proc/cmdline | grep "^sge-" | cut -d = -f 2)
	Step=$(sed -e "s/ /\n/g" /proc/cmdline | grep "^sge-" | cut -d = -f 1 | cut -d - -f 2)
	cd /root/sge/$Node
	./$Step.sh
fi
