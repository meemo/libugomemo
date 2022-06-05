#include <libugomemo.h>

/**
 * base64.c
 *
 * This file contains functions for decoding base64 data for use in rsa.c to decode PEM encoded keys.
 *
 * Since base64 is only used for decoding the keys, there are no encoding functions.
 */


/* ========================================== Constants ========================================= */
/* Normal base 64 alphabet for decoding PEM format keys. */
const char b64_alphabet[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/* ============================================================================================== */

