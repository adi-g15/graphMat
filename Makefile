# NOTE: graphMat2D won't be installed now, you can do the same by just copying graphMat2D into /usr/include/

install:
	mkdir -p ${DESTDIR}/usr/include/graphMat/include
	install -Dm 644 include/graphMat/* -t ${DESTDIR}/usr/include/graphMat/include/

uninstall:
	rm -r ${DESTDIR}/usr/include/graphMat
