/*
 * wireguard_daemon_ecc.js
 * Hashes oldpsk and signs it with the key RSA_KEY on the HSM. Then prints result in base64 to consol in format "++++<RESULT>"
 */

SmartCardHSM = require('scsh/sc-hsm/SmartCardHSM').SmartCardHSM;
HSMKeyStore = require('scsh/sc-hsm/HSMKeyStore').HSMKeyStore;
PKCS1 = require('scsh/pkcs/PKCS1').PKCS1;

// Create nessecary vars
var crypto = new Crypto();
var card = new Card(_scsh3.reader);
var sc = new SmartCardHSM(card);
sc.verifyUserPIN(new ByteString("654321", ASCII));
var ks = new HSMKeyStore(sc);

// Get Key
var key = ks.sc.getKey("RSA_KEY");

// Digest old psk
var oldpsk = "2022.02.15.13";
var message = new ByteString(oldpsk, ASCII);
var hash = crypto.digest(Crypto.SHA_256, message);
print(hash);

// Sign hash old psk and than hash again
print(key);
print(key.getType());
print("Crypto.ECDSA");
var signature = sc.getCrypto().sign(key, Crypto.ECDSA_SHA256, message);
var result = crypto.digest(Crypto.SHA_256, signature);
print("++++ " + result.toBase64().toString(ASCII));
