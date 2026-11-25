/* Branch application: calls the encryption library and shows where code and data are loaded. */
#define _DARWIN_C_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bankcrypt.h"

int global_counter = 42;

static void where(const char *what, const void *addr)
{
    Dl_info info;
    const char *img = "(not in an image)";
    if (dladdr(addr, &info) && info.dli_fname) img = info.dli_fname;
    printf("  %-22s %p  %s\n", what, addr, img);
}

int main(int argc, char **argv)
{
    char out[64];
    crypt_encrypt("ACCOUNT42", out);
    printf("pid %d using %s: encrypt(\"ACCOUNT42\") = \"%s\"\n", (int)getpid(), crypt_version(), out);
    if (argc > 1 && strcmp(argv[1], "--where") == 0) {
        int local = 0;
        void *heap = malloc(16);
        printf("Load addresses (ASLR changes them on every run):\n");
        where("main (program text)", (const void *)main);
        where("crypt_encrypt (library)", (const void *)crypt_encrypt);
        where("printf (libSystem)", (const void *)printf);
        where("global_counter (data)", &global_counter);
        printf("  %-22s %p\n  %-22s %p\n", "heap block", heap, "stack variable", (void *)&local);
        free(heap);
    }
    if (argc > 1 && strcmp(argv[1], "--wait") == 0) sleep(5);   /* keep running for vmmap */
    return 0;
}
