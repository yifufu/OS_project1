#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>

struct Process {
	//struct timespec t_start, t_end;
	char name[34];
	int ready_time, exec_time;
	int pid, priority;
};

int main(int argc, char *argv[]) {
	struct sched_param param;
	param.sched_priority = 99;
	if (sched_setscheduler(getpid(), SCHED_FIFO, &param)) printf("need permision!");

	long time_unit_usec;
	struct timespec t1, t2;
	clock_gettime(CLOCK_REALTIME, &t1);
	volatile unsigned long tmp; for (tmp=0;tmp<500 * 1000000UL;tmp++);
	clock_gettime(CLOCK_REALTIME, &t2);
	time_unit_usec = (t2.tv_sec - t1.tv_sec) * (1000000 / 500) + (t2.tv_nsec - t1.tv_nsec) / 500000;
	//printf("%ld.%ld\n", t1.tv_sec, t1.tv_nsec);
	//printf("%ld.%ld\n", t2.tv_sec, t2.tv_nsec);
	//printf("time_unit_usec: %ld\n\n", time_unit_usec);

	char policy[5];
	int num_of_process;
	scanf("%s", policy);
	scanf("%d", &num_of_process);
	struct Process *child_processes = malloc(num_of_process * sizeof(struct Process));
	for (int i=0; i<num_of_process; i++) {
		scanf("%s %d %d", child_processes[i].name, &child_processes[i].ready_time, &child_processes[i].exec_time);
	}
	/*printf("%s, %d", policy, num_of_process);
	for (int i=0; i<num_of_process; i++) {
		printf("%s, %d, %d", child_processes[i].name, child_processes[i].ready_time, child_processes[i].exec_time);
	}*/
	int pid = getpid();

	if (strcmp(policy, "FIFO")==0) {
		int *interval = malloc(num_of_process * sizeof(int));
		interval[0] = child_processes[0].ready_time;
		//child_processes[0].priority = 50;
		for (int i=1;i<num_of_process;i++) {
			interval[i] = child_processes[i].ready_time - child_processes[i-1].ready_time;
			//child_processes[i].priority = 50 - i;
		}
		int now_child = 0;
		while (pid!=0 && now_child!=num_of_process) {
			pid = fork();
			if (pid==0) {
				//printf("%d", getpid());
				struct timespec t_start, t_end;
				clock_gettime(CLOCK_REALTIME, &t_start);
				for (int i=0;i<child_processes[now_child].exec_time;i++) {
					volatile unsigned long tmp; for (tmp=0;tmp<1000000UL;tmp++);
				}
				clock_gettime(CLOCK_REALTIME, &t_end);
				syscall(333, getpid(), t_start.tv_sec, t_start.tv_nsec, t_end.tv_sec, t_end.tv_nsec);
			}
			else {
				struct sched_param param;
				//printf("a%da %d ", pid, now_child);
				param.sched_priority = 1;
				if (sched_setscheduler(pid, SCHED_FIFO, &param)) printf("need permision!");
				param.sched_priority = 50;
				if (sched_setscheduler(pid, SCHED_FIFO, &param)) printf("need permision!");
				child_processes[now_child].pid = pid;
				now_child += 1;
				if (now_child!=num_of_process) {
					//param.sched_priority = 99;
					//if (sched_setscheduler(getpid(), SCHED_FIFO, &param)) printf("need permision!");
					usleep(interval[now_child] * time_unit_usec);
				}
			}
		}

	}
	else if (strcmp(policy, "RR")==0) {
		int *interval = malloc(num_of_process * sizeof(int));
		interval[0] = child_processes[0].ready_time;
		//child_processes[0].priority = 50;
		for (int i=1;i<num_of_process;i++) {
			interval[i] = child_processes[i].ready_time - child_processes[i-1].ready_time;
			//child_processes[i].priority = 50;
		}
		int now_child = 0;
		while (pid!=0 && now_child!=num_of_process) {
			pid = fork();
			if (pid==0) {
				//printf("%d", getpid());
				struct timespec t_start, t_end;
				clock_gettime(CLOCK_REALTIME, &t_start);
				for (int i=0;i<child_processes[now_child].exec_time;i++) {
					if (i!=0 && i%500==0) sched_yield();
					volatile unsigned long tmp; for (tmp=0;tmp<1000000UL;tmp++);
				}
				clock_gettime(CLOCK_REALTIME, &t_end);
				syscall(333, getpid(), t_start.tv_sec, t_start.tv_nsec, t_end.tv_sec, t_end.tv_nsec);
			}
			else {
				struct sched_param param;
				//printf("a%da %d ", pid, now_child);
				//sched_getparam(pid, &param); printf("original priority: %d\n", param.sched_priority);
				param.sched_priority = 1;
				if (sched_setscheduler(pid, SCHED_FIFO, &param)) printf("need permision!");
				param.sched_priority = 50;
				if (sched_setscheduler(pid, SCHED_FIFO, &param)) printf("need permision!");
				child_processes[now_child].pid = pid;
				now_child += 1;
				if (now_child!=num_of_process) {
					//param.sched_priority = 99;
					//if (sched_setscheduler(getpid(), SCHED_FIFO, &param)) printf("need permision!");
					usleep(interval[now_child] * time_unit_usec);
				}
			}
		}

	}
	else if (strcmp(policy, "SJF")==0) {
		int *interval = malloc(num_of_process * sizeof(int));
		interval[0] = child_processes[0].ready_time;
		child_processes[0].priority = 50;
		//child_processes[0].priority = 50;
		for (int i=1;i<num_of_process;i++) {
			interval[i] = child_processes[i].ready_time - child_processes[i-1].ready_time;
			int time_less_than = 0;
			for (int j=1;j<num_of_process;j++) {
				if (child_processes[i].exec_time < child_processes[j].exec_time) time_less_than++;
			}
			child_processes[i].priority = time_less_than + 2;
		}
		int now_child = 0;
		while (pid!=0 && now_child!=num_of_process) {
			pid = fork();
			if (pid==0) {
				param.sched_priority = 90;
				if (sched_setscheduler(getpid(), SCHED_FIFO, &param)) printf("need permision!");
				//printf("%d", getpid());
				struct timespec t_start, t_end;
				clock_gettime(CLOCK_REALTIME, &t_start);
				for (int i=0;i<child_processes[now_child].exec_time;i++) {
					volatile unsigned long tmp; for (tmp=0;tmp<1000000UL;tmp++);
				}
				clock_gettime(CLOCK_REALTIME, &t_end);
				syscall(333, getpid(), t_start.tv_sec, t_start.tv_nsec, t_end.tv_sec, t_end.tv_nsec);
			}
			else {
				struct sched_param param;
				//printf("a%da %d ", pid, now_child);
				//sched_getparam(pid, &param); printf("original priority: %d\n", param.sched_priority);
				param.sched_priority = 1;
				if (sched_setscheduler(pid, SCHED_FIFO, &param)) printf("need permision!");
				param.sched_priority = child_processes[now_child].priority;
				if (sched_setscheduler(pid, SCHED_FIFO, &param)) printf("need permision!");
				child_processes[now_child].pid = pid;
				now_child += 1;
				if (now_child!=num_of_process) {
					//param.sched_priority = 99;
					//if (sched_setscheduler(getpid(), SCHED_FIFO, &param)) printf("need permision!");
					usleep(interval[now_child] * time_unit_usec);
				}
			}
		}
	}
	else if (strcmp(policy, "PSJF")==0) {
		int *interval = malloc(num_of_process * sizeof(int));
		interval[0] = child_processes[0].ready_time;
		//child_processes[0].priority = 50;
		for (int i=1;i<num_of_process;i++) {
			interval[i] = child_processes[i].ready_time - child_processes[i-1].ready_time;
			/*int time_less_than = 0;
			for (int j=1;j<num_of_process;j++) {
				if (child_processes[i].exec_time < child_processes[j].exec_time) time_less_than++;
			}
			child_processes[i].priority = time_less_than + 2;*/
		}
		int now_child = 0;
		int *sorted_exec_time = malloc(num_of_process * sizeof(int));
		for (int i=0;i<num_of_process;i++) sorted_exec_time[i] = child_processes[i].exec_time;
		for (int i=0;i<num_of_process;i++) {
			for (int j=i+1;j<num_of_process;j++) {
				if (sorted_exec_time[j] > sorted_exec_time[i]) {
					int tmp = sorted_exec_time[i];
					sorted_exec_time[i] = sorted_exec_time[j];
					sorted_exec_time[j] = tmp;
				}
			}
		}
		for (int i=0;i<num_of_process;i++) {
			for (int j=0;j<num_of_process;j++) {
				if (child_processes[i].exec_time > sorted_exec_time[j]) {
					child_processes[i].priority = j+1;
					break;
				}
				if (j==num_of_process-1) {
					child_processes[i].priority = j+2;
				}
			}
		}
		while (pid!=0 && now_child!=num_of_process) {
			pid = fork();
			if (pid==0) {
				int priority = 0;
				int exec_time = child_processes[now_child].exec_time;
				//printf("%d", getpid());
				struct timespec t_start, t_end;
				clock_gettime(CLOCK_REALTIME, &t_start);
				for (int i=0;i<child_processes[now_child].exec_time;i++) {
					volatile unsigned long tmp; for (tmp=0;tmp<1000000UL;tmp++);
					for (int j=0;j<num_of_process;j++) {
						if (exec_time-i-1 > sorted_exec_time[j]) {
							if (j + 1==priority) break;
							else {
								param.sched_priority = j + 1;
								if (sched_setscheduler(getpid(), SCHED_FIFO, &param)) printf("need permision!");
								priority = j + 1;
								break;
							}
						}
						if (j==num_of_process-1) {
							if (j+2!=priority) {
								param.sched_priority = j + 2;
								if (sched_setscheduler(getpid(), SCHED_FIFO, &param)) printf("need permision!");
								priority = j + 2;
							}
						}
					}
				}
				clock_gettime(CLOCK_REALTIME, &t_end);
				syscall(333, getpid(), t_start.tv_sec, t_start.tv_nsec, t_end.tv_sec, t_end.tv_nsec);
			}
			else {
				struct sched_param param;
				//printf("a%da %d ", pid, now_child);
				//sched_getparam(pid, &param); printf("original priority: %d\n", param.sched_priority);
				param.sched_priority = 1;
				if (sched_setscheduler(pid, SCHED_FIFO, &param)) printf("need permision!");
				param.sched_priority = child_processes[now_child].priority;
				if (sched_setscheduler(pid, SCHED_FIFO, &param)) printf("need permision!");
				child_processes[now_child].pid = pid;
				now_child += 1;
				if (now_child!=num_of_process) {
					//param.sched_priority = 99;
					//if (sched_setscheduler(getpid(), SCHED_FIFO, &param)) printf("need permision!");
					usleep(interval[now_child] * time_unit_usec);
				}
			}
		}

	}
	else {
		printf("policy not known!!");
	}

	if (pid!=0) {
		//printf("\n");
		for (int i=0;i<num_of_process;i++) {
			printf("%s %d\n", child_processes[i].name, child_processes[i].pid);
		}
	}

	return 0;

}