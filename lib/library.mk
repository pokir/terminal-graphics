include $(dir $(lastword $(MAKEFILE_LIST)))dependencies.mk

LIB_PATH := $(PROJECT_ROOT)/lib/src
LIB_HEADERS := $(wildcard $(LIB_PATH)/*.h)
LIB_SOURCES := $(wildcard $(LIB_PATH)/*.c)
LIB_BUILD := $(PROJECT_ROOT)/.deps/terminal-graphics
LIB_OBJECTS := $(patsubst $(LIB_PATH)/%.c,$(LIB_BUILD)/%.o,$(LIB_SOURCES))
LIBRARY := $(LIB_BUILD)/libterminal_graphics.a

$(LIB_BUILD):
	mkdir -p $@

$(LIB_BUILD)/%.o: $(LIB_PATH)/%.c $(LIB_HEADERS) $(DEPENDENCIES) | $(LIB_BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LIB_PRIVATE_INCSPATH) -c $< -o $@

$(LIBRARY): $(LIB_OBJECTS)
	$(AR) rcs $@ $^
