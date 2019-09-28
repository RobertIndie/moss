BUILD_PATH := \
	./src\
	./example/client\
	./example/server\

BUILD := $(shell mkdir -p ./build;cd ./build;pwd)

all: 
	@for mod in $(BUILD_PATH); do\
		cd $$mod && make && cd -;\
	done
	
clean:
	rm -rf $(BUILD)
	find . -name "*.o"|xargs rm
	find . -name "*.a"|xargs rm

rebuild:
	make clean
	make 
