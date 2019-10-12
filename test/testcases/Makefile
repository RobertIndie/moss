PROJ_BASE := ../..
include $(PROJ_BASE)/makefile.libs

TARGET_TYPE := EXE
TARGET := Test

INC += $(INC_MOSS) $(INC_GTEST) $(INC_TEST_PROTO)
LIB_DIR += $(LIB_DIR_GTEST)
LIB := $(LIB_TEST_PROTO) $(LIB_MOSS) $(LIB_GTEST) $(LIB)

include $(PROJ_BASE)/makefile.moss
