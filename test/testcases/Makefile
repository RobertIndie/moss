PROJ_BASE := ../..
include $(PROJ_BASE)/makefile.libs

TARGET_TYPE := EXE
TARGET := Test

INC += $(INC_MOSS) $(INC_GTEST) -I .
LIB_DIR += $(LIB_DIR_GTEST)
LIB += $(LIB_MOSS) $(LIB_GTEST)

PROTOC := protoc
PROTO_DIR := ./proto

include $(PROJ_BASE)/makefile.moss

.PHONY: build_proto

build_proto:
	cd $(PROTO_DIR) && $(PROTOC) --cpp_out=. *.proto
