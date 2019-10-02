BUILD_PATH := \
	./src\
	./example/client\
	./example/server\

BUILD := $(shell mkdir -p ./build;cd ./build;pwd)

.PHONY: all test clean rebuild

all: 
	@for mod in $(BUILD_PATH); do\
		cd $$mod && make && cd -;\
	done

test:
	cd test/testcases && make
	cd test/virtual_server && make
	./build/apps/Test
	
clean:
	-rm -rf $(BUILD)
	-find . -name "*.o"|xargs rm
	-find . -name "*.a"|xargs rm

rebuild:
	-make clean
	make 
