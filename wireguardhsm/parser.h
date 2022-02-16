/*
 * parser.h
 * Contains function necessary to parse wireguard config file to internal config struct.
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"

void parseAddress     (struct Config*,  char*       );
void parseListenPort  (struct Config*,  char*       );
void parsePublicKey   (struct Config*,  char*,   int);
void parsePresharedKey(struct Config*,  char*,   int);
void parseEndpoint    (struct Config*,  char*,   int);
void parseConfig      (struct Config*,  char*       );

/*
 * Function to parse address line of wireguard config file with format "Address = ________\n"
 *
 * @para config: pointer at config struct to store data in
 * @para line:   pointer at line
 */
void parseAddress(struct Config *config, char *line) {
	/* Remove new line */
	line[strlen(line)-1] = '\0';
	/* Walk through string and identify address */
	int counter = 0;
	char letter = line[counter];
	int idx = -1;
	while(letter != '\0') {
		//printf("original: %c\n", letter);
		if(idx >= 0 && letter != ' ') {
			config->interface.address[idx++] = letter;
			//printf("address: %c\n", config->interface.address[idx-1]);
		}
		if(letter == '=') idx++;
		letter = line[++counter];
	}
	config->interface.address[idx] = '\0';
	//printf("%s\n", config->interface.address);
}

/*
 * Function to parse ListenPort of wireguard config file with format "ListenPort = __________\n"
 *
 * @para config: pointer at config struct to store data in
 * @para line:   pointer at line
 */
void parseListenPort(struct Config *config, char *line) {
	/* Remove new line */
	line[strlen(line)-1] = '\0';
	/* Walk through line and identify port */
	char port[48];
	int counter = 0;
	char letter = line[counter];
	int idx = -1;
	while(letter != '\0') {
		//printf("original: %c\n", letter);
		if(idx >= 0 && letter != ' ') {
			port[idx++] = letter;
		}
		if(letter == '=') idx++;
		letter = line[++counter];
	}
	config->interface.listenPort = atoi(port);
	//printf("[parseListenPort] listenPort: %s\n", port);
}

/*
 * Function to parse PublicKey of wireguard config file with format "PublicKey = ______\n"
 *
 * @para config: pointer at config struct to store data in
 * @para line:   pointer at line
 * @para peer:   int number to identify which peer is worked on (first Peer in config file is 0, second is 1, ...)
 */
void parsePublicKey(struct Config *config, char *line, int peer) {
	/* Remove new line */
	line[strlen(line)-1] = '\0';
	/* Walk through line and identify Public Key */
	int counter = 0;
	char letter = line[counter];
	int idx = -1;
	while(letter != '\0') {
		//printf("original: %c\n", letter);
		if(idx >= 0 && letter != ' ') {
			config->peers[peer].pubKey[idx++] = letter;
		}
		if(letter == '=') idx++;
		letter = line[++counter];
	}
	config->peers[peer].pubKey[idx] = '\0';
	//printf("[parsePublicKey] publicKey: %s\n", config->peers[peer].pubKey);
}

/*
 * Function to parse PresharedKey of wireguard config file with format "PresharedKey = ______\n"
 *
 * @para config: pointer at config struct to store data in
 * @para line:   pointer at line
 * @para peer:   int number to identify which peer is worked on (first Peer in config file is 0, second is 1, ...)
 */
void parsePresharedKey(struct Config *config, char *line, int peer) {
	/* Remove new line */
	line[strlen(line)-1] = '\0';
	/* Walk through line and identify Preshared Key */
	int counter = 0;
	char letter = line[counter];
	int idx = -1;
	while(letter != '\0') {
		//printf("original: %c\n", letter);
		if(idx >= 0 && letter != ' ') {
			config->peers[peer].psk[idx++] = letter;
		}
		if(letter == '=') idx++;
		letter = line[++counter];
	}
	config->peers[peer].psk[idx] = '\0';
	//printf("[parsePresharedKey] psk: %s\n", config->peers[peer].psk);
}

/*
 * Function to parse Endpoint of wireguard config file with format "PublicKey = ________:------\n" (_ being ip address, - being port number)
 *
 * @para config: pointer at config struct to store data in
 * @para line:   pointer at line
 * @para peer:   int number to identify which peer is worked on (first Peer in config file is 0, second is 1, ...)
 */
void parseEndpoint(struct Config *config, char *line, int peer) {
	/* Remove new line */
	line[strlen(line)-1] = '\0';
	/* Walk through line and identify Endpoint and Endpoint Port */
	char port[48];
	int counter = 0;
	char letter = line[counter];
	int idx1 = -1;
	int idx2 = -1;
	while(letter != '\0') {
		//printf("original: %c\n", letter);
		if(idx1 >= 0 && letter != ' ' && idx2 == -1 && letter != ':') {
			config->peers[peer].endpoint[idx1++] = letter;
		}
		if(idx2 >= 0 && letter != ':') {
			port[idx2++] = letter;
		}
		if(letter == '=') idx1++;
		if(letter == ':') idx2++;
		letter = line[++counter];
	}
	config->peers[peer].endpoint[idx1] = '\0';
	port[idx2] = '\0';
	config->peers[peer].endpointPort = atoi(port);
	//printf("[parseEndpoint] Endpoint: %s:%d\n", config->peers[peer].endpoint, config->peers[peer].endpointPort);
}

/*
 * Function to init config file of Wireguard to config struct
 *
 * @para config:    pointer at config struct to store data in
 * @para interface: string with interface name (which also is name of config file, e.g. interface="wg0" has config file "wg0.conf")
 */
void initConfig(struct Config *config, char *interface) {
	/* Open config file of interface */
	char configFileName[BUF_SMALL];
	snprintf(configFileName, sizeof(configFileName), "/etc/wireguard/%s.conf", interface);
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *configFile = fopen(configFileName, "r");
	if(configFile == NULL) {
		printf("[parseConfig] failed to open file %s\n", configFileName);
		exit(EXIT_FAILURE);
	}
	int peerCounter = 0;
	/* Read lines */
	while ((read = getline(&line, &len, configFile)) != -1) {
		/* Ignore comments */
		if(line[0] == '#') continue;
		/* If peer found do: role=UNKNOWN (because initConfig and not parseConfig), connectionStarted=false, peerCounter increase */
		if(strstr(line, "[Peer]") != NULL) {
			config->peers[peerCounter].role = UNKNOWN;
			config->peers[peerCounter].connectionStarted = false;
			peerCounter++;
		}
		/* Check if more peers in config file than MAX_PEERS allows */
		if(peerCounter > MAX_PEERS) {
			printf("[parseConfig] configFile %s has more peers than MAX_PEERS allows.\n", configFileName);
			exit(EXIT_FAILURE);
		}
		if(strstr(line, "Address") != NULL) {
			parseAddress(config, line);
			//printf("[parseConfig] Address is %s\n", config->interface.address);
		}
		if(strstr(line, "ListenPort") != NULL) {
			parseListenPort(config, line);
			//printf("[parseConfig] ListenPort is %d\n", config->interface.listenPort);
		}
		if(strstr(line, "PublicKey") != NULL) {
			parsePublicKey(config, line, peerCounter-1);
			//printf("[parseConfig] PublicKey is %s\n", config->peers[peerCounter-1].pubKey);
		}
		if(strstr(line, "PresharedKey") != NULL) {
			parsePresharedKey(config, line, peerCounter-1);
			//printf("[parseConfig] PresharedKey is %s\n", config->peers[peerCounter-1].psk);
		}
		if(strstr(line, "Endpoint") != NULL) {
			parseEndpoint(config, line, peerCounter-1);
			//printf("[parseConfig] Endpoint is %s:%d\n", config->peers[peerCounter-1].endpoint, config->peers[peerCounter-1].endpointPort);
		}
	}
	config->peerNum = peerCounter;
	// free space
	fclose(configFile);
	if(line) free(line);
}

/*
 * Function to parse config file of Wireguard to config struct
 *
 * @para config:    pointer at config struct to store data in
 * @para interface: string with interface name (which also is name of config file, e.g. interface="wg0" has config file "wg0.conf")
 */
void parseConfig(struct Config *config, char *interface) {
	/* Open config file of interface */
	char configFileName[BUF_SMALL];
	snprintf(configFileName, sizeof(configFileName), "/etc/wireguard/%s.conf", interface);
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *configFile = fopen(configFileName, "r");
	if(configFile == NULL) {
		printf("[parseConfig] failed to open file %s\n", configFileName);
		exit(EXIT_FAILURE);
	}
	int peerCounter = 0;
	/* Read lines */
	while ((read = getline(&line, &len, configFile)) != -1) {
		/* Ignore comments */
		if(line[0] == '#') continue;
		/* If peer found increase peerCounter for latter check if to many peers in config file. */
		if(strstr(line, "[Peer]") != NULL) {
			peerCounter++;
		}
		/* Check if more peers in config file than MAX_PEERS allows */
		if(peerCounter > MAX_PEERS) {
			printf("[parseConfig] configFile %s has more peers than MAX_PEERS allows.\n", configFileName);
			exit(EXIT_FAILURE);
		}
		if(strstr(line, "Address") != NULL) {
			parseAddress(config, line);
			//printf("[parseConfig] Address is %s\n", config->interface.address);
		}
		if(strstr(line, "ListenPort") != NULL) {
			parseListenPort(config, line);
			//printf("[parseConfig] ListenPort is %d\n", config->interface.listenPort);
		}
		if(strstr(line, "PublicKey") != NULL) {
			parsePublicKey(config, line, peerCounter-1);
			//printf("[parseConfig] PublicKey is %s\n", config->peers[peerCounter-1].pubKey);
		}
		if(strstr(line, "PresharedKey") != NULL) {
			parsePresharedKey(config, line, peerCounter-1);
			//printf("[parseConfig] PresharedKey is %s\n", config->peers[peerCounter-1].psk);
		}
		if(strstr(line, "Endpoint") != NULL) {
			parseEndpoint(config, line, peerCounter-1);
			//printf("[parseConfig] Endpoint is %s:%d\n", config->peers[peerCounter-1].endpoint, config->peers[peerCounter-1].endpointPort);
		}
	}
	config->peerNum = peerCounter;
	/* Free space */
	fclose(configFile);
	if(line) free(line);
}

#endif // PARSER_H
