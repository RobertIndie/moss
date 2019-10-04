THIRD_BUILD = $(shell mkdir -p ./build;cd ./build;pwd)
THIRD_BASE = $(shell pwd)

.PHONY: build clean

build:
	make build_googletest
	make build_glog

build_googletest:
	mkdir -p ./build/googletest
	cd $(THIRD_BUILD)/googletest && cmake $(THIRD_BASE)/googletest && make

build_glog:
	cd $(THIRD_BASE)/glog && ./autogen.sh && ./configure && make
	cp $(THIRD_BASE)/glog/.libs/libglog.so.0 /usr/lib

clean:
	-rm -rf $(THIRD_BUILD)