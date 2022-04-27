#!/bin/bash
# Skript to be used for RSA keys outside of SmartCard-HSM

echo "2022.04.26.15" | tr -d "\n\r" > timestamp
openssl dgst -sha256 -sign ../WireGuardHSM-keys/private_key.pem -out sha256.sign timestamp
rm timestamp
echo "++++" $(cat sha256.sign | sha256sum | cut -d ' ' -f 1 | xxd -r -p | base64)
rm sha256.sign
