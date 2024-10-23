/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright(c) 2024 Itsuro Oda
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define NUM_COUNT 100000000

static int *total_count;

int main(void)
{
	pid_t pid;
	int i;

	total_count = mmap(NULL, sizeof(*total_count), PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_SHARED, -1, 0);
	if (total_count == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	*total_count = 0;

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return 1;
	} else if (pid == 0) {
		/* child */
		for (i = 0; i < NUM_COUNT; i++) {
			__atomic_fetch_add(total_count, 1, __ATOMIC_RELAXED);
		}
		exit(0);
	} else {
		/* parent */
		for (i = 0; i < NUM_COUNT; i++) {
			__atomic_fetch_add(total_count, 1, __ATOMIC_RELAXED);
		}
	}

	(void)wait(NULL);

	printf("total_count: %d\n", *total_count);

	return 0;
}
