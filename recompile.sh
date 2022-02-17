#!/bin/bash

INTERFACE=$(cat wireguardhsm/settings.h | grep "#define INTERFACE" | grep -oP '"\K[^"]+')
ENABLE_HSM=$(cat wireguardhsm/settings.h | grep "#define ENABLE_HSM" | grep -oP '"\K[^"]+')
ENABLE_TIMESTAMP=$(cat wireguardhsm/settings.h | grep "#define ENABLE_TIMESTAMP" | grep -oP '"\K[^"]+')

gcc -o start_$INTERFACE-hsm_$ENABLE_HSM-timestamp_$ENABLE_TIMESTAMP wireguardhsm/wireguardhsm.c
