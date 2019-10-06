PROJ_BASE := ../..
include $(PROJ_BASE)/makefile.libs

TARGET_TYPE := EXE
TARGET := Test

INC += $(INC_MOSS) $(INC_GTEST)
LIB_DIR += $(LIB_DIR_GTEST)
LIB += $(LIB_GTEST)

PROTOC := protc
PROTO_SRC_DIRS = $(shell find . -type d)
PROTO_SRC += $(foreach dir, $(PROTO_SRC_DIRS), $(wildcard $(dir)/*.proto))
%.pb.cc:%.proto
	$(PROTOC) --cpp_out=. $<
	

include $(PROJ_BASE)/makefile.moss
