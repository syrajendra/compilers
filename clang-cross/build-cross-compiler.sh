#!/bin/sh

VERSION="1.0"
TOP=$PWD
PLATFORM=`uname`
SRC=${TOP}/src
BUILD=${TOP}/build
PKG=${TOP}/pkg
LOG=${TOP}/log
# Save all sysroot's here
SYSROOT=${TOP}/sysroot

mkdir -p $PKG $SRC $LOG $BUILD

INSTALL=$PKG/$PLATFORM/install
PREINSTALL=$PKG/$PLATFORM/pre-install

# If installed ninja build system used to build llvm, clang & libs
NINJA=1

# For debug change below value to Debug
BUILD_TYPE=Release

# No rebuild every time
REBUILD=0

# Default use new gnu binutils
WHICH_BINUTILS="new_"

# HEAD build
REPOUPDATE=0

# Used to build libraries
SYSROOT_TARGETS=""

# Used to build binutils
TARGETS=""

###### DEPEDENT TOOLS CHECK #######
tool_check() {
	tool=$1
	mandatory=$2
	cmd=`which $tool`
	if [ "$cmd" = "" ]; then
		if [ "$mandatory" = "yes" ]; then
			echo "ERROR: No '$tool' command found on this system"
			exit 1
		else
			echo "WARN: No '$tool' command found on this system"
			if [ "ninja" = "$tool" ]; then
				NINJA=0
			fi
		fi
	fi
}

# 1. gmake
tool_check "gmake" "yes"
# 2. ninja
tool_check "ninja" "no"
# 3. cmake
tool_check "cmake" "yes"
# 4. wget
tool_check "wget" "yes"
# 5. git
tool_check "git" "yes"
# 6. runtest (Dejagnu testing for binutils)
#tool_check "runtest" "no"
#############################

# This script supports below builds
DEBUGGER="lldb"
TEST="test-suite"
EXTRA_TOOLS="clang-tools-extra"
LIBRARIES="compiler-rt libcxx libcxxabi libunwind"
EXTRA_LIBRARIES="libcxxrt"
BUILDS="binutils compiler library debugger test tools"

# https://sourceware.org/git/?p=binutils.git;a=summary
# GPLv2 version of binutils
BINUTILS_REPO="git://sourceware.org/git/binutils.git"
BINUTILS_PATCH="$PWD/0001-Added-FreeBSD-platform-support-in-configuration-file.patch"

# http://programmers.stackexchange.com/questions/218644/how-could-clang-release-under-bsd-license
NEW_BINUTILS_VERSION="2.25.1"
NEW_BINUTILS_REPO="https://ftp.gnu.org/gnu/binutils/binutils-${NEW_BINUTILS_VERSION}.tar.gz"


LLVM_MIRROR="https://github.com/llvm-mirror" 	# GIT repo

# Release builds
LLVM_RELEASE="http://llvm.org/releases"
LLVM_SUPPORTED_RELEASES="3.5.0 3.5.1 3.5.2 3.6.0 3.6.1 3.6.2 3.7.0 head"
#LLVM_SOURCES="llvm clang $LIBRARIES $EXTRA_TOOLS $DEBUGGER $TEST"
LLVM_SOURCES="llvm clang"
LLLVM_CXXRT="https://github.com/pathscale/libcxxrt/"

if [ "$PLATFORM" = "FreeBSD" ]; then
	CPUS=`sysctl hw.ncpu | sed s/.*://g | xargs`
	OS_RELEASE=`freebsd-version`
	# Clang cross compilers for below targets
	BINUTILS_TARGETS="x86_64-unknown-freebsd i386-unknown-freebsd armv6--freebsd-gnueabi powerpc-unknown-freebsd avr-unknown-none"
elif [ "$PLATFORM" = "Linux" ]; then
	CPUS=`cat /proc/cpuinfo | grep processor | wc -l | xargs`
	BINUTILS_TARGETS="x86_64-unknown-linux i386-unknown-linux armv6--linux-gnueabi powerpc-unknown-linux avr-unknown-none"
elif [ "$PLATFORM" = "Darwin" ]; then
	CPUS=`sysctl hw.ncpu | sed s/.*://g | xargs`
	BINUTILS_TARGETS="avr-unknown-none x86_64-unknown-darwin"
else
	echo "ERROR: Build host $PLATFORM not yet supported"
	exit 1
fi

run() {
	# No echo's/print's in this function because the last line echo act as a return for this function
	cmd=$1
	output=`$cmd 2>&1`
	if [ $? != 0 ]; then
		echo "ERROR: Failed to run '$cmd'"
		exit 1
	fi
	echo $output
}

execute() {
	cmd=$1
	log=$2
	echo $cmd >> $log
	echo "CMD: $cmd [LogFile: tail -f $log]"
	eval "$cmd >> $log 2>&1"
	if [ $? != 0 ]; then
		echo "ERROR: Failed to run command"
		exit 1
	fi
}

format_version() {
	ver=$1
	total=3 # major.minor.build
	len=`echo $ver |  tr '.' ' ' | wc -w | xargs`
	while [ $len -lt $total ]; do
		ver=${ver}.0
		len=$(($len+1))
	done
	echo $ver
}

check_version() {
	tool=$1
	min_version=$(format_version $2) # minimum tool version requirement
	cur_version=$(format_version $3) # current tool version
	i=1
	for tk1 in `echo ${cur_version} | tr '.' ' '`; do
		tk2=`echo ${min_version} | cut -d'.' -f$i`
		i=$(($i+1))
		if [ $tk1 -gt $tk2 ]; then
			break
		elif [ $tk1 -eq $tk2 ]; then
			continue
		else
			echo "Expecting ${tool} version >= ${min_version}"
			exit 1
		fi
	done
}

git_clone() {
	name=$1
	repo=$2
	if [ ! -d $name ]; then
		execute "git clone $repo" "$LOG/git-$name.log"
	else
		if [ $REPOUPDATE = 1 ]; then
			cd $name
			#XXX Caution - Any changes in local repo will be lost
			execute "git reset --hard HEAD" "$LOG/git-$name.log"
			execute "git pull" "$LOG/git-$name.log"
			cd -
		fi
	fi
}

git_patch() {
	name=$1
	patch=$2
	if [ ! -f $patch ]; then
		echo "ERROR: Binutils patch missing '$patch'"
		exit 1
	elsels
		cd $name
		#XXX Caution - Any changes in local repo will be lost
		execute "git reset --hard HEAD" "$LOG/git-$name.log"
		execute "git apply $patch" "$LOG/git-$name.log"
		cd -
	fi
}

get_new_binutils() {
	mkdir -p $SRC
	name=`basename $NEW_BINUTILS_REPO`
	reponame=`echo $name | sed s/.tar.gz//g | xargs`
	cd $SRC
	if [ ! -f $name ]; then
		execute "wget --no-check-certificate $NEW_BINUTILS_REPO" "$LOG/get-$reponame.log"
	fi
	if [ ! -d $reponame ]; then
		execute "tar -xzvf $name" "$LOG/get-$reponame.log"
		cd $reponame
		# Below modifies configure file to support freebsd
		execute "cp config.sub config-orginal.sub" "$LOG/get-$reponame.log"
		sed s/kfreebsd/freebsd/g config.sub > config-mod.sub
		execute "cp config-mod.sub config.sub" "$LOG/get-$reponame.log"
		cd -
	fi
}

check_compiler_exist() {
	cc=$1
	if [ ! -f $cc ]; then
		echo "ERROR: Compiler $cc does not exist. Build compiler before library"
		exit 1
	fi
	if [ ! -x $cc ]; then
		echo "ERROR: Compiler $cc is not executable. Build compiler correctly"
		exit 1
	fi
}

build_new_binutils() {
	arg=$1
	installpath=$2
	check_compiler_exist "$CC"
	check_compiler_exist "$CXX"
	pre=`basename $installpath | sed s/install//g | xargs`
	installpath="$installpath/$arg"
	mkdir -p $installpath
	name=`basename $NEW_BINUTILS_REPO`
	reponame=`echo $name | sed s/.tar.gz//g | xargs`
	cd $BUILD
	for TARGET in $TARGETS; do
		builddir="$BUILD/$PLATFORM/$reponame/$arg/${pre}build-${TARGET}"
		if [ -d $builddir ] && [ $REBUILD = 0 ]; then
			ld=$installpath/bin/${TARGET}-ld
			as=$installpath/bin/${TARGET}-as
			if [ -f $ld ] && [ -f $as ]; then
				echo "INFO: Binutils target '$TARGET' already built skipping. To rebuild use option -a|--action=clean-build"
				echo "INFO: Installed @ $installpath"
				continue
			fi
		fi
		rm -rf $builddir
		mkdir -p $builddir
		cd $builddir
		execute "$SRC/$reponame/configure --disable-build-warnings --with-sysroot=yes --target=$TARGET --prefix=${installpath} --disable-nls" "$LOG/configure-${pre}${reponame}-${TARGET}.log"
		execute "gmake -j $CPUS" "$LOG/build-${pre}${reponame}-${TARGET}.log"
		execute "gmake install" "$LOG/install-${pre}${reponame}-${TARGET}.log"
		cmd=`which runtest`
		if [ "$cmd" != "" ]; then
			execute "gmake check" "$LOG/check-${pre}${reponame}-${TARGET}.log"
		else
			echo "WARN: No 'runtest' command found to execute Dejagnu testing for binutils"
		fi
		cd -
	done
}

get_old_binutils() {
	reponame="binutils"
	mkdir -p $SRC
	cd $SRC
	git_clone "$reponame" "${BINUTILS_REPO}"
	git_patch "$reponame" "${BINUTILS_PATCH}"
}

build_old_binutils() {
	arg=$1
	installpath=$2
	check_compiler_exist "$CC"
	check_compiler_exist "$CXX"
	pre=`basename $installpath | sed s/install//g | xargs`
	installpath="$installpath/$arg"
	mkdir -p $installpath
	reponame="binutils"
	cd $BUILD
	for TARGET in $TARGETS; do
		builddir="$BUILD/$PLATFORM/$reponame/$arg/${pre}build-${TARGET}"
		if [ -d $builddir ] && [ $REBUILD = 0 ]; then
			ld=$installpath/bin/${TARGET}-ld
			as=$installpath/bin/${TARGET}-as
			if [ -f $ld ] && [ -f $as ]; then
				echo "INFO: Binutils target '$TARGET' already built skipping. To rebuild use option -a|--action=clean-build"
				echo "INFO: Installed @ $installpath"
				continue
			fi
		fi
		rm -rf $builddir
		mkdir -p $builddir
		cd $builddir
		execute "$SRC/$reponame/configure --disable-build-warnings --with-sysroot=yes --target=$TARGET --prefix=${installpath} --disable-nls" "$LOG/configure-${pre}${reponame}-${TARGET}.log"
		execute "gmake -j $CPUS" "$LOG/build-${pre}${reponame}-${TARGET}.log"
		execute "gmake install" "$LOG/install-${pre}${reponame}-${TARGET}.log"
		cmd=`which runtest`
		if [ "$cmd" != "" ]; then
			execute "gmake check" "$LOG/check-${pre}${reponame}-${TARGET}.log"
		else
			echo "WARN: No 'runtest' command found to execute Dejagnu testing for binutils"
		fi
		cd -
	done
}

get_no_binutils() {
	echo "--- No Binutils get ---"
}

build_no_binutils() {
	echo "--- No Binutils built ---"
}

link() {
	name=$1
	where=$2
	if [ -d $name ]; then
		if [ ! -d $where/$name ]; then
			cp -rf ./${name} ${where}/.
			#ln -sf ./${name} ${where}/.
		fi
	else
		echo "ERROR: LLVM project $name not present"
		exit 1
	fi
}

rearrange() {
	link clang		llvm/tools/
	# Building all in one shot does not work for all releases
	#link clang-tools-extra llvm/tools/clang/tools/
	#link compiler-rt 	llvm/projects/
	#link libcxx 		llvm/projects/
	# XXXX Build fail : ../../projects/libcxxabi/src/cxa_exception.hpp:19:10: fatal error: 'unwind.h' file not found
	#link libcxx 		llvm/projects/.
	#link libcxxunwind 	llvm/projects/.
}

download_llvm_release_src() {
	version=$1
	mkdir -p $SRC/release/$version
	cd $SRC/release/$version
	for d in $LLVM_SOURCES; do
		if [ "$d" = "clang" ]; then
			name=cfe-${version}.src.tar.xz
		else
			name=${d}-${version}.src.tar.xz
		fi
		if [ ! -f $name ]; then
			execute "wget ${LLVM_RELEASE}/${version}/$name" "$LOG/wget-$name.log"
		fi
		if [ ! -d $d ]; then
			folder=`echo $name | sed s/.tar.xz//g | xargs`
			if [ ! -d  $folder ]; then
				execute "tar -xJf $name" "$LOG/wget-$name.log"
			fi
			mv $folder $d
		fi
	done
	rearrange
	cd -
}

get_llvm_infra() {
	mkdir -p $SRC/head
	cd $SRC/head
	for s in $LLVM_SOURCES; do
		git_clone $s "${LLVM_MIRROR}/${s}.git"
	done
	rearrange
	cd -
}

no_build_llvm() {
	# No llvm is built here
	echo "--- No LLVM/Clang built ---"
}

debug_prints() {
	echo "CC: $CC"
	echo "CXX: $CXX"
	echo "CXXFLAGS: $CXXFLAGS"
	echo "LDFLAGS: $LDFLAGS"
	echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
}

fire_build_cmd() {
	pre=$1
	reponame=$2
	reposrc=$3
	cmake_opts=$4
	echo ""
	echo "--- Building $pre $reponame ---"
	echo "PWD: $PWD"
	#debug_prints
	if [ $NINJA = 1 ]; then
		execute "cmake $reposrc -G Ninja  $cmake_opts" "$LOG/ninja-configure-${pre}${reponame}.log"
		execute "ninja" "$LOG/ninja-build-${pre}${reponame}.log"
		execute "ninja install" "$LOG/ninja-install-${pre}${reponame}.log"
	else
		execute "cmake $reposrc -G \"Unix Makefiles\"  $cmake_opts" "$LOG/make-configure-${pre}${reponame}.log"
		execute "gmake -j $CPUS" "$LOG/make-build-${pre}${reponame}.log"
		execute "gmake install" "$LOG/make-install-${pre}${reponame}.log"
	fi
	echo "INFO: LLVM/Clang is built @ ${installpath}"
}

build_llvm() {
	arg=$1
	srcarg=`echo $arg | sed 's,^[^/]*/,,'`
	installpath=$2
	check_compiler_exist "$CC"
	check_compiler_exist "$CXX"
	reponame="llvm"
	pre=`basename $installpath | sed s/install//g | xargs`
	installpath="$installpath/$arg"
	builddir="$BUILD/$PLATFORM/$reponame/$arg/${pre}build"
	if [ -d $builddir ] && [ -f $installpath/bin/clang ] && [ -f $installpath/bin/clang++ ]; then
		if [ $REBUILD = "0" ]; then
			echo "INFO: LLVM/Clang is already built skipping. To rebuild specify option -a|--action=clean-build"
			echo "INFO: Installed @ $installpath"
			return
		fi
	fi

	if [ -d $builddir ]; then
		rm -rf $builddir
	fi
	mkdir -p $builddir
	cd $builddir
	reposrc="$SRC/$srcarg/$reponame"
	cmake_opts="-DCMAKE_INSTALL_PREFIX=${installpath} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ${LLVM_EXTRA_CONFIG}"
	fire_build_cmd "$pre" "$reponame" "$reposrc" "$cmake_opts"
	cd -
}

check_sysroot_exist() {
	sysroot=$1
	if [ ! -d $sysroot ]; then
		echo "ERROR: Sysroot $sysroot folder does not exist"
		exit 1
	fi
	SYSROOT_TARGETS=`cd $sysroot && find * -maxdepth 0 -type d | tr '\n' ' ' && cd -`
}

build_libcxxrt() {
	pre=$1
	reponame=$2
	reposrc=$3
	installpath=$4
	toprepo=`dirname $reposrc`
	cmake_opts="-DCMAKE_INSTALL_PREFIX=${installpath} \
				-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
				-DLLVM_PATH=${toprepo}/llvm \
				-DLLVM_CONFIG=${installpath}/bin/llvm-config \
				-DLIBCXXABI_LIBCXX_PATH=${toprepo}/libcxx \
				-DCMAKE_POLICY_DEFAULT_CMP0056=NEW "
	fire_build_cmd "$pre" "$reponame" "$reposrc" "$cmake_opts"
}


build_libcxxabi() {
	pre=$1
	reponame=$2
	reposrc=$3
	installpath=$4
	toprepo=`dirname $reposrc`
	cmake_opts="-DCMAKE_INSTALL_PREFIX=${installpath} \
				-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
				-DLLVM_PATH=${toprepo}/llvm \
				-DLLVM_CONFIG=${installpath}/bin/llvm-config \
				-DLIBCXXABI_LIBCXX_PATH=${toprepo}/libcxx"
	fire_build_cmd "$pre" "$reponame" "$reposrc" "$cmake_opts"
}

build_libcxx() {
	pre=$1
	reponame=$2
	reposrc=$3
	installpath=$4
	toprepo=`dirname $reposrc`
	cmake_opts="-DCMAKE_INSTALL_PREFIX=${installpath} \
				-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
				-DLLVM_PATH=${toprepo}/llvm \
				-DLIBCXX_CXX_ABI=libcxxabi \
				-DLIBCXX_CXX_ABI_INCLUDE_PATHS=${toprepo}/libcxxabi/include"
	fire_build_cmd "$pre" "$reponame" "$reposrc" "$cmake_opts"
}

build_compiler_rt() {
	pre=$1
	reponame=$2
	reposrc=$3
	installpath=$4
	cmake_opts="-DCMAKE_INSTALL_PREFIX=${installpath} \
				-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
				-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
				-DCOMPILER_RT_INSTALL_PATH=${installpath}/lib/clang/ \
				-DLLVM_CONFIG=${installpath}/bin/llvm-config \
				-DLLVM_CONFIG_PATH=${installpath}/bin/llvm-config"
	fire_build_cmd "$pre" "$reponame" "$reposrc" "$cmake_opts"
}

trigger_library_download() {
	revision=$1
	libname=$2
	LLVM_SOURCES="$LLVM_SOURCES $libname"
	if [ $revision = "head" ]; then
		get_llvm_infra
	else
		download_llvm_release_src $revision
	fi
}

download_libcxxrt() {
	revision=$1
	echo "ERROR: Implement this download_libcxxrt function"
	exit 1
}

build_library() {
	arg=$1
	revision=`basename $arg`
	installpath=$2
	reponame=$3
	trigger_library_download "$revision" "$reponame"
	srcarg=`echo $arg | sed 's,^[^/]*/,,'`
	pre=`basename $installpath | sed s/install//g | xargs`
	installpath=${installpath}/$arg
	check_compiler_exist "$CC"
	check_compiler_exist "$CXX"
	builddir="$BUILD/$PLATFORM/$reponame/$arg/${pre}build"
	if [ -d $builddir ] && [ $REBUILD = "1" ]; then
		rm -rf $builddir
	elif [ -d $builddir ] && [ $REBUILD = "0" ]; then
		echo "INFO: Library $reponame is already built. To rebuild use option -a|--action=clean-build"
		return
	fi
	mkdir -p $builddir
	cd $builddir
	if [ $reponame = "compiler-rt" ]; then
		build_compiler_rt "$pre" "$reponame" "$SRC/$srcarg/$reponame" "$installpath"
	elif [ $reponame = "libcxx" ]; then
		trigger_library_download "$revision" "libcxxabi"
		build_libcxx "$pre" "$reponame" "$SRC/$srcarg/$reponame" "$installpath"
	elif [ $reponame = "libcxxabi" ]; then
		trigger_library_download "$revision" "libcxx"
		build_libcxxabi "$pre" "$reponame" "$SRC/$srcarg/$reponame" "$installpath"
	elif [ $reponame = "libcxxrt" ]; then
		download_libcxxrt "$revision"
		build_libcxxrt "$pre" "$reponame" "$SRC/$srcarg/$reponame" "$installpath"
	elif [ $reponame = "libunwind" ]; then
		build_libunwind "$pre" "$reponame" "$SRC/$srcarg/$reponame" "$installpath"
	else
		echo "ERROR: Library '$reponame' not supported"
		exit 1
	fi
}

export_gcc_compiler() {
	GCC=$(run "which gcc")
	GPP=$(run "which g++")
	echo "Found GCC - $GCC"
	GCC_VERSION=`$GCC --version | grep gcc | sed s/.*\)//g | xargs`
	check_version "GCC" "4.7.0" "$GCC_VERSION"
	export CC=$GCC
	export CXX=$GPP
}

export_clang_compiler() {
	CLANG=$(run "which clang")
	CLANGPP=$(run "which clang++")
	echo "Found CLANG - $CLANG"
	export CC=${CLANG}
	export CXX=${CLANGPP}
}

llvm_build_depedent_libraries() {
	release=$1
	linstallpath=$2
	build_library "${WHICH_BINUTILS}binutils/$release" "$linstallpath" "libcxxabi"
	export LD_LIBRARY_PATH=${linstallpath}/${WHICH_BINUTILS}binutils/$release/lib:$LD_LIBRARY_PATH
	export LDFLAGS="-L${linstallpath}/${WHICH_BINUTILS}binutils/$release/lib"
	build_library "${WHICH_BINUTILS}binutils/$release" "$linstallpath" "libcxx"
	export LLVM_EXTRA_CONFIG="-DLLVM_ENABLE_LIBCXX=ON -DLLVM_ENABLE_LIBCXXABI=ON -DLLVM_ENABLE_CXX11=ON"
}

# Required only on Linux
locate_stdcpp_library() {
	if [ -z $LIB_STDCPP_PATH ]; then
		gcc=$1
		gcc_version=$2
		machine=`uname -m`
		guess_path=`dirname $gcc`
		libname=libstdc++.so
		if [ -f $guess_path/../lib64/$libname ]; then
			LIB_STDCPP_PATH=$guess_path/../lib64
		elif [ -f $guess_path/${machine}-linux-gnu/lib/gcc/$gcc_version/$libname ]; then
			LIB_STDCPP_PATH=$guess_path/../lib64
		else
			echo "ERROR: Failed to guess '$libname' library path. Please set environment variable LIB_STDCPP_PATH"
			exit 1
		fi
    else
    	if [ ! -f $LIB_STDCPP_PATH/$libname ]; then
    		echo "ERROR: Failed to locate '$libname' in path '$LIB_STDCPP_PATH'"
    		exit 1
    	fi
    fi
    export LLVM_EXTRA_CONFIG="$LLVM_EXTRA_CONFIG -DCMAKE_INSTALL_RPATH=\$ORIGIN/../lib:\$ORIGIN/"
}

common() {
	compiler=$1
	release=$2
	arg="${WHICH_BINUTILS}binutils/$2"
	mkdir -p ${INSTALL}/$arg ${PREINSTALL}/$arg

	if [ "$PLATFORM" = "FreeBSD" ]; then
		CLANG=$(run "which clang")
		clang_version=`$CLANG --version | grep version | sed s/.*version//g | sed s/\(.*//g | xargs`
		if [ "$clang_version" = "3.4.1" ] ; then
			export_clang_compiler
			build_${WHICH_BINUTILS}binutils "$arg" "${PREINSTALL}"
			# CC: error: unable to execute command: Segmentation fault (core dumped)
			# CC: error: clang frontend command failed due to signal (use -v to see invocation)
			# FreeBSD clang version 3.4.1 (tags/RELEASE_34/dot1-final 208032) 20140512
			# Target: x86_64-unknown-freebsd10.2

			# Extra step to first build clang using gcc
			if [ -f /usr/local/bin/gcc48 ] && [ -f /usr/local/bin/g++48 ]; then
				export CC=/usr/local/bin/gcc48
				export CXX=/usr/local/bin/g++48
				#llvm_build_depedent_libraries "$release" "$PREINSTALL"
				#export LDFLAGS="-B${PREINSTALL}/${WHICH_BINUTILS}binutils/$release/lib"
				${compiler}build_llvm $arg "${PREINSTALL}"
			else
				echo "ERROR: Install newer gcc48 from ports to build. CMD: pkg install lang/gcc48"
				exit 1
			fi
			export PATH=${PREINSTALL}/$arg/bin:$PATH
			CLANG=$(run "which clang")
			clang_version=`$CLANG --version | grep version | sed s/.*version//g | sed s/\(.*//g | xargs`
			if [ "$clang_version" != "3.4.1" ] ; then
				export_clang_compiler
				build_${WHICH_BINUTILS}binutils "$arg" "${INSTALL}"
				${compiler}build_llvm $arg "${INSTALL}"
			else
				echo "ERROR: Clang is not correctly built with gcc"
				exit 1
			fi
		else # if clang is higher than 3.4.1
			export_clang_compiler
			build_${WHICH_BINUTILS}binutils "$arg" "${INSTALL}"
			${compiler}build_llvm $arg "${INSTALL}"
		fi
	elif [ "$PLATFORM" = "Linux" ]; then
		# First build with GCC & then with clang
		export_gcc_compiler
		llvm_build_depedent_libraries "$release" "$PREINSTALL"
		export LDFLAGS="-B${PREINSTALL}/${WHICH_BINUTILS}binutils/$release/lib"
		build_${WHICH_BINUTILS}binutils "$arg" "${PREINSTALL}"
		${compiler}build_llvm $arg "${PREINSTALL}"

		export PATH=${PREINSTALL}/$arg/bin:$PATH
		# Need libstdc++.so lib to build clang using clang
		locate_stdcpp_library "$GCC" "$GCC_VERSION"
		export_clang_compiler
		#export CXXFLAGS="-stdlib=libc++"
		#export LDFLAGS="-L${INSTALL}/${WHICH_BINUTILS}binutils/$release/lib -llibc++abi"
		build_${WHICH_BINUTILS}binutils "$arg" "${INSTALL}"
		${compiler}build_llvm $arg "${INSTALL}"
	elif [ "$PLATFORM" = "Darwin" ]; then
		export_clang_compiler
		build_${WHICH_BINUTILS}binutils "$arg" "${INSTALL}"
		${compiler}build_llvm $arg "${INSTALL}"
	else
		echo "ERROR: $PLATFORM not yet supported"
		exit 1
	fi
}

binutils_head_build() {
	get_${WHICH_BINUTILS}binutils
	compiler="no_"
	common "$compiler" "head"
}

binutils_release_build() {
	version=$1
	get_${WHICH_BINUTILS}binutils
	compiler="no_"
	common "$compiler" "release/$version"
}

compiler_head_build() {
	get_${WHICH_BINUTILS}binutils
	get_llvm_infra
	compiler=""
	common "$compiler" "head"
}

compiler_release_build() {
	version=$1
	get_${WHICH_BINUTILS}binutils
	download_llvm_release_src $version
	compiler=""
	common "$compiler" "release/$version"
}

contains() {
    release="$1"
    shift
    list=$@
    output=0
    for r in $list; do
    	if [ "$r" = "$release" ]; then
    		output=1
    		break
    	fi
    done
    echo "$output"
}

check_target_triplet() {
	targets=$1
	binutils_path=$2
	if [ ! -f $binutils_path/config.sub ]; then
		echo "ERROR: Wrong binutils path "
		exit 1
	fi
	tlist=`echo $targets | tr ',' ' '`
	for t in $tlist; do
		ntarget=$(run "$binutils_path/config.sub $t")
		if [ $? != 0 ]; then
			echo "ERROR: Target '$t' not supported/correct"
			exit 1
		else
			if [ "$t" != "$ntarget" ]; then
				printf "INFO: Target '$t' is built as '$ntarget'\n"
			fi
			TARGETS="${TARGETS}$ntarget "
		fi
	done
	if [ "$TARGETS" = "" ]; then
		echo "ERROR: Target list not correct '$targets'"
		exit 1
	fi
}

check_arguments() {
	build=$1
	revision=$2
	targets=$3
	libname=$4
	action=$5
	binutils=$6
	ninja=$7
	if [ "$(contains $build $BUILDS)" = "1" ] ; then
		if [ "$revision" = "" ]; then
			echo "ERROR: Specify -r|--revision option"
			exit 1
		fi
		if [ "$build" = "binutils" ] && [ "$targets" = "" ]; then
			echo "ERROR: Specify -t|--targets option"
			exit 1
		fi
		if [ "$build" = "library" ] && [ "$libname" = "" ]; then
			echo "ERROR: Specify -l|--libname option"
			exit 1
		fi
	else
		echo "ERROR: Don't know how to build '$build'"
		exit 1
	fi

	if [ "$libanme" != "" ]; then
		if [ "$(contains $libname $LIBRARIES)" = "1" ] ; then
			echo "***** $libname ******"
		else
			echo "ERROR: Don't know how to build library $libname"
			exit 1
		fi
	fi

	if [ "$action" = "clean-build" ] || [ "$action" = "both" ]; then
		REBUILD=1
	fi

	if [ "$(contains $revision $LLVM_SUPPORTED_RELEASES)" = "0" ] ; then
		echo "ERROR: LLVM revision '$revision' not supported"
		exit 1
	fi

	if [ "$action" = "update-src" ] || [ "$action" = "both" ]; then
		if [ $revision = "head" ]; then
			REPOUPDATE=1
		else
			echo "ERROR: LLVM release is built with tar.xz file no update possible"
			exit 1
		fi
	fi

	#if [ "$build" = "binutils" ] || [ "$build" = "compiler"  ]; then
		name=""
		if   [ "$binutils" = "new" ]; then
			WHICH_BINUTILS="new_"
			name=`basename $NEW_BINUTILS_REPO`
		elif [ "$binutils" = "old" ]; then
			WHICH_BINUTILS="old_"
			name=`basename $BINUTILS_REPO`
		elif [ "$binutils" = "no" ]; then
			WHICH_BINUTILS="no_"
			echo "WARN: No targets for clang cross compiler"
		elif [ "$binutils" = "" ]; then
			if [ $WHICH_BINUTILS = "new_" ]; then
				name=`basename $NEW_BINUTILS_REPO`
				echo "INFO: By default new binutils-${NEW_BINUTILS_VERSION} will be used with compilers"
				binutils="new"
			elif [ $WHICH_BINUTILS = "old_" ]; then
				name=`basename $BINUTILS_REPO`
				echo "INFO: By default old binutils will be used with compilers"
				binutils="old"
			elif [ $WHICH_BINUTILS = "no_" ]; then
				name=""
				echo "INFO: By default binutils not used with compilers"
				binutils="no"
			fi
		else
			echo "ERROR: Acceptable values for -b|--binutils= options are <new/old/no>"
			exit 1
		fi

		if [ "$name" != "" ] && [ "$targets" != "" ]; then
			reponame=`echo $name | sed s/.tar.gz//g | xargs`
			get_${WHICH_BINUTILS}binutils
			check_target_triplet "$targets" "$SRC/$reponame"
		else
			if [ $WHICH_BINUTILS != "no_" ]; then
				TARGETS=$BINUTILS_TARGETS
			fi
		fi
	#fi

	if [ "$ninja" = "yes" ]; then
		tool_check "ninja" "yes"
		NINJA=1
	elif [ "$ninja" = "no" ]; then
		NINJA=0
	elif [ "$ninja" = "" ]; then
		if [ "$NINJA" = "1" ]; then
			ninja="yes"
		else
			ninja="no"
		fi
	else
		echo "Wrong value specified for -n|--ninja option"
		exit 1
	fi

	echo "*******************************************"
	printf "Build:			[$build]\n"
	printf "LLVM Revision:		[$revision]\n"
	printf "Target triplet:		[$TARGETS]\n"
	printf "Library:		[$libname]\n"
	printf "Action: 		[$action]\n"
	printf "Binutils:		[$binutils]\n"
	printf "Ninja:			[$ninja]\n"
	printf "Sysroot:		[\$PWD/sysroot]\n"
	echo "*******************************************"
}

usage() {
	name=$1
	echo "USAGE: $name <options>"
	echo "Supported options: "
	echo "  -b|--build= : One of the below supported packages"
	for bld in $BUILDS; do
		printf  "\t\t\t$bld\n"
	done

	echo "  -r|--revision= : One of the below supported LLVM release"
	for rel in $LLVM_SUPPORTED_RELEASES; do
		printf  "\t\t\t$rel\n"
	done

	echo "  -t|--targets= : Target triplet (comma separated)"

	echo "  -l|--libname= : One of the below"
	for lib in $LIBRARIES; do
		printf  "\t\t\t$lib\n"
	done
	echo "  -a|--action=	 : 'clean-build' or 'update-src' or both"
	echo "  -u|--binutils= : 'new', 'old' or 'no' "
	echo "  -n|--ninja= 	 : 'yes' or 'no'"
	echo "  -h|--help	 : Shows this help"
	echo "  -v|--version   : Shows version number of this script"
	exit 0
}

# Parse arguments
if [ $# = 0 ]; then
	echo "ERROR: Supply arguments"
	usage $0
else
	revision=""
	action=""
	binutils=""
	ninja=""
	build=""
	libname=""
	targets=""
	for i in "$@"; do
		case $i in
			-v|--version)
				echo "$VERSION"
				exit 0
				;;
			-h|--help)
				usage $0
				shift
				;;
			-r=*|--revision=*)
				revision="${i#*=}"
				shift
				;;
			-t=*|--targets=*)
				targets="${i#*=}"
				shift
				;;
			-a=*|--action=*)
				action="${i#*=}"
				shift
				;;
			-b=*|--build=*)
				build="${i#*=}"
				shift
				;;
			-l=*|--libname=*)
				libname="${i#*=}"
				shift
				;;
			-u=*|--binutils=*)
				binutils="${i#*=}"
				shift
				;;
			-n=*|--ninja=*)
				ninja="${i#*=}"
				shift
				;;
			*)
				echo "ERROR: Unknown argument '$i'"
				exit 1
				;;
		esac
	done

	check_arguments "$build" "$revision" "$targets" "$libname" "$action" "$binutils" "$ninja"

	if [ $build = "binutils" ]; then
		if [ $revision = "head" ]; then
			binutils_head_build
		else
			binutils_release_build "$revision"
		fi
	elif [ $build = "compiler" ]; then
		if [ $revision = "head" ]; then
			compiler_head_build
		else
			compiler_release_build "$revision"
		fi
	elif [ $build = "library" ]; then
		if [ $revision = "head" ]; then
			export CC="$INSTALL/${WHICH_BINUTILS}binutils/head/bin/clang"
			export CXX="$INSTALL/${WHICH_BINUTILS}binutils/head/bin/clang++"
			export LDFLAGS="-L${INSTALL}/${WHICH_BINUTILS}binutils/head/lib"
		else
			export CC="$INSTALL/${WHICH_BINUTILS}binutils/release/$revision/bin/clang"
			export CXX="$INSTALL/${WHICH_BINUTILS}binutils/release/$revision/bin/clang++"
			export LDFLAGS="-L${INSTALL}/${WHICH_BINUTILS}binutils/release/$revision/lib"
		fi
		if [ $libname = "libcxx" ]; then
			if [ ! -f "${INSTALL}/${WHICH_BINUTILS}binutils/release/$revision/lib/libc++abi.so" ]; then
				echo "ERROR: Build library libcxxabi before libcxx"
				exit 1
			fi
		fi
		build_library "${WHICH_BINUTILS}binutils/release/$revision" "$INSTALL" "$libname"
	else
		echo "ERROR: Unknown build type $build"
		exit 1
	fi
fi

