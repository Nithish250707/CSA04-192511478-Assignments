/* Encryption library, version 1.1 - the "security fix" shipped without rebuilding bank. */
#include <string.h>
#include "bankcrypt.h"
const char *crypt_version(void) { return "bankcrypt 1.1 (security fix)"; }
void crypt_encrypt(const char *in, char *out)
{
    size_t n = strlen(in);
    for (size_t i = 0; i < n; i++) out[i] = (char)(in[i] ^ 0x2A);   /* different algorithm */
    out[n] = '\0';
}
