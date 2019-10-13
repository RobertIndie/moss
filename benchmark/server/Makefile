PROJ_BASE := ../../
include $(PROJ_BASE)/makefile.libs

TARGET_TYPE := EXE
TARGET := BenchmarkServer

INC += $(INC_MOSS) $(INC_TEST_PROTO) -I $(PROJ_BASE)/benchmark/proto
LIB := -lBenchmarkProto $(LIB_MOSS) $(LIB) 

include $(PROJ_BASE)/makefile.moss
