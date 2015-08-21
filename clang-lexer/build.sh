#!/bin/bash -x

# Build clang as explained here 'http://clang.llvm.org/get_started.html'

TOP="/b/jenkins/head"
LLVM="${TOP}/llvm/"
BUILD="${TOP}/build/"

export CC="/usr/bin/clang -g "
export CXX="/usr/bin/clang++ -g "

$CXX \
	-I${LLVM}/tools/clang/include/ \
	-I${BUILD}/tools/clang/include/ \
  	`${BUILD}/bin/llvm-config --cxxflags` \
  	-fno-rtti -c clang-lexer.cpp

if [ $? = 0 ]; then
	$CXX `${BUILD}/bin/llvm-config --ldflags` \
  		-lLLVMSupport  -lLLVMBitReader \
  		-lLLVMBitWriter -lLLVMTarget \
  		-lLLVMMC  \
  		-lclangBasic -lclangLex  -lncurses \
  		 -lclangFrontend -lclangEdit \
  		-fno-rtti  -o clang-lexer clang-lexer.o
fi

# -lLLVMSystemZDesc -lLLVMBitReader -lLLVMBitWriter -lclangDriver -lclangParse -lclangAST -lclangASTMatchers -lclangARCMigrate -lclangAnalysis -lncurses \
# -undefined dynamic_lookup
# -lLLVMAArch64AsmParser -lLLVMAArch64AsmPrinter -lLLVMAArch64Desc -lLLVMAArch64Info