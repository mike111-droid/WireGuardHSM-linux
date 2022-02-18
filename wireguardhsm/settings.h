/*
 * settings.h
 * Contains settings macros needed for wireguard daemon.
 */

#ifndef SETTINGS_H
#define SETTINGS_H


/* Maximum number of peers allowed in the interface file. Program exists for to many peers. */
#define MAX_PEERS        2

/* Directory to wireguard interface config files. */
#define WIREGUARD_DIR     "/etc/wireguard"

/* Reset preshared key for connections with ENABLE_HSM="n" (without HSM). */
#define RESET_PSK         "or/ZJXL3mejqaF+5TyGpYhr02ceXgE15Ysqt2Xia81o="

/* Init value for ENABLE_TIMESTAMP=false and ENABLE_HSM="y". */
#define INIT_PSK          "Init_value_for_init_psk_hsm_without_timestamp"

/* Name of the interface. Corresponding config file is "wg0.conf". */
#define INTERFACE         "wg0"

/* Path to directory with wireguard daemon program. */
#define WORK_DIR          "/home/micha/github/WireguardHSM-linux"

/* Path to directory with SmartCard Shell 3 from CardContact with according js script for scsh3 and expect script for wireguard daemon. */
#define SCSH_DIR          "/home/micha/CardContact/scsh3"

/* Macro defines if first PSK should be calculate with the help of HSM(INIT_PSK) or use static RESET_PSK */
#define ENABLE_HSM        "y"

/* Macro defines if timestamp should be used */
#define ENABLE_TIMESTAMP  "y"

/* Macro defines if the PIN has to be inputed by user with each HSM access */
#define ENABLE_SECUREMODE "y"

/* Possible KEY_TYPE are RSA, ECC, AES */
#define KEY_TYPE          "RSA"

/* Label that is used to identify the key on the HSM (is used in wireguard_daemon.js) */
#define KEY_LABEL         "RSA_KEY"

#endif // SETTINGS_H
