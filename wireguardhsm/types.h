/*
 * types.h
 * Contains custom data types (structs) and macros used in the wireguard daemon.
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <sys/time.h>
#include "settings.h"

#define INIT_SND    1
#define INIT_RCV    2
#define RESP_SND    3
#define RESP_RCV    4
#define UNKNOWN     0
#define INITIATOR   1
#define RESPONDER   2
/* BUF sizes are used for dynamic inputs. Depending on system output high buffer size can be required. But also possible much smaller buffer sizes could be possible to decrease daemon footprint. */
#define BUF_SMALL   1024
#define BUF_MEDIUM  5012
#define BUF_BIG     10024
#define BUF_TIMSTMP 256
#define MSG_LEN     512
#define PIN_SIZE    128
/* Colors for output */
#define RESET       "\033[0m"
#define BLACK       "\033[30m"             /* Black */
#define RED         "\033[31m"             /* Red */
#define GREEN       "\033[32m"             /* Green */
#define YELLOW      "\033[33m"             /* Yellow */
#define BLUE        "\033[34m"             /* Blue */
#define MAGENTA     "\033[35m"             /* Magenta */
#define CYAN        "\033[36m"             /* Cyan */
#define WHITE       "\033[37m"             /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

/* Msg struct is used to interprocess communication with the message queue. */
struct Msg {
        int  msgType;
        char msg[MSG_LEN];
} ipcMsg;

/* Interface struct is used to save important data of interface that is in the config file. */
struct Interface {
	char         address[48];
	unsigned int listenPort;
};

/* Peer struct is used to save important peer data from config file and for overall peer depandant infos of daemon. */
struct Peer {
	char         pubKey[128];
	char         psk[128];
	char         endpoint[48];
	unsigned int endpointPort;
	int          role;
	clock_t      pskReload;
	int          initHandshakeCounter;
	bool         reloaded;
	bool         connectionStarted;

};

/* Config struct is used to save important data from config file and other important data for daemon. */
struct Config {
	struct Interface interface;
	int              peerNum;
	struct Peer      peers[MAX_PEERS];
};

#endif // TYPES_H
