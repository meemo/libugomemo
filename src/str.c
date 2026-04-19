#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <ugomemo.h>

// Instead of UTF-8 this does unicode codepoint escaping for non-ascii characters
char *utf16le_to_ascii(u8 *input, u32 input_len) {
    // Escapes look like e.g. \\u0000 so we need a rather oversized buffer for the string
    // Example worst case (PPM): 10 unprintable unicode characters plus null terminator
    // len("\\u0001\\u0001\\u0001\\u0001\\u0001\\u0001\\u0001\\u0001\\u0001\\u0001") = 70 + 1
    char *output = (char *) calloc((input_len * 7) + 1, sizeof(char));
    u32 i = 0, j = 0;

    while (i < input_len) {
        u16 codepoint = read_le16(input, i);  // This can be simplified because little endian

        // 0x20 is space, 0x7E is ~, 0x22 is ", 0x5C is backslash
        if (codepoint < 0x20 || codepoint > 0x7E || codepoint == 0x22 || codepoint == 0x5C) {
            if (codepoint == 0) break; // There's already NUL padding in the buffer

            snprintf(output + j, 7, "\\u%04X", codepoint);
            j += 6;
        } else {
            // Let it through unchanged, guaranteed to fit in a char
            output[j++] = (char)codepoint;
        }

        i += 2;
    }

    return output;
}

/* Convert string s to int out. https://stackoverflow.com/a/12923949
 *
 * @param[out] out The converted int. Cannot be NULL.
 *
 * @param[in] s Input string to be converted.
 *
 *     The format is the same as strtol,
 *     except that the following are inconvertible:
 *
 *     - empty string
 *     - leading whitespace
 *     - any trailing characters that are not part of the number
 *
 *     Cannot be NULL.
 *
 * @param[in] base Base to interpret string in. Same range as strtol (2 to 36).
 *
 * @return Indicates if the operation succeeded, or why it failed.
 */
str2int_errno str2int(int *out, char *s, int base) {
    char *end;
    if (s[0] == '\0' || isspace((unsigned char) s[0]))
        return STR2INT_INCONVERTIBLE;
    errno = 0;
    long l = strtol(s, &end, base);
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return STR2INT_OVERFLOW;
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return STR2INT_UNDERFLOW;
    if (*end != '\0')
        return STR2INT_INCONVERTIBLE;
    *out = l;
    return STR2INT_SUCCESS;
}

void print_hex(const char *label, const u8 *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02X", data[i]);
        if ((i + 1) % 32 == 0 && i + 1 < len) printf("\n%*s", (int)strlen(label) + 2, "");
    }
    printf("\n");
}
