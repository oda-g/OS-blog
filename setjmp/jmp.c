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
	/* not return */
}

static void b(void)
{
	printf("func b called\n");
	c();
	/* not return */
}

static void a(void)
{
	printf("func a called\n");
	b();
	/* not return */
}

int main(void)
{
	if (setjmp(env) == 1) {
		printf("return\n");
		return 0;
	}

	a();

	/* not reach here */
	printf("anything wrong\n");
	return 1;
}
