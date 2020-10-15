all: pam_sge-qrsh-setup$(SHAREDEXT)

pam_sge-qrsh-setup$(SHAREDEXT): ../3rdparty/sge-sshd-control/pam_sge-qrsh-setup.c
	$(CC) $(CFLAGS) $(SHARED_CFLAGS) $(SHARED_LFLAGS) -o $@ $<

clean:
	$(RM) pam_sge-qrsh-setup.so

depend:
