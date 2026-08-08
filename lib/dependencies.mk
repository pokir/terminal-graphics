PROJECT_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

include $(PROJECT_ROOT)/lib/third_party/assimp.mk

INCSPATH := -I$(PROJECT_ROOT)/lib/src $(ASSIMP_INCSPATH)
LIBS := -lm $(ASSIMP_LIBS)
DEPENDENCIES := $(ASSIMP_DEPENDENCIES)
