// SPDX-License-Identifier: GPL-2.0
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <dlfcn.h>
#include <stdio.h>
#include <errno.h>

#define pr_err(fmt, ...)						\
	({								\
		fprintf(stderr, fmt "\n", ##__VA_ARGS__);		\
		-1;							\
	})

#define pr_perror(fmt, ...)	pr_err(pr_err, fmt ": %s", ##__VA_ARGS__, stderror(errno))

typedef int (*vgettime_t)(clockid_t, struct timespec *);

vgettime_t vdso_clock_gettime;

static void fill_function_pointers(void)
{
	void *vdso = dlopen("linux-vdso.so.1",
			    RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD);
	if (!vdso)
		vdso = dlopen("linux-gate.so.1",
			      RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD);
	if (!vdso) {
		pr_err("[WARN]\tfailed to find vDSO\n");
		return;
	}

	vdso_clock_gettime = (vgettime_t)dlsym(vdso, "__vdso_clock_gettime");
	if (!vdso_clock_gettime)
		pr_err("Warning: failed to find clock_gettime in vDSO\n");

}

static void test(clock_t clockid, char *clockstr)
{
	struct timespec tp, start;
	long i = 0;
	const int timeout = 3;

	vdso_clock_gettime(clockid, &start);
	tp = start;
	for (tp = start; start.tv_sec + timeout > tp.tv_sec ||
			 (start.tv_sec + timeout == tp.tv_sec &&
			  start.tv_nsec > tp.tv_nsec); i++) {
		vdso_clock_gettime(clockid, &tp);
	}

	printf("clock: %16s\tcycles:\t%10ld\n",
			      clockstr, i);
}

int main(int argc, char *argv[])
{
	fill_function_pointers();

	test(CLOCK_MONOTONIC, "monotonic");
	test(CLOCK_MONOTONIC_COARSE, "monotonic-coarse");
	test(CLOCK_MONOTONIC_RAW, "monotonic-raw");
	test(CLOCK_BOOTTIME, "boottime");

	return 0;
}
