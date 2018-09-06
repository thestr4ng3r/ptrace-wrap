
#include <ptrace_wrap.h>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

static void *th_run(ptrace_wrap_instance *inst);

int ptrace_wrap_instance_start(ptrace_wrap_instance *inst) {
	int r = sem_init (&inst->request_sem, 0, 0);
	if (r != 0) {
		return r;
	}

	r = sem_init (&inst->result_sem, 0, 0);
	if (r != 0) {
		sem_destroy (&inst->request_sem);
		return r;
	}

	r = pthread_create (&inst->th, NULL, (void *(*)(void *)) th_run, inst);
	if (r != 0) {
		sem_destroy (&inst->request_sem);
		sem_destroy (&inst->result_sem);
		return r;
	}

	return 0;
}

void ptrace_wrap_instance_stop(ptrace_wrap_instance *inst) {
	inst->request.stop = 1;
	sem_post (&inst->request_sem);
	pthread_join (inst->th, NULL);
	sem_destroy (&inst->request_sem);
	sem_destroy (&inst->result_sem);
}

static void *th_run(ptrace_wrap_instance *inst) {
	while (1) {
		sem_wait (&inst->request_sem);
		if (inst->request.stop) {
			break;
		}
		inst->result = ptrace (inst->request.request, inst->request.pid, inst->request.addr, inst->request.data);
		*inst->request._errno = errno;
		sem_post (&inst->result_sem);
	}
	return NULL;
}

long ptrace_wrap(ptrace_wrap_instance *inst, enum __ptrace_request request, pid_t pid, void *addr, void *data, int *_errno) {
	inst->request.request = request;
	inst->request.pid = pid;
	inst->request.addr = addr;
	inst->request.data = data;
	inst->request._errno = _errno;
	inst->request.stop = 0;
	sem_post (&inst->request_sem);
	sem_wait (&inst->result_sem);
	return inst->result;
}