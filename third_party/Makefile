THIRD_BUILD = $(shell mkdir -p ./build;cd ./build;pwd)
THIRD_BASE = $(shell pwd)
THIRD_REPO := \
	./googletest\
	./glog\

.PHONY: build clean

build:
	make build_googletest
	make build_glog

init_third_repo:
	@for mod in $(THIRD_REPO); do\
		cd $$mod && git pull origin master && git checkout master && cd -;\
	done

build_googletest:
	mkdir -p ./build/googletest
	cd $(THIRD_BUILD)/googletest && cmake $(THIRD_BASE)/googletest && make

build_glog:
	cd $(THIRD_BASE)/glog && ./autogen.sh && ./configure && make
	cp $(THIRD_BASE)/glog/.libs/libglog.so.0 /usr/lib

clean:
	cd $(THIRD_BASE)/googletest && git checkout . && git clean -xdf
	cd $(THIRD_BASE)/glog && git checkout . && git clean -xdf
	-rm -rf $(THIRD_BUILD)