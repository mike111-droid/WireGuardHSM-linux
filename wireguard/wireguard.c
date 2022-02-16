// TODO: Check what headers really are needed
/*
 * wiregaurd.c
 * Not as daemon but as console application. The PSK change happens automatically.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include "types.h"
#include "settings.h"
#include "config_changer.h"
#include "signals.h"
#include "parser.h"

/*
 * Function to monitor dynamic debug output and sending it to message queue. Is called as new process. Is only killed with SIGINT (console: CTRL+C)
 *
 * @para msgid: id of the message queue
 */
void monitor_log(int msgid) {
        /* monitor all wiregaurd handshake messages and put in msg queue */
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        FILE *kernLogFile = popen("tail -n 5 -f /var/log/kern.log", "r");
        while(1) {
                if(getline(&line, &len, kernLogFile) == -1) {
                        printf("[MONITOR] An error occured while reading from /var/log/kern.log\n");
			continue;
                }
		ipcMsg.msgType = -1;
                /* put lines of wireguard handshakes into message queue with correct types */
		if((strstr(line, "wireguard") != NULL) && (strstr(line, "Sending handshake initiation to peer") != NULL)) {
			ipcMsg.msgType = INIT_SND;
		}
		if((strstr(line, "wireguard") != NULL) && (strstr(line, "Receiving handshake initiation from peer") != NULL)) {
			ipcMsg.msgType = INIT_RCV;
		}
		if((strstr(line, "wireguard") != NULL) && (strstr(line, "Sending handshake response to peer") != NULL)) {
			ipcMsg.msgType = RESP_SND;
		}
		if((strstr(line, "wireguard") != NULL) && (strstr(line, "Receiving handshake response from peer") != NULL)) {
			ipcMsg.msgType = RESP_RCV;
		}
		if(ipcMsg.msgType != -1) {
			if(MSG_LEN >= len) {
				memcpy(ipcMsg.msg, line, MSG_LEN);
			}else{
				printf("[MONITOR] The line is too big for buffer of ipcMsg. Ignoring it...\n");
				continue;
			}
			if(msgsnd(msgid, &ipcMsg, MSG_LEN, 0) == -1) {
				printf("[MONITOR] Error during writing to the message queue. Do something about that...;-)\n");
			}
		}
        }
}

int main() {
	// Necessary strings to identify messages
	int len_sending_init_handshake_line = strlen("wireguard: wgXXXXXXXXXXXXXXXXXXX: Sending handshake initiation to peer");
	char sending_init_handshake_line[len_sending_init_handshake_line];
	snprintf(sending_init_handshake_line, len_sending_init_handshake_line, "wireguard: %s: Sending handshake initiation to peer", INTERFACE);
	int len_rcv_init_handshake_line = strlen("wireguard: wgXXXXXXXXXXXXXXXXXXX: Receiving handshake initiation from peer");
	char rcv_init_handshake_line[len_rcv_init_handshake_line];
	snprintf(rcv_init_handshake_line, len_rcv_init_handshake_line, "wireguard: %s: Receiving handshake initiation from peer", INTERFACE);
	int len_rcv_resp_handshake_line = strlen("wireguard: wgXXXXXXXXXXXXXXXXXXX: Receiving handshake response from peer");
	char rcv_resp_handshake_line[len_rcv_resp_handshake_line];
	snprintf(rcv_resp_handshake_line, len_rcv_resp_handshake_line, "wireguard: %s: Receiving handshake response from peer", INTERFACE);
	int len_sending_resp_handshake_line = strlen("wireguard: wgXXXXXXXXXXXXXXXXXXX: Sending handshake response to peer");
	char sending_resp_handshake_line[len_sending_resp_handshake_line];
	snprintf(sending_resp_handshake_line, len_sending_resp_handshake_line, "wireguard: %s: Sending handshake response to peer", INTERFACE);

	// enable dynamic debug
	system("echo 'module wireguard +p' | sudo tee /sys/kernel/debug/dynamic_debug/control");

	/* Create string commands to start wireguard interface */
	char up_interface[BUF_MEDIUM];
        snprintf(up_interface, sizeof(up_interface), "sudo wg-quick up %s", INTERFACE);

	/* prevent monitor process to read old handshakes by writing the last 5 lines with + */
	for(int idx = 0; idx < 5; idx++) {
		system("sudo bash -c \"echo + >> /var/log/kern.log\"");
	}

	struct Config config;
	initConfig(&config, INTERFACE);
	char timestamp[BUF_TIMSTMP];
	get_timestamp(timestamp);
	printf("timestamp: %s\n", timestamp);

	/* reset all peers with according PK that need to be monitored */
	if(ENABLE_HSM == "y") {
		// use hsm functions
		for(int idx = 0; idx < MAX_PEERS; idx++) {
			if(strlen(config.peers[idx].pubKey) == 0) {
				printf("This peer is empty\n");
				continue;
			}
			if(ENABLE_TIMESTAMP == "y") {
				init_psk_hsm_timestamp(INTERFACE, config.peers[idx].pubKey, timestamp);
			}else{
				init_psk_hsm(INTERFACE, config.peers[idx].pubKey);
			}
		}
	}else{
		// use no hsm functions
		for(int idx = 0; idx < MAX_PEERS; idx++) {
			if(strlen(config.peers[idx].pubKey) == 0) {
				printf("This peer is empty\n");
				continue;
			}
			init_psk(INTERFACE, config.peers[idx].pubKey);
		}
	}


	/* create message queue to filter all relevant wirguard messages form dynamic debug output */
	key_t key_main = ftok(WORK_DIR, 64);
	int msgid_main = msgget(key_main, IPC_CREAT);

	if(fork() == 0) {
		signal(SIGINT, signal_callback_handler_monitor);
		monitor_log(msgid_main);
	}else{
		signal(SIGINT, signal_callback_handler_main);
	}

	/* Update config */
	parseConfig(&config, INTERFACE);
	/* Start wireguard */
	system(up_interface);

	/* monitor all peer connections */
	char port[48]; char endpoint[512]; bool msgProcessed = false; char oldTimestamp[BUF_TIMSTMP];
	while(1) {
		/* Update internal config */
		parseConfig(&config, INTERFACE);
		/* Monitor all messages form message queue */
		if(msgrcv(msgid_main, &ipcMsg, sizeof(ipcMsg), 0, IPC_NOWAIT) == -1) {
                        if(errno != ENOMSG) {
                                printf("[MAIN:%d] Error receiving message: %s\n", getpid(), strerror(errno));
			}
                }else{
                        printf("[MAIN:%d] msgType %d\n%s", getpid(), ipcMsg.msgType, ipcMsg.msg);
			/* mark message as not processed */
			msgProcessed = false;
                }
		for(int peer = 0; peer < config.peerNum; peer++) {
			/* Check if peer is empty or not */
			if(strlen(config.peers[peer].pubKey) == 0) {
                                //printf("This peer is empty\n");
                                continue;
                        }
			/* Check if timestamp changed. If yes reload with HSM(TIMESTAMP). */
			memcpy(oldTimestamp, timestamp, strlen(timestamp));
			get_timestamp(timestamp);
			if(strcmp(oldTimestamp, timestamp) != 0) {
				printf("[MAIN] Timestamp changed to %s...\n", timestamp);
				if(ENABLE_HSM == "y") {
					if(ENABLE_TIMESTAMP == "y") {
						reset_psk_hsm_timestamp(INTERFACE, config.peers[peer].pubKey, timestamp);
						config.peers[peer].reloaded = true;
					}
       		                }else{
       	       		       		reset_psk(INTERFACE, config.peers[peer].pubKey);
					config.peers[peer].reloaded = true;
                    		}
			}
			snprintf(endpoint, sizeof(endpoint), "%s:%d", config.peers[peer].endpoint, config.peers[peer].endpointPort);
			/* determine role */
			bool endpointFound = (strstr(ipcMsg.msg, endpoint) != NULL);
			if(endpointFound == true) {
				if(msgProcessed == false) {
					/* Set connectionStarted to true */
					config.peers[peer].connectionStarted = true;
					if((ipcMsg.msgType == INIT_SND) || (ipcMsg.msgType == RESP_RCV)) {
        	                                /* Message gives peer INITIATOR role */
                	                        /* if role==RESPONDER or role==UNKNOWN set role to INITIATOR */
                        	                if((config.peers[peer].role == RESPONDER) || (config.peers[peer].role == UNKNOWN)) {
        	                                        printf("[MAIN] Role of peer has changed to INITIATOR...\n");
	       	                                        config.peers[peer].role = INITIATOR;
                	                                config.peers[peer].pskReload = clock();
                        	                        config.peers[peer].initHandshakeCounter = 0;
                                	                config.peers[peer].reloaded = false;
                                       		}
	                                }else{
        	                                /* Message gives peer RESPONDER role */
                	                        /* if role==INITIATOR or role==UNKNOWN set role to RESPONDER */
                        	                if((config.peers[peer].role == INITIATOR) || (config.peers[peer].role == UNKNOWN)) {
                                	                printf("[MAIN] Role of peer has changed to RESPONDER...\n");
                                        	        config.peers[peer].role = RESPONDER;
	                                                config.peers[peer].pskReload = clock();
        	                                        config.peers[peer].initHandshakeCounter = 0;
                	                                config.peers[peer].reloaded = false;
                        	                }
                                	}
				}
			}
			/* check reload time (every time) */
			if((((clock() - config.peers[peer].pskReload)/CLOCKS_PER_SEC) >= 60) && (config.peers[peer].reloaded == false) && (config.peers[peer].connectionStarted == true)) {
				printf("[MAIN] 60 seconds passed since successful handshake (or no init handshake has arrived in 60 seconds). Reloading PSK...\n");
	                  	reload_config(INTERFACE, peer, config);
               		        config.peers[peer].pskReload = clock();
	                        config.peers[peer].reloaded = true;
				config.peers[peer].initHandshakeCounter = 0;
      			}
			/* do INITIATOR behaviour if role==INITIATOR */
			if(config.peers[peer].role == INITIATOR) {
               		       /* See if initiator/myself is trying to do new handshake */
	                       if(strstr(ipcMsg.msg, sending_init_handshake_line) && (msgProcessed == false) && (endpointFound == true)) {
	                                /* count initHandshakeCounter for possible reset */
               		                config.peers[peer].initHandshakeCounter += 1;
					printf("[MAIN] Counting initHandshakeCounter to %d.\n", config.peers[peer].initHandshakeCounter);
                       		}
	                        /* See if initiator is getting valid handshake response */
               		        if(strstr(ipcMsg.msg, rcv_resp_handshake_line) && (msgProcessed == false) && (endpointFound == true)) {
        	                     	/* Handshake was successfully completed and msg has not yet been processed */
	                                printf("[MAIN] Resetting pskReload clock. Setting reloaded to false. Setting initHandshakeCounter to 0...\n");
	                                config.peers[peer].pskReload = clock();
	                                config.peers[peer].reloaded = false;
					config.peers[peer].initHandshakeCounter = 0;
               		        }
	                        /* if no valid handshake response after 6 handshake inits do reset */
               		        if(config.peers[peer].initHandshakeCounter >= 6) {
	                                printf("[MAIN] Reset because 6 init handshakes have not been anwsered validly...\n");
					if(ENABLE_HSM == "y") {
                                        	if(ENABLE_TIMESTAMP == "y") {
                                                	reset_psk_hsm_timestamp(INTERFACE, config.peers[peer].pubKey, timestamp);
	       	                                }else{
							reset_psk_hsm(INTERFACE, config.peers[peer].pubKey);
						}
                	                }else{
                        	                reset_psk(INTERFACE, config.peers[peer].pubKey);
	                                }
	                                config.peers[peer].initHandshakeCounter = 0;
                               	        config.peers[peer].reloaded = true;
	                        }
	                }
			/* do REPSONDER behaviour if role==RESPONDER */
			if(config.peers[peer].role == RESPONDER) {
	                        /* See if responder/myself has received init handshake */
               		        if(strstr(ipcMsg.msg, rcv_init_handshake_line) && (msgProcessed == false) && (endpointFound == true)) {
	                                config.peers[peer].pskReload = clock();
	                                config.peers[peer].reloaded = false;
	                                config.peers[peer].initHandshakeCounter += 1;
					printf("[MAIN] Counting initHandshakeCounter to %d.\n", config.peers[peer].initHandshakeCounter);
               		        }
	                        /* if handshake_completed false -> if init_handshake_counter is 6 do reset*/
               		        if(config.peers[peer].initHandshakeCounter >= 6) {
					if(ENABLE_HSM == "y") {
                                        	if(ENABLE_TIMESTAMP == "y") {
                                                	reset_psk_hsm_timestamp(INTERFACE, config.peers[peer].pubKey, timestamp);
        	                                }else{
							reset_psk_hsm(INTERFACE, config.peers[peer].pubKey);
						}
                	                }else{
                        	                reset_psk(INTERFACE, config.peers[peer].pubKey);
        	                        }
                                        config.peers[peer].reloaded = true;
	                                config.peers[peer].initHandshakeCounter = 0;
	        	        }
	              	}
		}
		/* mark message as processed */
		msgProcessed = true;
	}
	return 0;
}

