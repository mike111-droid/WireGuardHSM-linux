#!/bin/bash
# Setup script for the WireguardHSM Daemon.

echo "[*] Trying to start the setup of the WireguardHSM Daemon on this machine..."

# Check if all necesary directories are available
echo -e "[*] Cecking for necessary directories..."
for name in scsh3 wireguardhsm
do
  if [ ! -d $name ];
  then
    echo -e "[ERROR] Directory $name is missing. Make sure to execute the bash script in the correct directory so it has access to the necessary files..."
    exit 1
  fi
done
echo "OK"

echo -e "[*] Checking for necessary files..."
# Check if all necessary files are in wireguard directory
suffix="wireguardhsm/"
for name in config_changer.h parser.h settings.h signals.h types.h wireguard.c
do
  if [ ! -f $suffix$name ];
  then
    echo -e "[ERROR] File $suffix$name does not exists from this directory. Make sure to execute this script from the correct directory..."
    exit 1
  fi
done

# Check if all necessary files are in scsh3 directory
suffix="scsh3/"
for name in wireguard_daemon.js wireguard_daemon.ex
do
  if [ ! -f $suffix$name ];
  then
    echo -e "[ERROR] File $suffix$name does not exists from this directory. Make sure to execute this script from the correct directory..."
    exit 1
  fi
done
echo "OK"

# Check for dependencies
echo "[*] Checking for the necessary dependencies..."
for name in expect wg sed wg-quick tee pcscd
do
  [[ $(which $name 2>/dev/null) ]] || { echo -en "\n$name needs to be installed. Use 'sudo apt-get install $name'";deps=1; }
done
[[ $deps -ne 1 ]] && echo "OK" || { echo -en "\nInstall the above and rerun this script\n";exit 1; }


# Set settings.h according to host machine
echo "[*] Setting settings.h according to host machine..."
### Setting up settings.h

# WORK_DIR
WORK_DIR=$(pwd)
echo -e "\t[*] Automatically setting WORK_DIR to persent working directory."
echo -e "\t-> Setting $WORK_DIR as WORK_DIR..."
sed -i  "s|#define WORK_DIR .*|#define WORK_DIR         \"$WORK_DIR\"|" wireguardhsm/settings.h

# SCSH3_DIR
echo -en "\t"
read -p "[*] Enter directory where SmartCard Shell 3 (scsh3) is located [$HOME/CardContact/scsh3]: " SCSH3_DIR
SCSH3_DIR=${SCSH3_DIR:-$HOME/CardContact/scsh3}
echo -e "\t-> Setting $SCSH3_DIR as SCSH3_DIR..."
sed -i "s|#define SCSH3_DIR .*|#define SCSH3_DIR         \"$SCSH3_DIR\"|" wireguardhsm/settings.h
echo -e "\t-> Copying wireguard_daemon.js and wireguard_daemon.ex to $SCSH3_DIR..."
cp scsh3/wireguard_daemon.js $SCSH3_DIR
cp scsh3/wireguard_daemon.ex $SCSH3_DIR

# INTERFACE
echo -en "\t"
read -p "[*] Enter name of interface you want the daemon to monitor [wg0]: " INTERFACE
INTERFACE=${INTERFACE:-wg0}
echo -e "\t-> Setting $INTERFACE as INTERFACE. $INTERFACE.conf is the according config file..."
sed -i "s|#define INTERFACE .*|#define INTERFACE        \"$INTERFACE\"|" wireguardhsm/settings.h

# MAX_PEERS
echo -en "\t"
read -p "[*] Enter the number of expected/maximal peers in the interface config [1]: " MAX_PEERS
MAX_PEERS=${MAX_PEERS:-1}
echo -e "\t-> Setting $MAX_PEERS as MAX_PEERS..."
sed -i "s|#define MAX_PEERS .*|#define MAX_PEERS        $MAX_PEERS|" wireguardhsm/settings.h

# ENABLE_HSM
echo -en "\t"
read -p "[*] Do you want to use the option ENABLE_HSM y/n? [y] " ENABLE_HSM
ENABLE_HSM=${ENABLE_HSM:-y}
echo -e "\t-> Setting $ENABLE_HSM as ENABLE_HSM..."
sed -i "s|#define ENABLE_HSM .*|#define ENABLE_HSM       \"$ENABLE_HSM\"|" wireguardhsm/settings.h

# ENABLE_TIMESTAMP
echo -en "\t"
read -p "[*] Do you want to use the option ENABLE_TIMESTAMP y/n? [y] " ENABLE_TIMESTAMP
ENABLE_HSM=${ENABLE_TIMESTAMP:-y}
echo -e "\t-> Setting $ENABLE_TIMESTAMP as ENABLE_TIMESTAMP..."
sed -i "s|#define ENABLE_TIMESTAMP .*|#define ENABLE_TIMESTAMP \"$ENABLE_HSM\"|" wireguardhsm/settings.h

gcc -o start wireguardhsm/wireguardhsm.c
echo -e "[*] The setup is done. You can change everything in $PWD/wireguard/settings.h and recompile, or restart setup.sh. Executable start was created."
