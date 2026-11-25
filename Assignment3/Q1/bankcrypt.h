#ifndef BANKCRYPT_H
#define BANKCRYPT_H
const char *crypt_version(void);
void crypt_encrypt(const char *in, char *out);   /* out must hold strlen(in)+1 bytes */
#endif
