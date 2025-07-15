/*
Embedded minimal big integer implementation
Public domain - replacement for GMP in MSE encryption
*/

#ifndef EMBEDDED_BIGINT_H
#define EMBEDDED_BIGINT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#include <string>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BIGINT_LIMBS 32

typedef struct {
    uint32_t limbs[MAX_BIGINT_LIMBS];
    int size;
    int sign;
} mpz_t[1];

// Initialize/clear functions
void mpz_init(mpz_t rop);
void mpz_clear(mpz_t rop);
void mpz_init_set(mpz_t rop, const mpz_t op);
void mpz_init_set_ui(mpz_t rop, unsigned long int op);
void mpz_init_set_str(mpz_t rop, const char *str, int base);

// Assignment functions  
void mpz_set(mpz_t rop, const mpz_t op);
void mpz_set_ui(mpz_t rop, unsigned long int op);
void mpz_set_str(mpz_t rop, const char *str, int base);

// Arithmetic functions
void mpz_add(mpz_t rop, const mpz_t op1, const mpz_t op2);
void mpz_sub(mpz_t rop, const mpz_t op1, const mpz_t op2);
void mpz_mul(mpz_t rop, const mpz_t op1, const mpz_t op2);
void mpz_powm(mpz_t rop, const mpz_t base, const mpz_t exp, const mpz_t mod);

// Utility functions
int mpz_cmp(const mpz_t op1, const mpz_t op2);
size_t mpz_sizeinbase(const mpz_t op, int base);
char *mpz_get_str(char *str, int base, const mpz_t op);
void mpz_random(mpz_t rop, unsigned int bits);

// Export/import functions
size_t mpz_export(void *rop, size_t *countp, int order, size_t size, int endian, size_t nails, const mpz_t op);
void mpz_import(mpz_t rop, size_t count, int order, size_t size, int endian, size_t nails, const void *op);

#ifdef __cplusplus
}
#endif

#endif // EMBEDDED_BIGINT_H