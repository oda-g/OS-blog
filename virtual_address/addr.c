/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright(c) 2024 Itsuro Oda
 */
#include <stdio.h>

static int static_var;

int main(void)
{
	int local_var;

	printf("static_var address: %p\n", &static_var);
	printf("local_var address: %p\n", &local_var);

	*((int *)0x10280a8) = 10;
	printf("static_var: %d\n", static_var);

	return 0;
}
