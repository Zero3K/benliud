/*
Embedded SHA1 implementation
Public domain - based on RFC 3174
*/

#include "sha1.h"
#include <string.h>

#define SHA1_ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))

static void SHA1Transform(SHA_CTX *ctx, const unsigned char data[])
{
    uint32_t a, b, c, d, e, i, j, t, m[80];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) + (data[j + 1] << 16) + (data[j + 2] << 8) + (data[j + 3]);
    for ( ; i < 80; ++i) {
        m[i] = (m[i - 3] ^ m[i - 8] ^ m[i - 14] ^ m[i - 16]);
        m[i] = SHA1_ROTLEFT(m[i], 1);
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];

    for (i = 0; i < 20; ++i) {
        t = SHA1_ROTLEFT(a, 5) + ((b & c) ^ (~b & d)) + e + 0x5A827999 + m[i];
        e = d;
        d = c;
        c = SHA1_ROTLEFT(b, 30);
        b = a;
        a = t;
    }
    for ( ; i < 40; ++i) {
        t = SHA1_ROTLEFT(a, 5) + (b ^ c ^ d) + e + 0x6ED9EBA1 + m[i];
        e = d;
        d = c;
        c = SHA1_ROTLEFT(b, 30);
        b = a;
        a = t;
    }
    for ( ; i < 60; ++i) {
        t = SHA1_ROTLEFT(a, 5) + ((b & c) ^ (b & d) ^ (c & d)) + e + 0x8F1BBCDC + m[i];
        e = d;
        d = c;
        c = SHA1_ROTLEFT(b, 30);
        b = a;
        a = t;
    }
    for ( ; i < 80; ++i) {
        t = SHA1_ROTLEFT(a, 5) + (b ^ c ^ d) + e + 0xCA62C1D6 + m[i];
        e = d;
        d = c;
        c = SHA1_ROTLEFT(b, 30);
        b = a;
        a = t;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
}

void SHA1_Init(SHA_CTX *ctx)
{
    ctx->count[0] = ctx->count[1] = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
}

void SHA1_Update(SHA_CTX *ctx, const unsigned char data[], unsigned int len)
{
    unsigned int i, j;

    j = ctx->count[0];
    if ((ctx->count[0] += len << 3) < j)
        ctx->count[1]++;
    ctx->count[1] += (len>>29);
    j = (j >> 3) & 63;
    if ((j + len) > 63) {
        memcpy(&ctx->buffer[j], data, (i = 64-j));
        SHA1Transform(ctx, ctx->buffer);
        for ( ; i + 63 < len; i += 64) {
            SHA1Transform(ctx, &data[i]);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&ctx->buffer[j], &data[i], len - i);
}

void SHA1_Final(unsigned char hashed[20], SHA_CTX *ctx)
{
    unsigned int i;
    unsigned char finalcount[8], c;

    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((ctx->count[(i >= 4 ? 0 : 1)] >> ((3-(i & 3)) * 8) ) & 255);
    }

    c = 0200;
    SHA1_Update(ctx, &c, 1);
    while ((ctx->count[0] & 504) != 448) {
        c = 0000;
        SHA1_Update(ctx, &c, 1);
    }
    SHA1_Update(ctx, finalcount, 8);
    for (i = 0; i < 20; i++) {
        hashed[i] = (unsigned char)((ctx->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }
}