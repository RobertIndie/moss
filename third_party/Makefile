THIRD_BUILD = $(shell mkdir -p ./build;cd ./build;pwd)
THIRD_BASE = $(shell pwd)
THIRD_REPO := \
	./googletest\
	./glog\

.PHONY: build clean

build:
	make build_googletest
	make build_glog
	make build_protobuf

build_googletest:
	mkdir -p ./build/googletest
	cd $(THIRD_BUILD)/googletest && cmake $(THIRD_BASE)/googletest && make

build_glog:
	cd $(THIRD_BASE)/glog && ./autogen.sh && ./configure && make
	#cp $(THIRD_BASE)/glog/.libs/libglog.so.0 /usr/lib
	export LD_LIBRARY_PATH=$(THIRD_BASE)/glog/.libs

build_protobuf:
	cd $(THIRD_BASE)/protobuf && git submodule update --init --recursive && ./autogen.sh && ./configure --disable-shared && make

clean:
	cd $(THIRD_BASE)/googletest && git checkout . && git clean -xdf
	cd $(THIRD_BASE)/glog && git checkout . && git clean -xdf
	-rm -rf $(THIRD_BUILD)