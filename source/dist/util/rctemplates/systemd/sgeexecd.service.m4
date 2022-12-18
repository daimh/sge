[Unit]
Description=Grid Engine execd
After=network-online.target remote-fs.target

[Service]
Type=forking
ExecStart=SGE_ROOT/default/common/sgeexecd
ExecStop=SGE_ROOT/default/common/sgeexecd stop
#Restart=on-failure
#RestartSec=30s

[Install]
WantedBy=multi-user.target
