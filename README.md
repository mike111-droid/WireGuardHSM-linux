# WireguardHSM-linux

## Introduction
This program is an attempt to include a Hardware Security Module into the [Wireguard VPN](https://github.com/WireGuard). For this, I used the [SmartCard-HSM USB Token](https://www.cardomatic.de/SmartCard-HSM-4K-USB-Token). In order for two communications peers to use this feature, you need identical keys on two seperate SmartCard-HSM's. The following will describe how to set up this application on Linux Ubuntu 20.04 (Disclaimer: It might work on other distros and versions as well but was not tested there).

## Setting up the SmartCard-HSM's
1. **Installing swissbit driver for SmartCard-HSM**: The necessary instructions are all in [SmartCard-HSM Starterkit](http://www.cardcontact.de/download/sc-hsm-starterkit.zip).
2. **Initializing the SmartCard-HSM with a shared DKEK**: A good instruction can be found [here](https://vessokolev.blogspot.com/2019/06/smartcard-hsm-usb-token-using-smart.html). Especially step six is needed to create a DKEK. Make sure to import the same DKEK share to the two HSM's.
3. **Generating, exporting, and importing generated keys**: [This](https://vessokolev.blogspot.com/2019/06/smartcard-hsm-usb-token-using-smart.html) contains the necessary instructions to create a key. You can then export the key by right-clicking it and selecting *Export key and certificate*. You can right-click the root folder in the left tree diagram for importing a key and selecting import. For this, you need to be logged in. (IMPORTANT: At the moment, WireguardHSM only works with a RSA key with the label *RSA_KEY*. This can be easily changed by altering *scsh3/wireguard_daemon.js*)

## Building
```
$ git clone https://github.com/mike111-droid/WireguardHSM-linux  
$ cd WireguardHSM-linux  
$ bash setup
$ sudo ./start_wg0-hsm_y-timestamp_y
```

*setup* starts a script that lets you input the necessary settings or automatically sets them and checks the dependencies required. *recompile* allows to recompile with the current settings. *start_wg0-hsm_y-timestamp_y* is the executable that needs to execute with root privileges to access wireguard. It also includes the important settings in the name (if HSM or HSM_TIMESTAMP is enabled, or which interface will be started). (IMPORTANT: WiregaurdHSM only works if you have pcscd start with *sudo pcscd -d -f*. The systemctl version always lead to a segmentation fault on my computer)

## ToDo's
1. **PIN Managment:** At the moment the PIN is written in *scsh3/wireguard_damon.js*. *wireguardhsm/wireguardhsm.c* should manage the PIN, also with possible user input.
2. **Program output:** For more than one peer the output of the program gets messy because one cannot tell which output belongs to which peer.
3. **Key Label Managment:** Key label is written in *scsh3/wireguard_daemon.js*. Should be more accessiable to user (over setup.sh).
4. **Allow different keys:** Allow the use of RSA, ECC and AES keys. Create alternative java scripts to perform these operations and allow *wireguardhsm/wireguardhsm.c* to choose which key the tunnel (and maybe also the peer) should use.
5. **pcscd problem:** WireguardHSM only seems to be working if *pcscd* is started with *sudo pcscd -d -f* and not with systemctl.

## Functionality
The application works as a wrap-around daemon. This means Wireguard was not changed at all. The WireguardHSM connection is only managed from the outside with shell commands and the wireguard config file. Essential is the shell command *wg addconf %s <(wg-quick strip wgX* that reloads the config file *wgX.conf* without restarting the tunnel.  

WireguardHSM creates dynamic, changing preshared keys (PSK's) with the possibility to use a HSM in the process, and loads them into the wireguard tunnel as PSK's. The application needs to run on both communication peers, so that both have the same PSK during the handshake.

### How does it work?
WireguardHSM works by creating a PSK at the start of the connection. Then with every successful handshake NEW_PSK = SHA256(OLD_PSK), similar to the Ratchet Algorithms of Signal. As a result, a CURRENT_PSK can only be used to decrypt the following PSK's. Because of the hash function's characteristics, the OLD_PSK's cannot be calculated. This obviously only works if INIT_PSK isn't static. The recommended way of using WiregaurdHSM is with the option ENABLE_HSM=y and ENABLE_TIMESTAMP=y.

### How do we stay synchronized?
The idea behind staying synchronized in this Hash-Chain depends on the handshakes. Usually, a handshake is done in Wireguard with two messages: the handshake initiation message (INIT) and the handshake response message (RESP). If an initiator receives a RESP message, it can be sure that the responder received the INIT message and is ready to change to the next packet encryption key (generated with the static, ephemeral, and preshared key). This constitutes a successful handshake for the initiator. For the responder, this is more difficult. Only the first encrypted package allows the responder ensure a successful handshake. Unfortunately, WireguardHSM does not have access to this. To solve this problem, the following behaviors have been defined.  
**Initiator role**: If an initiator receives the RESP message (handshake was successful), a timer starts. After 60 seconds, the new PSK is loaded.  
**Responder role**: If a responder receives the INIT message (handshake may be successful), a timer starts. After 60 seconds, the new PSK is loaded. If another INIT message arrives within these 60 seconds, the timer is reset. So only if for 60 seconds no INIT message was received will the next PSK be loaded. This is necessary because the responder does not know if its RESP message arrived at the initiator's side.


### How do we generate new PSK's?
The generation of the new PSK's depends on the settings selected.
1. *ENABLE_HSM==y && ENABLE_TIMESTAMP==y*  
At the start, the application generates the first PSK with HSM(TIMESTAMP). The timestamp has an accuracy of an hour, and with every changing timestamp, the PSK is changed with HSM(TIMESTAMP). Furthermore, a new PSK is generated with each successful handshake with NEW_PSK = SHA256(OLD_PSK). If the connection fails because of asynchronous PSK-Chains in peers, the reset is done with HSM(TIMESTAMP).
2. *ENABLE_HSM==y && ENABLE_TIMESTAMP==n*  
At the start, the application generates the first PSK with HSM(INIT_VALUE). Then it generates the NEW_PSK = SHA256(OLD_PSK). If the connection fails because of asynchronous PSK-Chains in peers, the reset is done with HSM(INIT_VALUE).
3. *ENABLE_HSM==n && ENABLE_TIMESTAMP==n*  
At the start, the application initializes the first PSK with RESET_PSK. Then it generate the NEW_PSK = SHA256(OLD_PSK). If the connection fails because of asynchronous PSK-Chains in peers, the reset is done with RESET_PSK.


