PROJ_BASE := ../..
include $(PROJ_BASE)/makefile.libs

TARGET_TYPE := EXE
TARGET := MossTest

INC += $(INC_MOSS) $(INC_GTEST)
LIB_DIR += $(LIB_DIR_GTEST)
LIB := $(LIB_MOSS) $(LIB_GTEST) $(LIB)

CXXFLAGS += -w
include $(PROJ_BASE)/makefile.moss
