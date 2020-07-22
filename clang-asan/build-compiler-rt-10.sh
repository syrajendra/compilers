#!/bin/sh

SCRIPT_PATH=`dirname $0`
DATE=`date "+%Y%m%d"`
OS=`uname -s`
OS_ID="Ubuntu-20.04"
MACHINE=`uname -m`
TOP=$PWD
SRC=$(realpath $SCRIPT_PATH)
TINSTALL=$TOP/install
TBUILD=$TOP/build
rm -rf $TBUILD
mkdir -p $TINSTALL $TBUILD

TARGETS="aarch64-unknown-freebsd13.0 armv6-unknown-freebsd13.0-gnueabi armv6-unknown-freebsd13.0-gnueabihf"
#TARGETS="armv6-unknown-freebsd13.0-gnueabihf"
TARGETS="armv6-unknown-freebsd13.0-gnueabi"
TARGETS="aarch64-unknown-freebsd13.0"
CLANG_COMP_DIR=/home/syrajendra/Rajendra/compiler/llvm-10/install
COMPILER_DIR=/home/syrajendra

for TARGET in $TARGETS; do
	ARCH=`echo $TARGET | sed s/-.*//g`
	if [ $ARCH = "armv6" ]; then
		ARCH=arm
	elif [ $ARCH = "aarch64" ]; then
		ARCH=arm64
		ARCH_LDFLAGS="-lpthread -lm -lc"
	fi
    BUILD=$TBUILD/$TARGET
    INSTALL=$TINSTALL/$TARGET
    SYSROOT=/home/syrajendra/Rajendra/compiler/sysroot/$TARGET
    EXTRA_LDFLAGS="$ARCH_LDFLAGS"
    export CC="$COMPILER_DIR/bin/${TARGET}-clang"
    export CXX="$COMPILER_DIR/bin/${TARGET}-clang++"
    export RANLIB=$CLANG_COMP_DIR/bin/llvm-ranlib
    export AR=$CLANG_COMP_DIR/bin/llvm-ar
    export NM=$CLANG_COMP_DIR/bin/llvm-nm
    export LD=$CLANG_COMP_DIR/bin/ld.lld
    export OBJCOPY=$CLANG_COMP_DIR/bin/llvm-objcopy
    export OBJDUMP=$CLANG_COMP_DIR/bin/llvm-objdump
    export STRIP=$CLANG_COMP_DIR/bin/llvm-strip
    export LDFLAGS="${EXTRA_LDFLAGS}"
    export CFLAGS="-I${SYSROOT}/usr/include"
    export CXXFLAGS="-I${SYSROOT}/usr/include"
    # 
    rm -rf $BUILD
	mkdir -p $BUILD $INSTALL
	cd $BUILD
	# -Wno-dev --debug-output --trace-expand
	cmake $SRC/compiler-rt \
	-G Ninja \
	-DCMAKE_BUILD_TYPE='Debug' \
	-DCMAKE_C_COMPILER_WORKS=1 \
	-DCMAKE_CXX_COMPILER_WORKS=1 \
	-DCMAKE_C_COMPILER=${CC} \
	-DCMAKE_CXX_COMPILER=${CXX} \
	-DCMAKE_C_FLAGS="${CFLAGS}" \
	-DCMAKE_CXX_FLAGS="${CXXFLAGS} -I${SYSROOT}/usr/include/c++/v1" \
	-DCMAKE_SYSROOT=${SYSROOT} \
	-DCMAKE_LINKER=${LD} \
	-DCMAKE_AR=${AR} \
	-DCMAKE_NM=${NM} \
	-DCMAKE_OBJCOPY=${OBJCOPY} \
	-DCMAKE_OBJDUMP=${OBJDUMP} \
	-DCMAKE_STRIP=${STRIP} \
	-DCMAKE_RANLIB=${RANLIB} \
	-DCMAKE_INSTALL_PREFIX=${INSTALL} \
	-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
	-DCMAKE_SHARED_LINKER_FLAGS="${LDFLAGS}" \
	-DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}" \
	-DCMAKE_CROSSCOMPILING=1 \
	-DCOMPILER_RT_BUILD_CRT=1 \
	-DCOMPILER_RT_OS_DIR=freebsd \
	-DCOMPILER_RT_BUILD_BUILTINS=1 \
	-DCOMPILER_RT_BUILD_SANITIZERS=1 \
	-DCOMPILER_RT_BUILD_XRAY=0 \
	-DCOMPILER_RT_BUILD_LIBFUZZER=0 \
	-DCOMPILER_RT_BUILD_PROFILE=0 \
	-DCOMPILER_RT_DEFAULT_TARGET_ARCH=$ARCH \
	-DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=$TARGET \
	-DCOMPILER_RT_DEFAULT_TARGET_ONLY=0 \
	-DLLVM_CONFIG_PATH=${CLANG_COMP_DIR}/bin/llvm-config
	#-DCOMPILER_RT_ASAN_SHADOW_SCALE=4
	ninja -v
	if [ $? != 0 ]; then
		echo "Build failed"
		exit 1
	else
		ninja install
	fi
	cd -
done