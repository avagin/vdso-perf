CC := $(CROSS_COMPILE)gcc
CFLAGS += -Wall -Werror
LDFLAGS := -lrt -ldl

gettime_perf: gettime_perf.c

clean:
	rm -rf gettime_perf
