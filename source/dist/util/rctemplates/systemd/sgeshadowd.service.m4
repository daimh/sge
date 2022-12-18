[Unit]
Description=Grid Engine shadow master
After=network-online.target network-fs.target

[Service]
Type=forking
EnvironmentFile=-/etc/sysconfig/sgemaster
# insist on setting ARCH in sysconfig instead of using shell?
ExecStart=/bin/sh -c SGE_ROOT/bin/$($SGE_ROOT/util/arch)/sge_shadowd

[Install]
WantedBy=multi-user.target
