HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.c)

out: $(HEADERS) $(SOURCES)
	gcc $(SOURCES) -o out
