/*
Embedded SHA1 implementation
Public domain - based on RFC 3174
*/

#ifndef EMBEDDED_SHA1_H
#define EMBEDDED_SHA1_H

#include <stdint.h>

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA_CTX;

#ifdef __cplusplus
extern "C" {
#endif

void SHA1_Init(SHA_CTX *context);
void SHA1_Update(SHA_CTX *context, const unsigned char *data, unsigned int len);
void SHA1_Final(unsigned char digest[20], SHA_CTX *context);

#ifdef __cplusplus
}
#endif

#endif // EMBEDDED_SHA1_H