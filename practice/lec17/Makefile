CC := gcc

override CFLAGS   := -std=c99 -Og -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := -D_DEFAULT_SOURCE -D_SVID_SOURCE $(CPPFLAGS)
override LDFLAGS  := $(LDFLAGS)
override LDLIBS   := $(LDLIBS)

.PHONY: list
list:
	@echo "Targets:"
	@ls *.c | sed 's/\.c//'

bounds: benchmark.h
faults: benchmark.h
timings: benchmark.h

.PHONY: clean
clean: $(wildcard *.c)
	$(RM) $(^:.c=)
