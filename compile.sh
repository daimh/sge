cd build
build() {
	cd source
	install -d MANSBUILD_sge/SEDMAN/man/man1
	install -d MANSBUILD_sge/SEDMAN/man/man3
	install -d MANSBUILD_sge/SEDMAN/man/man5
	install -d MANSBUILD_sge/SEDMAN/man/man8
	sh scripts/bootstrap.sh -no-java -no-jni
	./aimk -DGIT_REPO_VERSION=\\\"r37.97c4af9.dirty\\\" -no-java -no-jni -with-arch-linux -systemd
	install -d clients/gui-installer/dist
	touch clients/gui-installer/dist/installer.jar
	rm -f LINUXAMD64/config.status
	rm -f 3rdparty/qtcsh/LINUXAMD64/config.status
	rm -f 3rdparty/qtcsh/LINUXAMD64/atconfig
	rm -f 3rdparty/qmake/LINUXAMD64/config/Makefile
	rm -f 3rdparty/qmake/LINUXAMD64/w32/Makefile
	rm -f 3rdparty/qmake/LINUXAMD64/Makefile
	rm -f 3rdparty/qmake/LINUXAMD64/config.log
	rm -f 3rdparty/qmake/LINUXAMD64/config.status
	rm -f 3rdparty/qmake/LINUXAMD64/glob/Makefile
}
build
