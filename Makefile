PACKAGE_PREFIX=mt5_extraction_tools
ZIP=zip
TAR=tar
SCP=scp
PACKAGE_DIR=../pkg/
all :
	make binaries && make packages
#==============================PACKAGES========================================
packages :
	make src-packages && make bin-packages
bin-packages :
	SYSTEM_TARGET="win32" make bin-pkg && \
	SYSTEM_TARGET="linux32" make bin-pkg
src-packages :
	PACKAGE="ypvr" make src-pkg && \
	PACKAGE="ymt5" make src-pkg && \
	PACKAGE="ymt5gui" make src-pkg
bin-pkg :
	COMPRESSION=zip make bin-pkg-compression && \
	COMPRESSION=tar_bz2 make bin-pkg-compression
src-pkg :
	COMPRESSION=zip make src-pkg-compression && \
	COMPRESSION=tar_bz2 make src-pkg-compression
bin-pkg-compression :
	INPUT_PATH=bin/${SYSTEM_TARGET} OUTPUT_PATH=${PACKAGE_DIR} \
	PACKAGE_NAME=${PACKAGE_PREFIX}-${SYSTEM_TARGET}-bin \
	make ${COMPRESSION}-pkg
src-pkg-compression : 
	INPUT_PATH=src/${PACKAGE} OUTPUT_PATH=${PACKAGE_DIR} \
	PACKAGE_NAME=${PACKAGE}-src \
	make ${COMPRESSION}-pkg
zip-pkg :
	${ZIP} -r ${PACKAGE_NAME} ${INPUT_PATH} && \
	mv ${PACKAGE_NAME}.zip ${OUTPUT_PATH}
tar_bz2-pkg :
	${TAR} cjf ${PACKAGE_NAME}.tar.bz2 ${INPUT_PATH} && \
	mv ${PACKAGE_NAME}.tar.bz2 ${OUTPUT_PATH}
#===============================BINARIES=======================================
binaries :
	make linux32-bin && make win32-bin
win32-bin :
	EXT=".exe" SYSTEM_TARGET=win32 CONFIGURE_OPTION="--host=i586-mingw32msvc" \
	LDFLAGS=-static-libgcc \
	make any-bins
linux32-bin :
	EXT="" SYSTEM_TARGET=linux32 make any-bins
any-bins :
	YTARGET="ypvr" make get-autotools-bin && \
	YTARGET="ymt5" make get-autotools-bin && \
	YTARGET="ymt5gui" make get-mono-exe
get-autotools-bin :
	cd src/${YTARGET} && \
	./configure ${CONFIGURE_OPTION} \
	&& make && cp src/${YTARGET}${EXT} \
	../../bin/${SYSTEM_TARGET}/ && make distclean
get-mono-exe :
	cp src/${YTARGET}/${YTARGET}/bin/Debug/${YTARGET}.exe \
	bin/${SYSTEM_TARGET}/ && if [ "${SYSTEM_TARGET}" = "win32" ]; then \
	echo "mono ${YTARGET}.exe" \
	> bin/${SYSTEM_TARGET}/${YTARGET}.bat; fi
