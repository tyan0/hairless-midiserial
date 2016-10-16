#!/bin/bash
#
# Shell script to build Qt static, runs as part
# of the Travis continuous integration build.
#
# Variables
# - TRAVIS_BUILD_DIR
# - QTDIR - install pathfor Qt
# - QTURL - URL to Qt .tar.xz file
# - QTPKG - Name of directory unpacked by .tar.xz
# - TARGET - linux64, win32

set -e

cd ${TRAVIS_BUILD_DIR}
wget -q "${QTURL}"
tar -Jxf ${QTPKG}.tar.xz

QTCOMMONOPTS="-release -optimize-size -nomake examples -nomake tests -skip qtwebengine -opensource -skip qtconnectivity -static -no-feature-treewidget -no-feature-undoview -skip qt3d -skip qtactiveqt -skip qtandroidextras -skip qtcanvas3d -skip qtcharts -skip qtconnectivity -skip qtdatavis3d -skip qtdeclarative -skip qtdoc -skip qtgamepad -skip qtgraphicaleffects -skip qtimageformats -skip qtlocation -skip qtmultimedia -skip qtnetworkauth -skip qtpurchasing -skip qtquickcontrols -skip qtquickcontrols2 -skip qtremoteobjects -skip qtscript -skip qtscxml -skip qtsensors -skip qtserialbus -skip qtspeech -skip qtsvg -skip qttools -skip qttranslations -skip qtvirtualkeyboard -skip qtwayland -skip qtwebchannel -skip qtwebglplugin -skip qtwebsockets -skip qtwebview -skip qtxmlpatterns -no-feature-tuiotouch -no-feature-sqlmodel -no-feature-itemmodeltester -no-feature-cupsjobwidget -no-feature-linux-netlink -no-feature-http -no-feature-ftp -no-feature-udpsocket -no-feature-networkproxy -no-feature-socks5 -no-feature-networkdiskcache -no-feature-bearermanagement -no-feature-localserver -no-feature-dnslookup -no-feature-textodfwriter -no-feature-movie -no-feature-sql-ibase -no-feature-sql-mysql -no-feature-sql-oci -no-feature-sql-odbc -no-feature-sql-psql -no-feature-sql-sqlite2 -no-feature-sql-sqlite -no-feature-sql-tds -no-feature-dom -no-feature-vnc -no-feature-directfb -no-opengl -no-ssl -skip qtmacextras -confirm-license -prefix ${QTDIR}"

case $TARGET in
	linux64)
		QTOPTS="-no-feature-pdf -no-feature-printer"
		;;
    linux32)
        QTOPTS="-no-feature-pdf -no-feature-printer -sysroot / -xplatform linux-g++-32"
        ;;
	win32)
		QTOPTS="-no-feature-pdf -no-feature-printer -xplatform win32-g++ -device-option CROSS_COMPILE=i686-w64-mingw32-"
		;;
    macos)
        # macos includes PDF & Printer features to avoid errors compiling QMacPrintEngine
        QTOPTS=""
        ;;
    *)
        echo "Target not found: $TARGET"
        exit 1
        ;;
esac

echo "Applying Qt patches..."

cd ${TRAVIS_BUILD_DIR}/${QTPKG}

PATCHDIRS="${TRAVIS_BUILD_DIR}/ci_build/qt_patches/${TARGET}"
for PATCHDIR in $PATCHDIRS; do
    if [ -d ${PATCHDIR} ]; then
        for PATCH in ${PATCHDIR}/*.patch; do
            echo "Applying ${PATCH}..."
            patch -p1 < ${PATCH}
        done
    fi
done

echo "Configure arguments ${QTCOMMONOPTS} ${QTOPTS}"

mkdir ${TRAVIS_BUILD_DIR}/qt_build
cd ${TRAVIS_BUILD_DIR}/qt_build

# tail config.log on failure, as most of the details are there
${TRAVIS_BUILD_DIR}/${QTPKG}/configure ${QTCOMMONOPTS} ${QTOPTS} || (tail -n 100 config.log && exit 1)

# Avoid log limits by at most outputting the last N lines of build on a failure
echo "Building Qt silently, will print error on failure"
( make install 2>&1 > /tmp/qtbuild.log ) || (tail -n 100 /tmp/qtbuild.log && exit 1)
