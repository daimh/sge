PKGVER=$(strip $(shell sed -n '/^pkgver()/,/^}/p' PKGBUILD.in | grep -w git | sh ) )

build/source/3rdparty/qtcsh/LINUXAMD64/tcsh :
	install -d build
	cp -pr source build
	echo cd build > compile.sh
	sed -n '/^build()/,/^}/p' PKGBUILD.in | sed -e "s/\$$pkgver/$(PKGVER)/" >> compile.sh
	echo build >> compile.sh
	sh compile.sh
install : build/source/LINUXAMD64/sge_qmaster
	sed -n '/^pre_install()/,/}/p; /^post_install()/,/}/p' PKGBUILD.install > install.sh
	echo pre_install >> install.sh
	echo "install -d /opt/sge/build" >> install.sh
	echo "cp -pr build/source /opt/sge/build" >> install.sh
	echo post_install >> install.sh
	sh install.sh
clean : build/
	rm -rf build/
