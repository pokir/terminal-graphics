TARGET ?= out
.DEFAULT_GOAL := $(TARGET)

include $(dir $(lastword $(MAKEFILE_LIST)))library.mk

HEADERS := $(wildcard *.h)
SOURCES := $(wildcard *.c)

$(TARGET): $(DEPENDENCIES) $(LIB_HEADERS) $(LIB_SOURCES) $(HEADERS) $(SOURCES)
	$(CC) $(INCSPATH) \
		$(LIB_SOURCES) \
		$(SOURCES) \
		$(LIBS) \
		-o $(TARGET)
