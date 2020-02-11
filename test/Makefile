TEST_BUILD_PATH := \
	./proto\
	./test_server\
	./testcases

.PHONY: build_test run_test build_proto

PROTOC := protoc
PROTO_DIR := ./proto

build_test:
	make build_proto
	@for mod in $(TEST_BUILD_PATH); do\
		cd $$mod && make && cd -;\
	done

build_proto:
	cd $(PROTO_DIR) && $(PROTOC) --cpp_out=. *.proto

run_test:
	../build/apps/Test
