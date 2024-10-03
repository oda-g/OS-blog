/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright(c) 2024 Itsuro Oda
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

volatile static int done = 0;

static void alarm_handler(int not_used)
{
	done = 1;
}

int main(void)
{
	unsigned int count = 0;

	if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
		perror("signal");
		return 1;
	}

	(void)alarm(1U);

	while (!done) {
		count++;
	}

	printf("count: %u\n", count);

	return 0;
}
