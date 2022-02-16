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
```

*setup* starts a script that lets you input the necessary settings or that automatically set them. **
## Functionality
