TARGET ?= out
.DEFAULT_GOAL := $(TARGET)

include $(dir $(lastword $(MAKEFILE_LIST)))library.mk

HEADERS := $(wildcard *.h)
SOURCES := $(wildcard *.c)

$(TARGET): $(LIBRARY) $(HEADERS) $(SOURCES)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCSPATH) \
		$(SOURCES) \
		$(LIBRARY) \
		$(LIBS) \
		-o $(TARGET)
