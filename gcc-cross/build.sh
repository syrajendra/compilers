#!/bin/sh -e

TOP=$PWD

TAR_DIR=$TOP/tar
BUILD_DIR=$TOP/build
INSTALL_DIR=$TOP/install
SRC_DIR=$TOP/src

SYSROOT=/home/syrajendra/Rajendra/compiler/sysroot

mkdir -p $TAR_DIR $BUILD_DIR $INSTALL_DIR $SRC_DIR

GCC_VERSION=10.2.0
BINUTILS_VERSION=2.35
MPC_VERSION=1.2.0
GMP_VERSION=6.2.0
MPFR_VERSION=4.1.0
GLIBC_VERSION=2.32

cd $SRC_DIR
COMP=gmp-${GMP_VERSION}
if [ ! -d $SRC_DIR/${COMP} ]; then
    cd $TAR_DIR
    if [ ! -f ${COMP}.tar.xz ]; then
        wget https://ftp.gnu.org/gnu/gmp/${COMP}.tar.xz
        tar -xvf ${COMP}.tar.xz
        mv ${COMP} $SRC_DIR/
    fi
    cd -
fi

cd $SRC_DIR
COMP=mpfr-${MPFR_VERSION}
if [ ! -d $SRC_DIR/${COMP} ]; then
    cd $TAR_DIR
    if [ ! -f ${COMP}.tar.xz ]; then
        wget https://ftp.gnu.org/gnu/mpfr/mpfr-${MPFR_VERSION}.tar.xz
        tar -xvf ${COMP}.tar.xz
        mv ${COMP} $SRC_DIR/
    fi
    cd -
fi

cd $SRC_DIR
COMP=mpc-${MPC_VERSION}
if [ ! -d $SRC_DIR/${COMP} ]; then
    cd $TAR_DIR
    wget https://ftp.gnu.org/gnu/mpc/${COMP}.tar.gz
    tar -xzvf ${COMP}.tar.gz
    mv ${COMP} $SRC_DIR/
    cd -
fi

cd $SRC_DIR
COMP=mpc-${MPC_VERSION}
if [ ! -d $SRC_DIR/${COMP} ]; then
    cd $TAR_DIR
    wget https://ftp.gnu.org/gnu/mpc/${COMP}.tar.gz
    tar -xzvf ${COMP}.tar.gz
    mv ${COMP} $SRC_DIR/
    cd -
fi

cd $SRC_DIR
COMP=binutils-${BINUTILS_VERSION}
if [ ! -d $SRC_DIR/${COMP} ]; then
    cd $TAR_DIR
    if [ ! -f ${COMP}.tar.xz ]; then
        wget https://ftp.gnu.org/gnu/binutils/${COMP}.tar.xz
        tar -xvf ${COMP}.tar.xz
        mv ${COMP} $SRC_DIR/
    fi
    cd -
fi

cd $SRC_DIR
COMP=gcc-${GCC_VERSION}
if [ ! -d $SRC_DIR/${COMP} ]; then
    cd $TAR_DIR
    if [ ! -f ${COMP}.tar.xz ]; then
        wget https://ftp.gnu.org/gnu/gcc/${COMP}/${COMP}.tar.xz
        tar -xvf ${COMP}.tar.xz
        mv ${COMP} $SRC_DIR/
    fi
    cd -
fi

cd $SRC_DIR
COMP=glibc-${GLIBC_VERSION}
if [ ! -d $SRC_DIR/${COMP} ]; then
    cd $TAR_DIR
    if [ ! -f ${COMP}.tar.xz ]; then
        wget https://ftp.gnu.org/gnu/glibc/glibc-${GLIBC_VERSION}.tar.xz
        tar -xvf ${COMP}.tar.xz
        mv ${COMP} $SRC_DIR/
    fi
    cd -
fi

if [ ! -f $INSTALL_DIR/lib/libgmp.a ]; then
COMP=gmp-${GMP_VERSION}
COMP_SRC=$SRC_DIR/$COMP
mkdir -p $BUILD_DIR/$COMP
cd $BUILD_DIR/$COMP
$COMP_SRC/configure --disable-shared --prefix=$INSTALL_DIR
make -j4
make install
fi

if [ ! -f $INSTALL_DIR/lib/libmpfr.a ]; then
COMP=mpfr-${MPFR_VERSION}
COMP_SRC=$SRC_DIR/$COMP
mkdir -p $BUILD_DIR/$COMP
cd $BUILD_DIR/$COMP
$COMP_SRC/configure --disable-shared --prefix=$INSTALL_DIR --with-gmp=$INSTALL_DIR
make -j4
make install
fi

if [ ! -f $INSTALL_DIR/lib/libmpc.a ]; then
COMP=mpc-${MPC_VERSION}
COMP_SRC=$SRC_DIR/$COMP
mkdir -p $BUILD_DIR/$COMP
cd $BUILD_DIR/$COMP
$COMP_SRC/configure \
--disable-shared \
--prefix=$INSTALL_DIR \
--with-gmp=$INSTALL_DIR \
--with-mpfr=$INSTALL_DIR
make -j4
make install
fi

TARGETS="x86_64-unknown-freebsd12.1 x86_64-linux-gnu"
TARGET="x86_64-linux-gnu"    

if [ ! -f $INSTALL_DIR/$TARGET/bin/ld ]; then
COMP=binutils-${BINUTILS_VERSION}
COMP_SRC=$SRC_DIR/$COMP
mkdir -p $BUILD_DIR/$COMP/$TARGET
cd $BUILD_DIR/$COMP/$TARGET
$COMP_SRC/configure \
--prefix=$INSTALL_DIR  \
--disable-build-warnings \
--enable-install-bfd \
--disable-iconv \
--disable-nls \
--with-sysroot=yes \
--target=$TARGET
make -j4
make install
fi

if [ ! -f $INSTALL_DIR/bin/${TARGET}-gcc ]; then
COMP=gcc-${GCC_VERSION}
COMP_SRC=$SRC_DIR/$COMP
mkdir -p $BUILD_DIR/$COMP/$TARGET
cd $BUILD_DIR/$COMP/$TARGET
$COMP_SRC/configure \
--prefix=$INSTALL_DIR  \
--target=$TARGET \
--disable-nls \
--disable-vtable-verify \
--enable-plugin \
--with-system-zlib \
--enable-languages=c,c++ \
--disable-build-warnings \
--disable-libsanitizer \
--enable-checking=release \
--enable-multilib \
--enable-tls \
--enable-threads \
--with-gmp=$INSTALL_DIR \
--with-mpfr=$INSTALL_DIR \
--with-mpc=$INSTALL_DIR \
--with-ld=$INSTALL_DIR/$TARGET/bin/ld \
--with-as=$INSTALL_DIR/$TARGET/bin/as \
--with-ar=$INSTALL_DIR/$TARGET/bin/ar \
--with-nm=$INSTALL_DIR/$TARGET/bin/nm \
--with-ranlib=$INSTALL_DIR/$TARGET/bin/ranlib \
--with-objdump=$INSTALL_DIR/$TARGET/bin/objdump \
--with-strip=$INSTALL_DIR/$TARGET/bin/strip
make -j4 all-gcc
make install-gcc
fi

case $TARGET in
*linux*)
# Not supported on FreeBSD
COMP=glibc-${GLIBC_VERSION}
COMP_SRC=$SRC_DIR/$COMP
mkdir -p $BUILD_DIR/$COMP/$TARGET
cd $BUILD_DIR/$COMP/$TARGET
$COMP_SRC/configure \
--prefix=$INSTALL_DIR  \
--target=$TARGET \
--host=$TARGET \
--with-headers=$SYSROOT/$TARGET/usr/include \
libc_cv_forced_unwind=yes
make install-bootstrap-headers=yes install-headers
make -j4 csu/subdir_lib
install csu/crt1.o csu/crti.o csu/crtn.o $INSTALL_DIR/$TARGET/lib
$INSTALL_DIR/$TARGET-gcc -nostdlib -nostartfiles -shared -x c /dev/null -o $INSTALL_DIR/$TARGET/lib/libc.so

# support library build
cd $BUILD_DIR/gcc-${GCC_VERSION}/$TARGET
make -j4 all-target-libgcc
make install-target-libgcc

# c library build
cd $BUILD_DIR/glibc-${GLIBC_VERSION}/$TARGET
make -j4
make install

# c++ library build
cd $BUILD_DIR/gcc-${GCC_VERSION}/$TARGET
make -j4 
make install

;;
*freebsd*)
echo "glibc skipped"
;;
esac

