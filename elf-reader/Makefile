HAB 			= /volume/hab
OS 				= $(shell uname -s)
ARCH 			= $(shell uname -m)
OS_ID 			= $(shell /volume/hab/$(OS)/bin/os-id)
LLVM_VERSION 	= 7.0
LLVM 			= llvm/$(LLVM_VERSION)
COMPILER 		= $(LLVM)/current/bin/clang++
SYSROOT_PATH 	= /volume/compiler-images/sysroot
EXE_NAME 		= elf-reader
MKDIR 			= mkdir
CC 				= $(HAB)/$(OS)/$(OS_ID)/$(ARCH)/$(COMPILER)
GCC 			= $(CC)
#GCC 			= /usr/bin/g++

BIN 			= bin
INCLUDES 		= -Iinclude

DEBUG 			= -g2
CFLAGS 			= -c $(DEBUG) $(INCLUDES)
LDFLAGS 		= -DDEBUG -g2
VPATH           = src
SOURCE_PATH     = src
SOURCES 		= $(SOURCE_PATH)/common.cpp \
					$(SOURCE_PATH)/host.cpp \
					$(SOURCE_PATH)/elf-header.cpp \
					$(SOURCE_PATH)/program-header.cpp \
					$(SOURCE_PATH)/section-header.cpp \
					$(SOURCE_PATH)/symbol-table.cpp \
					$(SOURCE_PATH)/dynamic-linking.cpp \
					$(SOURCE_PATH)/symbol-info.cpp \
					$(SOURCE_PATH)/dynamic-symbol-table.cpp \
					$(SOURCE_PATH)/relocation.cpp \
					$(SOURCE_PATH)/elf-reader.cpp

OBJ_SUFFIX 		= o
SOURCES_BARE    = $(notdir $(SOURCES))
OBJECTS 		= ${SOURCES_BARE:%.cpp=.objects_native/%.$(OBJ_SUFFIX)}
BUILDS 			= native \
				  amd64-unknown-freebsd10.1 \
				  i386-unknown-freebsd10.1 \
				  armv6--freebsd10.1-gnueabi \
				  amd64-unknown-freebsd11.0 \
				  i386-unknown-freebsd11.0 \
				  armv6--freebsd11.0-gnueabihf

.PHONY: all

all: native

.objects_native/%.$(OBJ_SUFFIX): %.cpp
	@mkdir -p $(dir $@)
	@echo Compiling: $<
	$(GCC) $(CFLAGS) $(CROSS_FLAGS) -o $@ $<

native: clean-objs ${OBJECTS}
	$(eval EXE_PATH := $(BIN)/native/$(OS)/$(ARCH))
	@$(MKDIR) -p $(EXE_PATH)
	$(eval EXE := $(EXE_PATH)/$(EXE_NAME))
	@echo Linking: $(EXE)
	$(GCC) -o $(EXE) $(OBJECTS) $(LDFLAGS)

cross-setup:
	$(eval TARGET   := --target=$(TRIPLE))
	$(eval SYSROOT  := --sysroot=$(SYSROOT_PATH)/$(TRIPLE))
	$(eval EXE_PATH := $(BIN)/cross/$(TRIPLE))
	@$(MKDIR) -p $(EXE_PATH)
	$(eval EXE 		:= $(EXE_PATH)/$(EXE_NAME))
	@echo Linking: $(EXE)
	$(eval CC := $(CC) $(TARGET) $(SYSROOT))

cross: clean-objs cross-setup ${OBJECTS}
	$(CC) -o $(EXE) $(OBJECTS) $(LDFLAGS)

amd64-fbsd10-triple:
	$(eval TRIPLE   := amd64-unknown-freebsd10.1)

amd64-fbsd10: amd64-fbsd10-triple  cross

i386-fbsd10-triple:
	$(eval TRIPLE   := i386-unknown-freebsd10.1)

i386-fbsd10: i386-fbsd10-triple cross

armv6-fbsd10-triple:
	$(eval TRIPLE   := armv6--freebsd10.1-gnueabi)

armv6-fbsd10: armv6-fbsd10-triple cross

clean:
	@echo "Cleaning..."
	@rm -rf $(BIN)/* .objects*

clean-objs:
	@rm -rf .objects*

print-%: ; @echo $*=$($*)


# make --warn-undefined-variables
# valgrind --leak-check=full --log-file=val.log --show-reachable=yes --tool=memcheck ./bin/native/Linux/x86_64/elf-reader ./bin/native/Linux/x86_64/elf-reader -r
