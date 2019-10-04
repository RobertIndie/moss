BUILD_PATH := \
	./src\
	./example/client\
	./example/server\

BUILD := $(shell mkdir -p ./build;cd ./build;pwd)

.PHONY: all test clean rebuild third_party

all: 
	make third_party
	@for mod in $(BUILD_PATH); do\
		cd $$mod && make && cd -;\
	done

debug:
	@for mod in $(BUILD_PATH); do\
		cd $$mod && make && cd -;\
	done

third_party:
	cd ./third_party/ && make && cd -;

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
