/* SPDX-License-Identifier: BSD-2-Clause
 * Copyright(c) 2024 Itsuro Oda
 */
#include <stdio.h>
#include <ucontext.h>

struct func_context {
	struct func_context *next;
	int is_resume;
	ucontext_t ucontext;
};

static struct func_context *head, *tail;
#define NUM_THREAD 2
static struct func_context func_context[NUM_THREAD];
static ucontext_t con_s;

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
	struct func_context *con;

	con = head;
	if (head) {
		head = head->next;
		if (tail == con) {
			tail = NULL;
		}
	}

	return con;
}

static void sched(void)
{
	struct func_context *con;

	getcontext(&con_s);
	con = get_sched_q();
	if (con) {
		con->is_resume = 1;
		setcontext(&con->ucontext);
	}
	/* no thead. return */
}

static void yeild(int id)
{
	struct func_context *con = &func_context[id];

	con->is_resume = 0;
	getcontext(&con->ucontext);
	if (con->is_resume == 0) {
		put_sched_q(con);
		setcontext(&con_s);
	}
}

static void func(int id)
{
	int i;

	for (i = 0; i < 3; i++) {
		printf("func_%d work (%d)\n", id, i);
		yeild(id);
	}
}

static void create_thread(int id)
{
	struct func_context *con = &func_context[id];

	getcontext(&con->ucontext);
	con->ucontext.uc_stack.ss_sp = stack[id];
	con->ucontext.uc_stack.ss_size = sizeof(stack[id]);
	con->ucontext.uc_link = &con_s;
	con->is_resume = 0;
	makecontext(&con->ucontext, (void (*)())func, 1, id);

	put_sched_q(con);
}

int main(void)
{
	int i;

	for (i = 0; i < NUM_THREAD; i++) {
		create_thread(i);
	}

	sched();

	return 0;
}
