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
static struct func_context *cur_fcon = NULL;

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

static void sched(void)
{
	struct func_context *con, *prev;

	if (cur_fcon == NULL) {
		/* called first time */
		if (getcontext(&con_sched) == -1) {
			perror("getcontext");
			exit(1);
		}
		/* here is resume point when func return */
	}

	con = get_sched_q();
	if (con) {
		if (cur_fcon == NULL) {
			cur_fcon = con;
			if (setcontext(&con->ucontext) == -1) {
				perror("sched: setcontext");
				exit(1);
			}
		} else {
			prev = cur_fcon;
			cur_fcon = con;
			if (swapcontext(&prev->ucontext, &con->ucontext) == -1) {
				perror("sched: swapcontext");
				exit(1);
			}
		}
	}
	/* no thread. return to main. */
}

static void yeild(int id)
{
	struct func_context *con = &func_context[id];

	put_sched_q(con);
	sched();
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

	sched();
	return 0;
}
