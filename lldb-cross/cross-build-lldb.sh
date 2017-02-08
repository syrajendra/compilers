#!/bin/sh -e

#set -x

if [ `uname` != FreeBSD ]; then
  echo "Supported on FreeBSD only"
  exit 1
fi

CLANGPATH="/volume/hab/FreeBSD/10/amd64/llvm/3.9/current"
CLANG_TABLGEN_PATH="/c/syrajendra/release_400/build/Release/clang/bin"
LLVM_TABLGEN_PATH="/c/syrajendra/release_400/install/clang/bin"

CUR_DIR=$PWD
LLVM_SRC="/c/syrajendra/release_400/src/llvm"
CLANG_SRC="$LLVM_SRC/tools/clang"
LLDB_SRC="/c/syrajendra/release_400/src/lldb"
LLDB_INSTALL=$CUR_DIR/install
LLDB_BUILD=$CUR_DIR/build
SYSROOT_TOP=${CUR_DIR}/sysroot

BUILD_TYPE=Debug # Release
DISABLE_PYTHON=False # True
EXPORT_SYMBOLS="-DLLDB_EXPORT_ALL_SYMBOLS=1"

LLDB_TARGETS_11="armv6--freebsd11.0-gnueabihf i386-unknown-freebsd11.0 amd64-unknown-freebsd11.0"
LLDB_TARGETS="$LLDB_TARGETS_11"

contains() {
	pattern="$1"
	input="$2"
	echo "$input" | grep -q "$pattern"
}

LLDB_VERSION=5.0.0

for TARGET_TRIPLE in $LLDB_TARGETS; do # loop for all supported boards

	if contains "amd64" "$TARGET_TRIPLE" ; then
		TARGET_ARCH=X86
	elif contains "i386" "$TARGET_TRIPLE" ; then
		TARGET_ARCH=X86
	elif contains "arm" "$TARGET_TRIPLE" ; then
		TARGET_ARCH=ARM
		TARGET_HARDFLOAT_OPTION="-mfloat-abi=hard"
	else
		echo "Target triple not known $TARGET_TRIPLE"
		exit 1
	fi

	SYSROOT=${SYSROOT_TOP}/$TARGET_TRIPLE

	if [ ! -d $SYSROOT ]; then
		echo "Sysroot for $TARGET_TRIPLE not found at '$SYSROOT'"
		exit 1
	fi

	PREFIX="${LLDB_INSTALL}/${TARGET_TRIPLE}"
	PYTHON_PKG=${SYSROOT}/usr/local

	# Python requirement
	if [ $DISABLE_PYTHON = "False" ]; then
		# On target set PYTHONHOME
		RELOCATABLE_PYTHON="-DLLDB_RELOCATABLE_PYTHON=True"
		# Python includes
		PINCLUDES="$PYTHON_PKG/include/python2.7"
		PYTHON_INCLUDE_DIR="-DPYTHON_INCLUDE_DIR=$PINCLUDES"
		if [ ! -d $PINCLUDES ]; then
			echo "Need python includes @ $PINCLUDES"
			exit 1
		fi
		# Python lib
		PLIB="$PYTHON_PKG/lib/libpython2.7.so"
		PYTHON_LIBRARY="-DPYTHON_LIBRARY=$PLIB"
		if [ ! -f $PLIB ]; then
			echo "Need python.so library @ $PLIB"
			exit 1
		else
			mkdir -p ${PREFIX}/lib
    		ln -sf $PYTHON_PKG/lib/libpython2.7* ${PREFIX}/lib/.
		fi
	fi

	LIBXML2_INCLUDES="$SYSROOT/usr/local/include/libxml2"
	if [ ! -d $LIBXML2_INCLUDES ]; then
		echo "Need libxml2 includes @ $LIBXML2_INCLUDES"
		exit 1
	fi

	LIBXML2_LIB="$SYSROOT/usr/local/lib/libxml2.so"
	if [ ! -f $LIBXML2_LIB ]; then
		echo "Need libxml2.so library @ $LIBXML2_LIB"
		exit 1
	fi

	if [ `uname` = FreeBSD ]; then
		if [ ! -x $CLANGPATH/bin/clang ]; then
			echo "Set CLANGPATH environment variable"
			exit 1
		fi
		if [ ! -x $CLANGPATH/bin/clang++ ]; then
			echo "Set CLANGPATH environment variable"
			exit 1
		fi

		if [ ! -x ${LLVM_TABLGEN_PATH}/llvm-tblgen ]; then
			echo "Need 'llvm-tblgen' tool set LLVM_TABLGEN_PATH environment variable"
			exit 1
		fi

		# 'clang-tablgen' tool is not in install location so we pick it from some location
		if [ ! -x ${CLANG_TABLGEN_PATH}/clang-tblgen ]; then
			echo "Need 'clang-tblgen' tool set CLANG_TABLGEN_PATH environment variable"
			exit 1
		fi

		TARGET_TOOLS="$CLANGPATH/bin/${TARGET_TRIPLE}"
		CROSS_FLAGS="--target=${TARGET_TRIPLE} --sysroot=$SYSROOT"
		export CC="${CLANGPATH}/bin/clang ${CROSS_FLAGS}"
		export CXX="${CLANGPATH}/bin/clang++ ${CROSS_FLAGS}"
		export CFLAGS="$TARGET_HARDFLOAT_OPTION"
		export CXXFLAGS="$TARGET_HARDFLOAT_OPTION"
		export LDFLAGS="-L${SYSROOT}/usr/local/lib"
	else
		echo "Host $OS  does not support lldb cross build"
		exit 1
	fi

	echo "void main() {}" > hello.c
	${CC} -v hello.c
	rm hello.c
	if [ ! -f a.out ]; then
		echo "Failed for $TARGET_TRIPLE!"
		exit 1
	fi
	rm a.out

	build=$LLDB_BUILD/${BUILD_TYPE}/${TARGET_TRIPLE}
	src=bundle

	if [ -d $build ]; then
		mv  $build bca-del
		rm -rf bca-del &
	fi

	mkdir -p $build
	cd $build
	if [ -d $LLDB_SRC ] && [ -d $LLVM_SRC ]; then
		rm -rf *
		# For build to succeed we have to bundle llvm, clang & lldb in to one package
		# Below we are creating a dummy bundle
		mkdir $src
		ln -sf $LLVM_SRC     $src/.
		ln -sf $LLDB_SRC     $src/llvm/tools/.
		ln -sf $LLVM_SRC/tools/clang $LLDB_SRC/../. # lldb hard coded relative path of clang include
		SOURCE_NEW=$PWD/$src # overwrite this to pick dummy bundle

		# Why -fPIC option required to fix below error on amd64 it because we are trying to link static lib with shared lib
		# But the static lib is not compiled with -fPIC
		# for relocation R_X86_64_32S against `.rodata' can not be used when making a shared object; recompile with -fPIC
		cmake $SOURCE_NEW/llvm \
	  	 -G Ninja \
	 	 -DCMAKE_SYSTEM_LIBRARY_PATH="${SYSROOT}/usr/lib" \
	 	 -DLLVM_PARALLEL_LINK_JOBS=1 \
	 	 -DCMAKE_LINKER=${TARGET_TOOLS}-ld \
	 	 -DCMAKE_AR=${TARGET_TOOLS}-ar \
	 	 -DCMAKE_NM=${TARGET_TOOLS}-nm \
	 	 -DCMAKE_OBJCOPY=${TARGET_TOOLS}-objcopy \
	 	 -DCMAKE_OBJDUMP=${TARGET_TOOLS}-objdump \
	 	 -DCMAKE_STRIP=${TARGET_TOOLS}-strip \
	 	 -DCMAKE_RANLIB=${TARGET_TOOLS}-ranlib \
	 	 -DCMAKE_INSTALL_PREFIX=${PREFIX} \
	 	 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
	 	 -DCMAKE_CROSSCOMPILING=True \
	 	 -DLLVM_TABLEGEN=${LLVM_TABLGEN_PATH}/llvm-tblgen \
	 	 -DCLANG_TABLEGEN=${CLANG_TABLGEN_PATH}/clang-tblgen \
	 	 -DLLVM_DEFAULT_TARGET_TRIPLE=${TARGET_TRIPLE} \
	 	 -DLLVM_HOST_TRIPLE=${TARGET_TRIPLE} \
	 	 -DLLVM_TARGET_ARCH=${TARGET_ARCH} \
	 	 -DLLVM_TARGETS_TO_BUILD=${TARGET_ARCH} \
	 	 -DLLDB_DISABLE_PYTHON=${DISABLE_PYTHON} \
	 	 ${RELOCATABLE_PYTHON} \
	 	 ${PYTHON_INCLUDE_DIR} \
	 	 ${PYTHON_LIBRARY} \
	 	 -DLLDB_DISABLE_LIBEDIT=False \
	 	 -DLLDB_DISABLE_CURSES=False \
	 	 -DLLVM_ENABLE_TERMINFO=False \
	 	 -DLLVM_ENABLE_LIBCXX=True \
	 	 -DLIBXML2_INCLUDE_DIR=${LIBXML2_INCLUDES} \
	 	 -DLIBXML2_LIBRARIES=${LIBXML2_LIB} \
	 	 -DLLDB_CROSSCOMPILING=True \
	 	 -DLLDB_SYSROOT="${SYSROOT}" \
	 	 -DBacktrace_LIBRARY="${SYSROOT}/usr/lib/libexecinfo.so" \
	 	 -DCMAKE_C_FLAGS="-fPIC" \
	 	 -DCMAKE_CXX_FLAGS="-fPIC" \
	 	 $EXPORT_SYMBOLS \
	 	 2>&1 | tee $build/configure.log

		# test configure
		#exit 1
		if [ -f build.ninja ]; then
			# Just build
			build_log=${build}/build.log
			ninja -v lldb 2>&1 | tee $build_log
			if [ $? != 0 ]; then
				echo "LLDB ${TARGET_TRIPLE} build failed. Check build log $build_log"
				exit 1
			fi
			ninja install 2>&1 | tee ${build}/install.log
		else
			echo "BUILD_ERROR: Problem with configure"
			exit 1
		fi
		echo "BUILD lldb cross ${TARGET_ARCH} OK"
	else
		echo "llvm or lldb source missing while cross building for $TARGET_TRIPLE"
		exit 1
	fi
	exit 1
done

