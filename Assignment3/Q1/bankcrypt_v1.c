/* Encryption library, version 1.0 (toy Caesar shift - stands in for a real crypto library). */
#include <string.h>
#include "bankcrypt.h"
const char *crypt_version(void) { return "bankcrypt 1.0"; }
void crypt_encrypt(const char *in, char *out)
{
    size_t n = strlen(in);
    for (size_t i = 0; i < n; i++) out[i] = (char)(in[i] + 3);
    out[n] = '\0';
}
