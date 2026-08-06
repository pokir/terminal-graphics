LIB_PATH=../../lib/src

INCSPATH=-I$(LIB_PATH)
LIBS=-lm

LIB_HEADERS=$(wildcard $(LIB_PATH)/*.h)
LIB_SOURCES=$(wildcard $(LIB_PATH)/*.c)

HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.c)

out: $(LIB_HEADERS) $(LIB_SOURCES) $(HEADERS) $(SOURCES)
	gcc $(INCSPATH) \
		$(LIB_SOURCES) \
		$(SOURCES) \
		$(LIBS) \
		-o out
