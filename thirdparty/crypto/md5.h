/*
Embedded MD5 implementation
Public domain - based on RFC 1321
*/

#ifndef EMBEDDED_MD5_H
#define EMBEDDED_MD5_H

#include <stdint.h>

typedef struct {
    uint32_t state[4];
    uint32_t count[2];
    unsigned char buffer[64];
} MD5_CTX;

#ifdef __cplusplus
extern "C" {
#endif

void MD5Init(MD5_CTX *context);
void MD5Update(MD5_CTX *context, const unsigned char *input, unsigned int inputLen);
void MD5Final(unsigned char digest[16], MD5_CTX *context);

#ifdef __cplusplus
}
#endif

#endif // EMBEDDED_MD5_H