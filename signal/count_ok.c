#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_COUNT 1000000

static int *total_count;
static sem_t *sem;

int main(void)
{
	pid_t pid;
	int i;
	int wstatus;
	int err = 0;

	total_count = mmap(NULL, sizeof(*total_count) + sizeof(sem_t),
		PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
	if (total_count == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	*total_count = 0;
	sem = (sem_t *)(total_count + 1);

	if (sem_init(sem, 1, 1) == -1) {
		perror("sem_init");
		return 1;
	}

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return 1;
	} else if (pid == 0) {
		/* child */
		for (i = 0; i < NUM_COUNT; i++) {
			if (sem_wait(sem) == -1) {
				perror("child sem_wait");
				exit(1);
			}
			(*total_count)++;
			(void)sem_post(sem); /* there is no error case */
		}
		exit(0);
	} else {
		/* parent */
		for (i = 0; i < NUM_COUNT; i++) {
			if (sem_wait(sem) == -1) {
				perror("parent sem_wait");
				err = 1;
				break;
			}
			(*total_count)++;
			(void)sem_post(sem); /* there is no error case */
		}
	}

	(void)wait(&wstatus);
	if (WEXITSTATUS(wstatus) != 0 || err) {
		printf("something wrong.\n");
	} else {
		printf("total_count: %d\n", *total_count);
	}

	return 0;
}
