/* Load the encryption library only when it is needed, with dlopen()/dlsym(). */
#define _DARWIN_C_SOURCE
#include <dlfcn.h>
#include <stdio.h>

typedef const char *(*version_fn)(void);
typedef void (*encrypt_fn)(const char *, char *);

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : "./libbankcrypt.dylib";
    printf("Start-up done; encryption not loaded yet.\n");
    void *h = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!h) {   /* graceful error handling instead of the loader killing the program */
        fprintf(stderr, "dlopen failed: %s\nContinuing without encryption (feature disabled).\n", dlerror());
        return 2;
    }
    version_fn ver = (version_fn)dlsym(h, "crypt_version");
    encrypt_fn enc = (encrypt_fn)dlsym(h, "crypt_encrypt");
    if (!ver || !enc) { fprintf(stderr, "dlsym failed: %s\n", dlerror()); dlclose(h); return 3; }
    char out[64];
    enc("ACCOUNT42", out);
    printf("Loaded %s at run time: encrypt(\"ACCOUNT42\") = \"%s\" (crypt_encrypt at %p)\n", ver(), out, (void *)enc);
    dlclose(h);
    return 0;
}
