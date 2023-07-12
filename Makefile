CC := clang
OPTIM := -O2
DEBUG :=
CCFLAGS := -std=c17 -Wall -Wextra -pedantic $(DEBUG) $(OPTIM) $(XCCFLAGS)
LDFLAGS := -lm $(XLDFLAGS)

3darr: 3darr.o
	$(CC) $^ -o $@ $(LDFLAGS) $(COMMONFLAGS)

3darr.o: 3darr.c
	$(CC) -c $^ -o $@ $(CCFLAGS) $(COMMONFLAGS)

.PHONY: clean
clean:
	rm -f *.o
	rm -f 3darr
