# Maintainer: Manhong Dai <daimh@umich.edu>
pkgname=sge
pkgver=20201015
pkgrel=1
pkgdesc="Son of Grid Engine/Sun Grid Engine"
arch=('x86_64')
url="https://github.com/daimh/sge"
license=('GPL')
depends=(
	'awk'
	'fakeroot'
	'file'
	'gcc'
	'grep'
	'hwloc'
	'inetutils'
	'libtirpc'
	'libxt'
	'make'
	'openmotif'
	'tcsh'
)
prepare() {
	cp -pr ../source .
}
build() {
	cd source
	install -d MANSBUILD_sge/SEDMAN/man/man1
	install -d MANSBUILD_sge/SEDMAN/man/man3
	install -d MANSBUILD_sge/SEDMAN/man/man5
	install -d MANSBUILD_sge/SEDMAN/man/man8
	sh scripts/bootstrap.sh -no-java -no-jni
	./aimk -no-java -no-jni -with-arch-linux
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
package() {
	install -d $pkgdir/opt/sge
	cp -pr ./ $pkgdir/opt/sge/build
}
install=sge.install
