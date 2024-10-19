ALL := 3darr doc
CC := gcc
OPTIM := -O3
CCFLAGS := -std=c17 -Wall -Wextra -pedantic $(DEBUG) $(OPTIM) $(XCCFLAGS)
LDFLAGS := $(XLDFLAGS)

.PHONY: all
all: $(ALL)

3darr: 3darr.o
	$(CC) $^ -o $@ $(LDFLAGS) $(COMMONFLAGS)

3darr.o: 3darr.c
	$(CC) -c $^ -o $@ $(CCFLAGS) $(COMMONFLAGS)

doc: 3darr.c Doxyfile
	doxygen Doxyfile

.PHONY: clean
clean:
	rm -f *.o
	rm -rf $(ALL)

