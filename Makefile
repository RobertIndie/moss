BUILD_PATH := \
	./example/client\
	./example/server\

BUILD := $(shell mkdir -p ./build;cd ./build;pwd)

all: 
	@for mod in $(BUILD_PATH); do\
		cd $$mod && make && cd -;\
	done
	
clean:
	rm -rf $(BUILD)
