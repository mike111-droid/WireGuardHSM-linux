# WireguardHSM-linux

## Introduction
This program is the attempt to include a Hardware Security Module into the [Wireguard VPN](https://github.com/WireGuard). For this I used the [SmartCard-HSM USB Token](https://www.cardomatic.de/SmartCard-HSM-4K-USB-Token). In order for two communications peers to use this feature you need to identical keys on two seperat SmartCard-HSM's. The following will describe how to setup this application on Linux Ubuntu 20.04 (Disclaimer: It might work on other distros and versions too but was not tested there).

## Setting up the SmartCard-HSM's
1. **Installing swissbit driver for SmartCard-HSM**: The necessary instructions are all in [SmartCard-HSM Starterkit](http://www.cardcontact.de/download/sc-hsm-starterkit.zip)
2. **Initializing the SmartCard-HSM with a shared DKEK**: A good instruction can be found [here](https://vessokolev.blogspot.com/2019/06/smartcard-hsm-usb-token-using-smart.html). Especially step six is needed in order to create a DKEK. Make sure to import the same DKEK share to the two HSM's.
3. **Generating, exporting and importing generated keys**: Again [this](https://vessokolev.blogspot.com/2019/06/smartcard-hsm-usb-token-using-smart.html) contains the necessary instructions to generate a key. You can then export the key by right-clicking it and selecting *Export key and certificate*. For importing you can right-click the root folder in the left tree diagramm and select import. For this you need to be logged in. (IMPORTANT: At the moment WireguardHSM only works with a RSA key with the label *RSA_KEY*. This can be easily changed by altering *scsh3/wireguard_daemon.js*)

## Building
```
$ git clone https://github.com/mike111-droid/WireguardHSM-linux  
$ cd WireguardHSM-linux  
$ bash setup
$ sudo ./start_wg0-hsm_y-timestamp_y
```

*setup* starts a script that lets you input the necessary settings or that automatically sets them, and checks the necessary dependencies. *recompile* allows to recompile with the current settings. *start_wg0-hsm_y-timestamp_y* is the executable that needs to execute with root privilages in order to access wireguard. It also includes the important settings in the name such as if HSM or HSM_TIMESTAMP is enabled, or which interface will be started. (IMPORTANT: WiregaurdHSM only works if you have pcscd start with *sudo pcscd -d -f*. The systemctl version always lead to a segmentation fault on my computer)

## Functionality
The application works as a wrap-around daemon. This means Wireguard was not changed at all. The WireguardHSM connection is only managed from the outside with shell commands and the wireguard config file. Essential is the shell command *wg addconf %s <(wg-quick strip wgX* that allows to reload the config file *wgX.conf* without restarting the tunnel.  

WireguardHSM creates dynamic, changing preshared keys (PSK's) with the possibility to use a HSM in the process, and loads them into the wireguard tunnel as PSK's. The application needs to run on both communication peers, so that both have the same PSK during the handshake.

### How does it work?


### How do we stay synchronized?
The idea behind staying synchronized in this Hash-Chain depends on the handshakes. Usually a handshake is done in Wireguard with two messages: the handshake initiation message (INIT) and the handshake response message (RESP). If a initiator receives a RESP message it can be sure that the responder received the INIT message and is ready to change to the next packat encryption key (generated with the static, ephemeral and preshared key). This constitutes a successful handshake for the initatior. For the responder this is more difficult. Only the first encrypted package allows the responder to be sure of a successful handshake. Unfortuantly, WireguardHSM does not have access to this. In order to solve this problem the following behaviours have been defined.  
**Initiator role**: If an initiator receives RESP message (handshake was successful), a timer starts. After 60 seconds the new PSK is loaded.  
**Responder role**: If a responder receives INIT message (handshake may be successful), a timer starts. After 60 seconds the new PSk is loaded. If within these 60 seconds another INIT message arrives, the timer is reset. So only if for 60 seconds no INIT message was received will the next PSK be loaded. This is necessary because the responder does not know if its RESP message arrived at the initiators side.


### How do we generate new PSK's?
The generation of the new PSK's depends on the settings selected.
1. ENABLE_HSM==y && ENABLE_TIMESTAMP==y  
At the start the application generates the first PSK with HSM(TIMESTAMP).The timestamp has an accuracy of an hour and with every changing timestamp the PSK is changed with HSM(TIMESTAMP). Furthermore, a new PSK is generated with each successful handshake with NEW_PSK = SHA256(OLD_PSK). If the connections fails because of asynchron PSK-Chains in peers, the reset is done with HSM(TIMESTAMP).
2. ENABLE_HSM==y && ENABLE_TIMESTAMP==n  
At the start the application generates the first PSK with HSM(INIT_VALUE). Then it generates the NEW_PSK = SHA256(OLD_PSK). If the connections fails because of asynchron PSK-Chains in peers, the reset is done with HSM(INIT_VALUE).
3. ENABLE_HSM==n && ENABLE_TIMESTAMP==n  
At the start the application initlializes the first PSK with RESET_PSK. Then it generate the NEW_PSK = SHA256(OLD_PSK). If the connections fails because of asynchron PSK-Chains in peers, the reset is done with RESET_PSK.


