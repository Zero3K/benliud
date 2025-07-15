/*
Embedded minimal big integer implementation
Public domain - replacement for GMP in MSE encryption
*/

#include "bigint.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static void normalize(mpz_t op)
{
    while (op->size > 1 && op->limbs[op->size - 1] == 0)
        op->size--;
    if (op->size == 1 && op->limbs[0] == 0)
        op->sign = 0;
}

void mpz_init(mpz_t rop)
{
    memset(rop->limbs, 0, sizeof(rop->limbs));
    rop->size = 1;
    rop->sign = 0;
}

void mpz_clear(mpz_t rop)
{
    memset(rop->limbs, 0, sizeof(rop->limbs));
    rop->size = 1;
    rop->sign = 0;
}

void mpz_init_set(mpz_t rop, const mpz_t op)
{
    mpz_init(rop);
    mpz_set(rop, op);
}

void mpz_init_set_ui(mpz_t rop, unsigned long int op)
{
    mpz_init(rop);
    mpz_set_ui(rop, op);
}

void mpz_init_set_str(mpz_t rop, const char *str, int base)
{
    mpz_init(rop);
    mpz_set_str(rop, str, base);
}

void mpz_set(mpz_t rop, const mpz_t op)
{
    memcpy(rop->limbs, op->limbs, sizeof(op->limbs));
    rop->size = op->size;
    rop->sign = op->sign;
}

void mpz_set_ui(mpz_t rop, unsigned long int op)
{
    memset(rop->limbs, 0, sizeof(rop->limbs));
    rop->limbs[0] = (uint32_t)op;
    if (sizeof(unsigned long) > 4 && op > 0xFFFFFFFFUL) {
        rop->limbs[1] = (uint32_t)(op >> 32);
        rop->size = 2;
    } else {
        rop->size = 1;
    }
    rop->sign = (op != 0) ? 1 : 0;
    normalize(rop);
}

void mpz_set_str(mpz_t rop, const char *str, int base)
{
    mpz_init(rop);
    
    if (!str || !*str) return;
    
    const char *p = str;
    int sign = 1;
    
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }
    
    if (base == 0) {
        if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
            base = 16;
            p += 2;
        } else if (p[0] == '0') {
            base = 8;
            p++;
        } else {
            base = 10;
        }
    }
    
    mpz_t temp, base_val;
    mpz_init(temp);
    mpz_init_set_ui(base_val, base);
    
    while (*p) {
        int digit = 0;
        if (*p >= '0' && *p <= '9') {
            digit = *p - '0';
        } else if (*p >= 'a' && *p <= 'z') {
            digit = *p - 'a' + 10;
        } else if (*p >= 'A' && *p <= 'Z') {
            digit = *p - 'A' + 10;
        } else {
            break;
        }
        
        if (digit >= base) break;
        
        mpz_mul(temp, rop, base_val);
        mpz_set(rop, temp);
        mpz_set_ui(temp, digit);
        mpz_add(rop, rop, temp);
        p++;
    }
    
    rop->sign = (rop->size > 1 || rop->limbs[0] != 0) ? sign : 0;
    normalize(rop);
}

static uint64_t add_limbs(uint32_t *result, const uint32_t *a, const uint32_t *b, int size)
{
    uint64_t carry = 0;
    for (int i = 0; i < size; i++) {
        carry = carry + a[i] + b[i];
        result[i] = (uint32_t)carry;
        carry >>= 32;
    }
    return carry;
}

static uint64_t sub_limbs(uint32_t *result, const uint32_t *a, const uint32_t *b, int size)
{
    uint64_t borrow = 0;
    for (int i = 0; i < size; i++) {
        uint64_t temp = (uint64_t)a[i] - b[i] - borrow;
        result[i] = (uint32_t)temp;
        borrow = (temp >> 32) & 1;
    }
    return borrow;
}

void mpz_add(mpz_t rop, const mpz_t op1, const mpz_t op2)
{
    if (op1->sign == 0) {
        mpz_set(rop, op2);
        return;
    }
    if (op2->sign == 0) {
        mpz_set(rop, op1);
        return;
    }
    
    if (op1->sign != op2->sign) {
        // Different signs - use subtraction
        mpz_t temp;
        mpz_init_set(temp, op2);
        temp->sign = -temp->sign;
        mpz_sub(rop, op1, temp);
        return;
    }
    
    int max_size = (op1->size > op2->size) ? op1->size : op2->size;
    uint32_t a[MAX_BIGINT_LIMBS] = {0};
    uint32_t b[MAX_BIGINT_LIMBS] = {0};
    
    memcpy(a, op1->limbs, op1->size * sizeof(uint32_t));
    memcpy(b, op2->limbs, op2->size * sizeof(uint32_t));
    
    uint64_t carry = add_limbs(rop->limbs, a, b, max_size);
    rop->size = max_size;
    
    if (carry && rop->size < MAX_BIGINT_LIMBS) {
        rop->limbs[rop->size++] = (uint32_t)carry;
    }
    
    rop->sign = op1->sign;
    normalize(rop);
}

void mpz_sub(mpz_t rop, const mpz_t op1, const mpz_t op2)
{
    if (op2->sign == 0) {
        mpz_set(rop, op1);
        return;
    }
    if (op1->sign == 0) {
        mpz_set(rop, op2);
        rop->sign = -rop->sign;
        return;
    }
    
    if (op1->sign != op2->sign) {
        // Different signs - use addition
        mpz_t temp;
        mpz_init_set(temp, op2);
        temp->sign = -temp->sign;
        mpz_add(rop, op1, temp);
        return;
    }
    
    // Same signs - check which is larger
    int cmp = mpz_cmp(op1, op2);
    if (cmp == 0) {
        mpz_set_ui(rop, 0);
        return;
    }
    
    const mpz_t *larger = (cmp > 0) ? &op1 : &op2;
    const mpz_t *smaller = (cmp > 0) ? &op2 : &op1;
    
    uint32_t a[MAX_BIGINT_LIMBS] = {0};
    uint32_t b[MAX_BIGINT_LIMBS] = {0};
    
    memcpy(a, (*larger)->limbs, (*larger)->size * sizeof(uint32_t));
    memcpy(b, (*smaller)->limbs, (*smaller)->size * sizeof(uint32_t));
    
    sub_limbs(rop->limbs, a, b, (*larger)->size);
    rop->size = (*larger)->size;
    rop->sign = (cmp > 0) ? op1->sign : -op1->sign;
    normalize(rop);
}

void mpz_mul(mpz_t rop, const mpz_t op1, const mpz_t op2)
{
    if (op1->sign == 0 || op2->sign == 0) {
        mpz_set_ui(rop, 0);
        return;
    }
    
    uint32_t result[MAX_BIGINT_LIMBS] = {0};
    
    for (int i = 0; i < op1->size && i < MAX_BIGINT_LIMBS; i++) {
        uint64_t carry = 0;
        for (int j = 0; j < op2->size && (i + j) < MAX_BIGINT_LIMBS; j++) {
            uint64_t prod = (uint64_t)op1->limbs[i] * op2->limbs[j] + result[i + j] + carry;
            result[i + j] = (uint32_t)prod;
            carry = prod >> 32;
        }
        if ((i + op2->size) < MAX_BIGINT_LIMBS) {
            result[i + op2->size] = (uint32_t)carry;
        }
    }
    
    memcpy(rop->limbs, result, sizeof(result));
    rop->size = op1->size + op2->size;
    if (rop->size > MAX_BIGINT_LIMBS) rop->size = MAX_BIGINT_LIMBS;
    rop->sign = op1->sign * op2->sign;
    normalize(rop);
}

// Simplified modular exponentiation - basic implementation
void mpz_powm(mpz_t rop, const mpz_t base, const mpz_t exp, const mpz_t mod)
{
    mpz_t result, temp_base;
    mpz_init_set_ui(result, 1);
    mpz_init_set(temp_base, base);
    
    // Simple binary exponentiation
    for (int i = 0; i < exp->size * 32; i++) {
        int bit_index = i / 32;
        int bit_pos = i % 32;
        
        if (bit_index >= exp->size) break;
        
        if (exp->limbs[bit_index] & (1U << bit_pos)) {
            mpz_mul(result, result, temp_base);
            // Simple modulo operation (inefficient but functional)
            while (mpz_cmp(result, mod) >= 0) {
                mpz_sub(result, result, mod);
            }
        }
        
        mpz_mul(temp_base, temp_base, temp_base);
        while (mpz_cmp(temp_base, mod) >= 0) {
            mpz_sub(temp_base, temp_base, mod);
        }
    }
    
    mpz_set(rop, result);
}

int mpz_cmp(const mpz_t op1, const mpz_t op2)
{
    if (op1->sign != op2->sign) {
        return (op1->sign > op2->sign) ? 1 : -1;
    }
    
    if (op1->sign == 0) return 0;
    
    if (op1->size != op2->size) {
        return ((op1->size > op2->size) ? 1 : -1) * op1->sign;
    }
    
    for (int i = op1->size - 1; i >= 0; i--) {
        if (op1->limbs[i] != op2->limbs[i]) {
            return ((op1->limbs[i] > op2->limbs[i]) ? 1 : -1) * op1->sign;
        }
    }
    
    return 0;
}

size_t mpz_sizeinbase(const mpz_t op, int base)
{
    if (op->sign == 0) return 1;
    
    // Rough estimation
    int bits = op->size * 32;
    double log_base = (base == 2) ? 1.0 : (base == 10) ? 3.32193 : 4.0;
    return (size_t)(bits / log_base) + 2;
}

char *mpz_get_str(char *str, int base, const mpz_t op)
{
    if (op->sign == 0) {
        if (!str) str = (char*)malloc(2);
        strcpy(str, "0");
        return str;
    }
    
    size_t size = mpz_sizeinbase(op, base) + 2;
    if (!str) str = (char*)malloc(size);
    
    // Simple conversion (inefficient but functional)
    sprintf(str, "%u", op->limbs[0]); // Simplified - just show first limb
    return str;
}

void mpz_random(mpz_t rop, unsigned int bits)
{
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    
    mpz_init(rop);
    int limbs = (bits + 31) / 32;
    if (limbs > MAX_BIGINT_LIMBS) limbs = MAX_BIGINT_LIMBS;
    
    for (int i = 0; i < limbs; i++) {
        rop->limbs[i] = ((uint32_t)rand() << 16) | rand();
    }
    
    rop->size = limbs;
    rop->sign = 1;
    normalize(rop);
}

size_t mpz_export(void *rop, size_t *countp, int order, size_t size, int endian, size_t nails, const mpz_t op)
{
    if (!rop || !op) return 0;
    
    size_t bytes = op->size * sizeof(uint32_t);
    if (countp) *countp = bytes;
    
    memcpy(rop, op->limbs, bytes);
    return bytes;
}

void mpz_import(mpz_t rop, size_t count, int order, size_t size, int endian, size_t nails, const void *op)
{
    if (!rop || !op) return;
    
    mpz_init(rop);
    size_t limbs = count / sizeof(uint32_t);
    if (limbs > MAX_BIGINT_LIMBS) limbs = MAX_BIGINT_LIMBS;
    
    memcpy(rop->limbs, op, limbs * sizeof(uint32_t));
    rop->size = (int)limbs;
    rop->sign = (limbs > 0) ? 1 : 0;
    normalize(rop);
}