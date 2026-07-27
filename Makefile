HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.c)

out: $(HEADERS) $(SOURCES)
	gcc -lm $(SOURCES) -o out
