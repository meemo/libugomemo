#ifndef UGOMEMO_KEYS_H_
#define UGOMEMO_KEYS_H_

#include <ugomemo/crypto.h>

// Built-in public keys for signature verification.
// These functions populate the provided key struct with verification-only
// key material (no private exponent). Returns UGOMEMO_OK.
int ugomemo_kwz_public_key(ugomemo_rsa_key *key);
int ugomemo_ppm_public_key(ugomemo_rsa_key *key);

#endif
