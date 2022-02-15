/* test.c */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include "parser.h"
#include "config_changer.h"

int main() {
	config_change("wg0", "XJhoQ9+pQHyF5ENYGyzUq1IFZPA4tIVVMij6/DztXks=", "Hallo");
	//printf("[test] address: %s\n", config.interface.address);
	//signal(TIMER_ENDED, signal_callback_handler_main);
	/*struct sigaction action;
        action.sa_flags = SA_SIGINFO;
        action.sa_sigaction = &signal_action;
        if (sigaction(TIMER_ENDED, &action, NULL) == -1) {
                perror("sigusr: sigaction");
                return 0;
        }

	pid_t connection_pids[10] = { 0 };
	pid_t **shmem_pids = mmap(NULL, 10 * sizeof(pid_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	for(int idx = 0; idx < 10; idx++) {
		shmem_pids[idx] = (pid_t*) mmap(NULL, sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		*shmem_pids[idx] =  123412;
	}

	struct Connection connection;
	connection.whoami = -100;
	struct Connection *connection_shmem = mmap(NULL, sizeof(struct Connection), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        memcpy(connection_shmem, &connection, sizeof(struct Connection));
	printf("connection.whoami: %d\n", connection.whoami);
	printf("connection_shmem.whoami: %d\n", connection_shmem->whoami);

	if(fork() == 0) {
		union sigval value;
		value.sival_ptr = (void*) shmem_pids[0];
		printf("pid before: %d\n", (int) *shmem_pids[0]);
		sigqueue(getpid(), TIMER_ENDED, value);
		printf("pid after: %d\n", (int) *shmem_pids[0]);
	}else{
		while(1) {
			sleep(1);
			printf("pid from parent after: %d\n", (int) *shmem_pids[0]);
		}
	}*/
}

