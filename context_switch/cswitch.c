/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright(c) 2024 Itsuro Oda
 */
#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>

struct func_context {
	struct func_context *next;
	ucontext_t ucontext;
};

static struct func_context *head, *tail;
#define NUM_THREAD 2
static struct func_context func_context[NUM_THREAD];
static ucontext_t con_sched;

/* CAUTION: must be sure the stack size is enough or
 * should add to handle stack overflow.
 */ 
#define STACK_SIZE 10000
static char stack[NUM_THREAD][STACK_SIZE];

static void put_sched_q(struct func_context *con)
{
	con->next = NULL;
	if (tail == NULL) {
		head = tail = con;
	} else {
		tail->next = con;
		tail = con;
	}
}

static struct func_context *get_sched_q(void)
{
	struct func_context *con = head;

	if (head) {
		head = head->next;
		if (tail == con) {
			tail = NULL;
		}
	}

	return con;
}

static int sched(void)
{
	struct func_context *con;

	if (getcontext(&con_sched) == -1) {
		perror("sched: getcontext");
		return 1;
	}
	con = get_sched_q();
	if (con) {
		if (setcontext(&con->ucontext) == -1) {
			perror("sched: setcontext");
			return 1;
		}
	}
	/* no thead. return */
	return 0;
}

static void yeild(int id)
{
	struct func_context *con = &func_context[id];

	put_sched_q(con);
	if (swapcontext(&con->ucontext, &con_sched) == -1) {
		perror("swapcontest");
		exit(1);
	}
	/* note: swapcontext returns 0 when resumed. */
}

static void func(int id)
{
	for (int i = 0; i < 3; i++) {
		printf("func_%d work (%d)\n", id, i);
		yeild(id);
	}
}

static int create_thread(int id)
{
	struct func_context *con = &func_context[id];
	ucontext_t *ucon = &con->ucontext;

	if (getcontext(ucon) == -1) {
		perror("getcontext");
		return 1;
	}
	ucon->uc_stack.ss_sp = stack[id];
	ucon->uc_stack.ss_size = sizeof(stack[id]);
	ucon->uc_link = &con_sched;
	makecontext(ucon, (void (*)())func, 1, id);

	put_sched_q(con);
	return 0;
}

int main(void)
{
	for (int i = 0; i < NUM_THREAD; i++) {
		if (create_thread(i) == 1) {
			return 1;
		}
	}

	return sched();
}
