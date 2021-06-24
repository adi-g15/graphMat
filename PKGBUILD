# Maintainer: Aditya Gupta <ag15035 at gmail dot com>
pkgname=graphmat-git
pkgver=1
pkgrel=1
pkgdesc="A matrix header-only library, uses graphs internally"
arch=(x86_64)
url="https://github.com/adi-g15/graphMat"
license=('MIT')
depends=()
makedepends=('git' 'cmake')
provides=("${pkgname%-git}")
conflicts=("${pkgname%-git}")
source=(${pkgname%-git}::'git+https://github.com/adi-g15/graphMat')
md5sums=('SKIP')

pkgver() {
	cd "$srcdir/${pkgname%-git}"

	printf "%s" "$(git describe --long | sed 's/\([^-]*-\)g/r\1/;s/-/./g')"
}

build() {
	cd "$srcdir/${pkgname%-git}"
	cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
}

package() {
	cd "$srcdir/${pkgname%-git}"
	DESTDIR="$pkgdir/" cmake --install build
}
