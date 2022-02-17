# WireguardHSM-linux

## Introduction
This program is the attempt to include a Hardware Security Module into the [Wireguard VPN](https://github.com/WireGuard). For this I used the [SmartCard-HSM USB Token](https://www.cardomatic.de/SmartCard-HSM-4K-USB-Token). In order for two communications peers to use this feature you need to identical keys on two seperat SmartCard-HSM's. The following will describe how to setup this application on Linux Ubuntu 20.04 (Disclaimer: It migth work on other distros and versions too but was not tested there).

## Setting up the SmartCard-HSM's
1. **Installing swissbit driver for SmartCard-HSM**: The necessary instructions are all in [SmartCard-HSM Starterkit](http://www.cardcontact.de/download/sc-hsm-starterkit.zip)
2. **Initializing the SmartCard-HSM with a shared DKEK**: A good instruction can be found [here](https://vessokolev.blogspot.com/2019/06/smartcard-hsm-usb-token-using-smart.html). Especially step six is needed in order to create a DKEK. Make sure to import the same DKEK share to the two HSM's.
3. **Generating, exporting and importing generated keys**: Again [this](https://vessokolev.blogspot.com/2019/06/smartcard-hsm-usb-token-using-smart.html) contains the necessary instructions to generate a key. You can then export the key by right-clicking it and selecting *Export key and certificate*. For importing you can right-click the root folder in the left tree diagramm and select import. For this you need to be logged in.

## Building
```
$ git clone https://github.com/mike111-droid/WireguardHSM-linux  
$ cd WireguardHSM-linux  
$ bash setup
$ sudo ./start
```

*setup* starts a script that lets you input the necessary settings or that automatically sets them, and checks the necessary dependencies. *recompile* allows to recompile with the current settings. *start* is the executable that needs to execute with root privilages in order to access wireguard.

## Functionality
The application works as a wrap-around daemon. This means Wireguard was not changed at all. The WireguardHSM connection is only managed from the outside with shell commands and the wireguard config file. Essential is the shell command *wg addconf %s <(wg-quick strip wgX* that allows to reload the config file *wgX.conf* without restarting the tunnel.  

WireguardHSM creates dynamic preshared keys (PSK's) with the possibility to use a HSM in the process and loads them into the wireguard tunnel.

### How does it work?

### How do we stay synchronized?

### How do we generate new PSK's?
