
#ifndef PTRACE_WRAP_H
#define PTRACE_WRAP_H

#include <pthread.h>
#include <semaphore.h>
#include <sys/ptrace.h>

typedef struct ptrace_wrap_request_t {
	enum __ptrace_request request;
	pid_t pid;
	void *addr;
	void *data;
	int *_errno;
	int stop;
} ptrace_wrap_request;

typedef struct ptrace_wrap_instance_t {
	pthread_t th;
	sem_t request_sem;
	ptrace_wrap_request request;
	sem_t result_sem;
	long result;
} ptrace_wrap_instance;

int ptrace_wrap_instance_start(ptrace_wrap_instance *inst);
void ptrace_wrap_instance_stop(ptrace_wrap_instance *inst);
long ptrace_wrap(ptrace_wrap_instance *inst, enum __ptrace_request request, pid_t pid, void *addr, void *data, int *_errno);

#endif //PTRACE_WRAP_H
