[Unit]
Description=Grid Engine qmaster
After=network-online.target remote-fs.target autofs.service

[Service]
Type=forking
# Modify if the cell isn't "default"
#PIDFile=SGE_ROOT/default/spool/qmaster/qmaster.pid
ExecStart=SGE_ROOT/default/common/sgemaster
ExecStop=SGE_ROOT/default/common/sgemaster stop
#Restart=on-failure
#RestartSec=30s

[Install]
WantedBy=multi-user.target
