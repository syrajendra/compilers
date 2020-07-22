#!/bin/sh
set -ex

TOP=$PWD
mkdir -p $TOP/build $TOP/install

git clone https://github.com/llvm/llvm-project.git
cd llvm-project
git checkout release/10.x

cd $TOP/build

cmake -G "Unix Makefiles" \
	$TOP/llvm-project/llvm \
	-DLLVM_ENABLE_PROJECTS="compiler-rt;libunwind;lld;clang;clang-tools-extra;libcxx;libcxxabi;lldb" \
	-DCMAKE_INSTALL_PREFIX="$TOP/install" \
	-DCMAKE_BUILD_TYPE='Release' 2>&1 | tee configure.log
make -j 4 2>&1 | tee build.log
make install 2>&1 | tee install.log


