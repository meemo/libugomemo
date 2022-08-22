#include <libugomemo.h>

/**
 * base32.c
 *
 * Functions for encoding and decoding base32 data, specifically intended for KWZ file names.
 */


/* ========================================== Constants ========================================= */
/* Non-standard base 32 alphabet that is used for .kwz filenames. */
const char kwz_b32_alphabet[32] = "cwmfjordvegbalksnthpyxquiz012345";
/* ============================================================================================== */
