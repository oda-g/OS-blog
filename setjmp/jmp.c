/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright(c) 2024 Itsuro Oda
 */
#include <stdio.h>
#include <setjmp.h>

static jmp_buf env;

static void c(void)
{
	printf("func c called\n");
	longjmp(env, 1);
	printf("not reach here\n");
}

static void b(void)
{
	printf("func b called\n");
	c();
	printf("not reach here\n");
}

static void a(void)
{
	printf("func a called\n");
	b();
	printf("not reach here\n");
}

int main(void)
{
	if (setjmp(env) == 1) {
		printf("return\n");
		return 0;
	}

	a();

	printf("not reach here\n");
	return 1;
}
