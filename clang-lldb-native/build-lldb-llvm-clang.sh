#!/bin/sh -e

set -x
WHAT=$1

if [ X$WHAT = X ]; then
	echo "What you want to build lldb or clang ??"
	exit 1
fi

# On Ubuntu
# sudo apt-get install libedit-dev swig ncurses-dev libxml2-dev libeditline-dev

if [ `uname` = Linux ]; then
	MAKE=make
	LLVM_VERSION=3.7
else
	MAKE=gmake
	LLVM_VERSION=3.9
fi

RELEASE_TAG="RELEASE_400/rc2"

svn_exe=`which svn`
if [ X$svn_exe = X ]; then
	echo "svn not found"
	exit 1
fi

# http://llvm.org/svn/llvm-project/llvm/tags/
get_revision() {
	repo=$1
	tag=$2
	revision=`$svn_exe log http://llvm.org/svn/llvm-project/$repo/tags/$tag -v --stop-on-copy | head -2 | tail -1 | tr '|' '\n' | head -1 | sed s/r//g`
	echo $revision
}

LLVM_REVISION=$(get_revision "llvm" "$RELEASE_TAG")
CLANG_REVISION=$(get_revision "cfe" "$RELEASE_TAG")
LLDB_REVISION=$(get_revision "lldb" "$RELEASE_TAG")

echo "Release TAG: $RELEASE_TAG"
echo "LLVM revision : $LLVM_REVISION"
echo "CLANG revision: $CLANG_REVISION"
echo "LLDB revision : $LLDB_REVISION"

if [ $WHAT = "clang" ]; then
	BUILD_TYPE="Release"
	LLVM_CONFIGURATION="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
else
	BUILD_TYPE="Debug"
	LLVM_CONFIGURATION="-DLLDB_EXPORT_ALL_SYMBOLS=1 -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
fi

if [ -d "/c" ]; then
	MOUNT="c"
elif [ -d "/n" ]; then
	MOUNT="n"
elif [ -d "/b" ]; then
	MOUNT="b"
fi

DISK="/${MOUNT}/syrajendra/${RELEASE_TAG}"

mkdir -p $DISK
if [ ! -d $DISK ]; then
	echo "No disk space found"
	exit 1
fi

ninja_exe=`which ninja`
if [ X$ninja_exe = X ]; then
	echo "ninja not found"
	exit 1
fi

cmake_exe=`which cmake`
if [ X$cmake_exe = X ]; then
	echo "cmake not found"
	exit 1
fi


echo "Disk space found : $DISK"

OS=`uname`
OS_ID=`/volume/hab/$OS/bin/os-id`
MACHINE=`uname -m`
export CC=/volume/hab/$OS/$OS_ID/$MACHINE/llvm/$LLVM_VERSION/current/bin/clang
export CXX=/volume/hab/$OS/$OS_ID/$MACHINE/llvm/$LLVM_VERSION/current/bin/clang++

SRC=$DISK/src
mkdir -p $SRC

cd $SRC
echo "Checkout llvm"
if [ ! -d $SRC/llvm ]; then
	svn co -q -r $LLVM_REVISION http://llvm.org/svn/llvm-project/llvm/trunk llvm
fi

if [ ! -d $SRC/llvm/tools ]; then
	echo "LLVM checkout failed"
	exit 1
fi

echo "Checkout clang"
cd $SRC/llvm/tools/
if [ ! -d clang ]; then
	svn co -q -r $CLANG_REVISION http://llvm.org/svn/llvm-project/cfe/trunk clang
fi
cd -

if [ ! -d $SRC/llvm/tools/clang ]; then
	echo "Clang checkout failed"
	exit 1
fi

echo "Checkout lldb"
if [ ! -d $SRC/lldb ]; then
	svn co -q -r $LLDB_REVISION http://llvm.org/svn/llvm-project/lldb/trunk lldb
fi

if [ ! -d $SRC/lldb ]; then
	echo "Clang checkout failed"
	exit 1
fi

cd $SRC/llvm/tools/clang
cd $DISK
install_dir=$PWD/install/${BUILD_TYPE}/${WHAT}
build=$PWD/build/${BUILD_TYPE}/${WHAT}
mkdir -p $build $install_dir

cd $build
if [ $WHAT = "clang" ]; then
	cmake -G "Unix Makefiles" $SRC/llvm -DCMAKE_INSTALL_PREFIX=$install_dir $LLVM_CONFIGURATION
	$MAKE -j 16
	$MAKE install
else
	new_src=bundle
	mkdir -p $new_src
	ln -sf $SRC/llvm     $new_src/.
	ln -sf $SRC.lldb     $new_src/llvm/tools/.
	ln -sf $SRC/llvm/tools/clang $SRC/lldb/../. # lldb hard coded relative path of clang include
	SOURCE_NEW=$PWD/$new_src # overwrite this to pick dummy bundle
	cmake -G Ninja $SOURCE_NEW/llvm -DCMAKE_INSTALL_PREFIX=$install_dir $LLVM_CONFIGURATION
	ninja lldb
	ninja install
fi


