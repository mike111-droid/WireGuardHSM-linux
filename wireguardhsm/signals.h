// TODO: Allow other signal communication with the program
/*
 * signals.h
 * Defines signal callbacks
 */

#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>
#include "config_changer.h"
#include "settings.h"

/* Signal_callback_handler for main process */
void signal_callback_handler_main(int signum) {
	printf("[MAIN] Signal %d caught in process with pid %ld...\n", signum, (long)getpid());
	if(signum == SIGINT) {
		/* Shutdown interface */
		char command[BUF_MEDIUM];
		snprintf(command, sizeof(command), "sudo wg-quick down %s", INTERFACE);
		system(command);
		/* Delete message queue */
		key_t key_main = ftok(WORK_DIR, 64);
		int msgid_main = msgget(key_main, 0666);
		if(msgctl(msgid_main, IPC_RMID, NULL) == -1) {
			fprintf(stderr, "Message queue could not be deleted.\n");
			exit(EXIT_FAILURE);
		}
		if(ENABLE_SECUREMODE != "y" && ENABLE_HSM == "y") {
			write_pin_to_js_all("654321");	
		}
		printf("[MAIN] Message queue was deleted\n");
		/* Disabling dynamic debug */
		system("echo 'module wireguard -p' | sudo tee /sys/kernel/debug/dynamic_debug/control");
		/* Exit program */
		exit(SIGINT);
	}
}

/* Signal_callback_handler for monitor processes */
void signal_callback_handler_monitor(int signum) {
	printf("[MONITOR] Signal %d caught in process with pid %ld...\n", signum, (long)getpid());
	if(signum == SIGINT) {
		exit(SIGINT);
	}
}

#endif //SIGNALS_H
