// TODO: Interface is given as parameter most of the times although there is a macro for the interface. Previous daemon implementations wanted to allow for multiple interface montior in one daemon. This is now impossible -> replace interface parameters with INTERFACE macro.
/*
 * config_changer.h
 * Header for the wireguard daemon that enables changes to the PSK in /etc/wireguard/wgx.conf
 */

#ifndef CONFIG_CHANGER_H
#define CONFIG_CHANGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "settings.h"
#include "parser.h"

void getPassword                (char*                       );
void get_timestamp              (char*                       );
void init_psk_hsm               (char*,  char*               );
void init_psk_hsm_timestamp     (char*,  char*,         char*);
void reset_psk_hsm              (char*,  char*               );
void reset_psk_hsm_timestamp    (char*,  char*,         char*);
void init_psk                   (char*,  char*               );
void reset_psk                  (char*,  char*               );
int  config_change              (char*,  char*,         char*);
void reload_config              (char*,    int, struct Config);
int  get_file_length            (FILE*                       );
void write_oldpsk_to_js         (char*                       );
void write_pin_to_js            (char*                       );


/*
 * Function to get Password from user input without displaying the password on console.
 *
 * @para password: pointer to char array with size PIN_SIZE
 */
void getPassword(char password[])
{
	static struct termios oldt, newt;
    	int i = 0;
    	int c;

    	/*saving the old settings of STDIN_FILENO and copy settings for resetting*/
    	tcgetattr( STDIN_FILENO, &oldt);
    	newt = oldt;

    	/*setting the approriate bit in the termios struct*/
    	newt.c_lflag &= ~(ECHO);

    	/*setting the new bits*/
    	tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    	/*reading the password from the console*/
    	while ((c = getchar())!= '\n' && c != EOF && i < PIN_SIZE){
	        password[i++] = c;
    	}
    	password[i] = '\0';

    	/*resetting our old STDIN_FILENO*/
    	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}


/*
 * Function to get timestamp with hour accuracy.
 *
 * @para timestamp: string for timestamp
 */
void get_timestamp(char *timestamp) {
	time_t ltime;
	ltime = time(NULL);
	struct tm *tm;
	tm = localtime(&ltime);

	// TODO: Check if tm->tm_mon for January really is zero. In Februry it returns 1.
	sprintf(timestamp, "%04d.%02d.%02d.%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour);
}

/*
 * Function to init the PSK with HSM(INIT_PSK) without reload config wireguard command.
 *
 * @para interface: string of interface to identify correct config file
 * @para pk:        public key to identify peer
 */
void init_psk_hsm(char *interface, char *pk) {
	printf("Starting init_psk_hsm...\n");

	char pin[PIN_SIZE];
	if(ENABLE_SECUREMODE == "y" && ENABLE_HSM == "y") {
		/* Get PIN for HSM access */
		printf( BOLDRED "Enter the PIN for the HSM: \n" RESET);
		getPassword(pin);
		/* Write PIN to java script file that needs it */
		write_pin_to_js(pin);
	}
	
	/* Write INIT_PSK to js for scsh3 execution */
	write_oldpsk_to_js(INIT_PSK);
	/* Execute js script with scsh3 and the help of expect */
	char command1[BUF_MEDIUM];
	snprintf(command1, sizeof(command1), "bash -c \"cd %s; expect wireguard_daemon.expect;\"", SCSH_DIR);
	FILE *fp;
        char line[BUF_BIG];
        /* Open the command for reading. */
        fp = popen(command1, "r");
        if (fp == NULL) {
                printf( RED "[ERROR] Failed to run command\n" RESET);
                exit(EXIT_FAILURE);
        }
        /* Read the output a line at a time - output it. */
        while (fgets(line, sizeof(line), fp) != NULL) {
		/* Javascript for scsh3 prints new PSK as "++++NEWPSK\n" */
              	if(strstr(line, "++++") != NULL) {
			break;
		}
        }
        /* Close */
        pclose(fp);
	/* Get new psk from line */
	char psk[BUF_SMALL];
	/* PSK starts at 5. character */
	memcpy(psk, &line[5], strlen(line)-5);
	/* Remove new line */
	psk[strlen(psk)-1] = '\0';

	if(ENABLE_SECUREMODE == "y" && ENABLE_HSM == "y") {
		/* Remove PIN from java script file */
		write_pin_to_js("654321");
		/* Override array with PIN to make sure it is gone */
		for(int idx = 0; idx < strlen(pin); idx++){
			pin[idx] = '\0';
		}
	}

	/* Load new psk to config file */
	int ret = config_change(interface, pk, psk);
	if(ret != 0) printf( RED "[ERROR] config_change failed\n" RESET);
	printf("PSK was init\n");
	printf("\tnew psk: %s\n", psk);
	printf("\tprocess: %d\n", getpid());
}

/*
 * Function to init the PSK with HSM(TIMESTAMP) without reload config wireguard command.
 *
 * @para interface: string of interface to identify correct config file
 * @para pk:        public key to identify peer
 * @para timestamp: string of timestamp
 */
void init_psk_hsm_timestamp(char *interface, char *pk, char *timestamp) {
	printf("Starting init_psk_hsm_timestamp...\n");

	char pin[PIN_SIZE];
	if(ENABLE_SECUREMODE == "y" && ENABLE_HSM == "y") {
		/* Get PIN for HSM access */
		printf( BOLDRED "Enter the PIN for the HSM: \n" RESET );
		getPassword(pin);
		/* Write PIN to java script file that needs it */
		write_pin_to_js(pin);
	}

	/* Write timestamp to js for scsh3 execution */
	write_oldpsk_to_js(timestamp);
	/* Execute js script with scsh3 and the help of expect */
	char command1[BUF_MEDIUM];
	snprintf(command1, sizeof(command1), "bash -c \"cd %s; expect wireguard_daemon.expect;\"", SCSH_DIR);
	FILE *fp;
        char line[BUF_BIG];
        /* Open the command for reading. */
        fp = popen(command1, "r");
        if (fp == NULL) {
                printf("Failed to run command\n" );
                exit(EXIT_FAILURE);
        }
        /* Read the output a line at a time - output it. */
        while (fgets(line, sizeof(line), fp) != NULL) {
		/* Javascript for scsh3 prints new PSK as "++++NEWPSK\n" */
              	if(strstr(line, "++++") != NULL) {
			break;
		}
        }
        /* Close */
        pclose(fp);
	/* Get new psk from line */
	char psk[BUF_SMALL];
	/* PSK starts at 5. character */
	memcpy(psk, &line[5], strlen(line)-5);
	/* Remove new line */
	psk[strlen(psk)-1] = '\0';

	if(ENABLE_SECUREMODE == "y" && ENABLE_HSM == "y") {
		/* Remove PIN from java script file */
		write_pin_to_js("654321");
		/* Override array with PIN to make sure it is gone */
		for(int idx = 0; idx < strlen(pin); idx++){
			pin[idx] = '\0';
		}
	}

	/* Load new psk to config file */
	int ret = config_change(interface, pk, psk);
	if(ret != 0) printf( RED "[ERROR] config_change failed\n" RESET);
	printf("PSK was init\n");
	printf("\tnew psk: %s\n", psk);
	printf("\ttimestamp: %s\n", timestamp);
	printf("\tprocess: %d\n", getpid());
}

/*
 * Function to reset the PSK with HSM(INIT_PSK).
 *
 * @para interface: string of interface to identify correct config file
 * @para pk:        public key to identify peer
 */
void reset_psk_hsm(char *interface, char *pk) {
	printf("\tStarting reset_psk_hsm...\n");

	char pin[PIN_SIZE];
	if(ENABLE_SECUREMODE == "y" && ENABLE_HSM == "y") {
		/* Get PIN for HSM access */
		printf( BOLDRED "\tEnter the PIN for the HSM: \n" RESET);
		getPassword(pin);
		/* Write PIN to java script file that needs it */
		write_pin_to_js(pin);
	}

	/* Write INIT_PSK to js for scsh3 execution */
	write_oldpsk_to_js(INIT_PSK);
	/* Execute js script with scsh3 and the help of expect */
	char command1[BUF_MEDIUM];
	snprintf(command1, sizeof(command1), "bash -c \"cd %s; expect wireguard_daemon.expect;\"", SCSH_DIR);
	FILE *fp;
        char line[BUF_BIG];
        /* Open the command for reading. */
        fp = popen(command1, "r");
        if (fp == NULL) {
                printf( RED "[ERROR] Failed to run command\n" RESET);
                exit(EXIT_FAILURE);
        }
        /* Read the output a line at a time - output it. */
        while (fgets(line, sizeof(line), fp) != NULL) {
		/* Javascript for scsh3 prints new PSK as "++++NEWPSK\n" */
              	if(strstr(line, "++++") != NULL) {
			break;
		}
        }
        /* Close */
        pclose(fp);

	/* Get new psk from line */
	char psk[48];
	/* Get new psk from line */
	memcpy(psk, &line[5], strlen(line)-5);
	/* Remove new line */
	psk[strlen(psk)-1] = '\0';
	
	if(ENABLE_SECUREMODE == "y" && ENABLE_HSM == "y") {
		/* Remove PIN from java script file */
		write_pin_to_js("654321");
		/* Override array with PIN to make sure it is gone */
		for(int idx = 0; idx < strlen(pin); idx++){
			pin[idx] = '\0';
		}
	}

	/* Load new psk to config file and reload config in wireguard */
	int ret = config_change(interface, pk, psk);
	if(ret != 0) printf( RED "[ERROR] config_change failed\n" RESET);
	/* Reload config file in wireguard */
	char command2[BUF_MEDIUM];
	snprintf(command2, sizeof(command2), "sudo bash -c \"wg addconf %s <(wg-quick strip %s)\"", interface, interface);
	system(command2);
	printf("\tPSK was reseted\n");
	printf("\t\tnew psk: %s\n", psk);
	printf("\t\tprocess: %d\n", getpid());
}

/*
 * Function to reset the PSK with HSM(TIMESTAMP).
 *
 * @para interface: string of interface to identify correct config file
 * @para pk:        public key to identify peer
 * @para timestamp: string of timestamp
 */
void reset_psk_hsm_timestamp(char *interface, char *pk, char *timestamp) {
	printf("\tStarting reset_psk_hsm_timestamp...\n");

	char pin[PIN_SIZE];
	if(ENABLE_SECUREMODE == "y" && ENABLE_HSM == "y") {
		/* Get PIN for HSM access */
		printf( BOLDRED "\tEnter the PIN for the HSM: \n" RESET);
		getPassword(pin);
		/* Write PIN to java script file that needs it */
		write_pin_to_js(pin);
	}

	/* Write timestamp to js for scsh3 execution */
	write_oldpsk_to_js(timestamp);
	/* Execute js script with scsh3 and the help of expect */
	char command1[BUF_MEDIUM];
	snprintf(command1, sizeof(command1), "bash -c \"cd %s; expect wireguard_daemon.expect;\"", SCSH_DIR);
	FILE *fp;
        char line[BUF_BIG];
        /* Open the command for reading. */
        fp = popen(command1, "r");
        if (fp == NULL) {
                printf( RED "[ERROR] Failed to run command\n" RESET);
                exit(EXIT_FAILURE);
        }
        /* Read the output a line at a time - output it. */
        while (fgets(line, sizeof(line), fp) != NULL) {
		/* Javascript for scsh3 prints new PSK as "++++NEWPSK\n" */
              	if(strstr(line, "++++") != NULL) {
			break;
		}
        }
        /* Close */
        pclose(fp);

	/* Get new psk from line */
	char psk[48];
	/* Get new psk from line */
	memcpy(psk, &line[5], strlen(line)-5);
	/* Remove new line */
	psk[strlen(psk)-1] = '\0';

	if(ENABLE_SECUREMODE == "y" && ENABLE_HSM == "y") {
		/* Remove PIN from java script file */
		write_pin_to_js("654321");
		/* Override array with PIN to make sure it is gone */
		for(int idx = 0; idx < strlen(pin); idx++){
			pin[idx] = '\0';
		}
	}

	/* Load new psk to config file and reload config in wireguard */
	int ret = config_change(interface, pk, psk);
	if(ret != 0) printf( RED "[ERROR] config_change failed\n" RESET );
	/* Reload config file in wireguard */
	char command2[BUF_MEDIUM];
	snprintf(command2, sizeof(command2), "sudo bash -c \"wg addconf %s <(wg-quick strip %s)\"", interface, interface);
	system(command2);
	printf("\tPSK was reseted\n");
	printf("\t\tnew psk: %s\n", psk);
	printf("\t\tprocess: %d\n", getpid());
}

/*
 * Function to init the PSK without HSM(INIT_PSK) and without calling wireguard reload command.
 *
 * @para interface: string of interface to identify correct config file
 * @para pk:        public key to identify peer
 */
void init_psk(char *interface, char *pk) {
	printf("Starting init_psk...\n");
        int ret = config_change(interface, pk, RESET_PSK);
	if(ret != 0) printf( RED "[ERROR] config_change failed\n" RESET );
	printf("PSK was init\n");
        printf("\tnew psk: %s\n", RESET_PSK);
}

/*
 * Function to reset the PSK without HSM(INIT_PSK).
 *
 * @para interface: string of interface to identify correct config file
 * @para pk:        public key to identify peer
 */
void reset_psk(char *interface, char *pk) {
	printf("\tStarting reset_psk...\n");
	int ret = config_change(interface, pk, RESET_PSK);
	if(ret != 0) printf( RED "[ERROR] config_change failed\n" RESET );
	char command[BUF_MEDIUM];
	snprintf(command, sizeof(command), "sudo bash -c \"wg addconf %s <(wg-quick strip %s)\"", interface, interface);
	system(command);
	printf("\tPSK was reseted\n");
	printf("\t\tnew psk: %s\n", RESET_PSK);
}

/*
 * Function replaces pre-shared key of peer with corresponding public key.
 *
 * @para interface: string with interface to identify the right config file
 * @para pk:        public key of peer that needs to have psk replaced
 * @para psk:       new preshared key that needs to be inserted
 */
int config_change(char *interface, char *pk, char *psk) {
	/* Open wireguard config file */
	char filepath[128];
	snprintf(filepath, sizeof(filepath), "%s/%s.conf", WIREGUARD_DIR, interface);
	FILE *config_file;
	config_file = fopen(filepath, "r");
	if(config_file == NULL)
	{
		printf( RED "[ERROR] Couldn't open file %s\n" RESET, filepath);
		return 1;
	}

	/* Read file into string array */
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	/* memorize important lines with psk_line being the psk of the correct peer */
	int psk_line = -1; int idx = 0; bool right_peer_found = false;

	/* construct PK line that needs to be search for */
	char pk_line[BUF_MEDIUM];
	snprintf(pk_line, sizeof(pk_line), "PublicKey = %s", pk);

	while((read = getline(&line, &len, config_file)) != -1) {
		/* ignore commented lines */
		if(line[0] != '#') {
			if(strstr(line, pk_line) != NULL) {
				right_peer_found = true;
			}
			if(strstr(line, "[Peer]") != NULL) {
				right_peer_found = false;
			}
			if(right_peer_found) {
				if(strstr(line, "PresharedKey") != NULL) {
					/* Check if already PSK found */
					if(psk_line != -1) {
						printf( RED "[ERROR] At least two possible PresharedKeys for one peer have been found.\n" RESET );
						return 1;
					}else{
						psk_line = idx;
						break;
					}
				}
			}
		}
		/* next line */
		idx++;
	}

	if(psk_line == -1) {
		printf( RED "[ERROR] The psk_line is -1. This means no PresharedKey was found for given public key %s.\n" RESET, pk);
		return 1;
	}
	fclose(config_file);
	if(line) free(line);

	/* Execute shell command to replace line psk_line+1 with psk */
	char command[BUF_MEDIUM];
	snprintf(command, sizeof(command), "sed -i \"%ds|.*|PresharedKey = %s|\" %s", psk_line+1, psk, filepath);
	system(command);
	return 0;
}


/*
 * Function to reload config with new PSK by hashing (SHA256) old PSK in Base64 format.
 *
 * @para interface: string with interface to identify the right config file
 * @para pk:        public key of peer that needs to be reloaded
 */
void reload_config(char *interface, int peer, struct Config config) {
        printf( GREEN "[PEER_%d]" RESET " Received signal to reload config file with new PSK (no HSM)...\n", peer+1);
	/* Reload PSK with old PSK hashed with sha256sum */
	char command[BUF_MEDIUM];
        snprintf(command, sizeof(command), "echo \"%s\" | tr -d \"\n\r\" | sha256sum | cut -d ' ' -f 1 | xxd -r -p | base64", config.peers[peer].psk);
        FILE *fp;
        char line[BUF_BIG];
        /* Open the command for reading. */
        fp = popen(command, "r");
        if (fp == NULL) {
                printf( RED "[ERROR] Failed to run command\n" RESET );
                exit(1);
        }
        /* Read the output. Should only be one line. Nonetheless read all until last line */
        if(fgets(line, sizeof(line), fp) == NULL) {
		printf( RED "[reload_config] Error occured during reading output of command for calucalting new PSK.\n" RESET );
		pclose(fp);
		return;
	}
        /* Close */
        pclose(fp);

	/* Remove new line */
        line[strlen(line)-1] = '\0';
        int ret = config_change(interface, config.peers[peer].pubKey, line);
	if(ret != 0) printf( RED "[ERROR] config_change failed\n" RESET);
	/* Excute shell command to reload config for wireguard */
        system("sudo bash -c \"wg addconf wg0 <(wg-quick strip wg0)\"");
	printf( GREEN "[PEER_%d]" RESET " Reloaded psk with %s from old %s\n", peer+1, line, config.peers[peer].psk);
}


// TODO: Better implementation of helper functions (less sticks and stones)
/* ------------------------------------ begin helper functions ---------------------------------------------*/
/* FILE MUST BE OPEN TO READ */
int get_file_length(FILE *fp) {
        int lines = 0; char ch;
        while(!feof(fp)) {
                ch = fgetc(fp);
                if(ch == '\n')
                        lines++;
        }
        fseek(fp, 0, SEEK_SET);
        return lines;
}

/* Function to write oldpsk wireguard_daemon.js that is used for HSM access */
void write_oldpsk_to_js(char *oldpsk) {
	char command[BUF_MEDIUM];
	snprintf(command, sizeof(command), "sed -i 's|var oldpsk =.*|var oldpsk = \"%s\";|g' %s/wireguard_daemon.js", oldpsk, SCSH_DIR);
	system(command);
}

/* Function to write PIN to wireguard_daemon.js that is needed for HSM */
void write_pin_to_js(char *pin) {
	char command[BUF_MEDIUM];
	snprintf(command, sizeof(command), "sed -i 's|sc.verifyUserPIN.*|sc.verifyUserPIN(new ByteString(\"%s\", ASCII));|g' %s/wireguard_daemon.js", pin, SCSH_DIR);
	system(command);
}
/* -------------------------------------- end helper functions ---------------------------------------------*/

#endif
