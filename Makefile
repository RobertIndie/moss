BUILD_PATH := \
	./src\
	./example/client\
	./example/server\

BUILD := $(shell mkdir -p ./build;cd ./build;pwd)

.PHONY: all test clean rebuild third_party

all: 
	git submodule update --init --recursive && make third_party
	@for mod in $(BUILD_PATH); do\
		cd $$mod && make && cd -;\
	done

debug:
	@for mod in $(BUILD_PATH); do\
		cd $$mod && make && cd -;\
	done

third_party:
	cd ./third_party && make && cd -;

test:
	cd test/testcases && make build_proto && make

run_test:	
	./build/apps/Test
	
debug_clean:
	-rm -rf $(BUILD)
	-find . -name "*.o" ! -path "./third_party/*"|xargs rm
	-find . -name "*.a" ! -path "./third_party/*"|xargs rm
	-find . -name "*.pb.h" ! -path "./third_party/*"|xargs rm
	-find . -name "*.pb.cc" ! -path "./third_party/*"|xargs rm

clean:
	-rm -rf $(BUILD)
	-find . -name "*.o"|xargs rm
	-find . -name "*.a"|xargs rm
	-find . -name "*.pb.h"|xargs rm
	-find . -name "*.pb.cc"|xargs rm
	-cd ./third_party && make clean

rebuild:
	-make clean
	make 
