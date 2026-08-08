include $(dir $(lastword $(MAKEFILE_LIST)))dependencies.mk

LIB_PATH := $(PROJECT_ROOT)/lib/src
LIB_HEADERS := $(wildcard $(LIB_PATH)/*.h)
LIB_SOURCES := $(wildcard $(LIB_PATH)/*.c)
