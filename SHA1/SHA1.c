#include "SHA1.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint8_t *SHA1_padding(const unsigned char *message, size_t length, size_t *padded_length)
{
    *padded_length = ((length + 9) / 64 + 1) * 64;
    uint8_t *padded_buf = calloc(*padded_length, sizeof(uint8_t)); //calloc for zero padding
    if (padded_buf == NULL)
    {
        *padded_length = 0;
        return padded_buf;
    }

    memcpy(padded_buf, message, length);

    padded_buf[length] = 0x80;

    uint64_t bit_length = (uint64_t)length * 8;

    padded_buf[*padded_length - 8] = (uint8_t)(bit_length >> 56);
    padded_buf[*padded_length - 7] = (uint8_t)(bit_length >> 48);
    padded_buf[*padded_length - 6] = (uint8_t)(bit_length >> 40);
    padded_buf[*padded_length - 5] = (uint8_t)(bit_length >> 32);
    padded_buf[*padded_length - 4] = (uint8_t)(bit_length >> 24);
    padded_buf[*padded_length - 3] = (uint8_t)(bit_length >> 16);
    padded_buf[*padded_length - 2] = (uint8_t)(bit_length >> 8);
    padded_buf[*padded_length - 1] = (uint8_t)(bit_length);

    return padded_buf;
}


void SHA1_hash(const unsigned char *message, size_t length, uint8_t hash[20])
{
    size_t padded_length, i;
    uint8_t *padded_buf = SHA1_padding(message, length, &padded_length);
    uint32_t H[5] = {H0, H1, H2, H3, H4};

    for (i = 0; i < padded_length; i += 64)
    {
        SHA1_compress_blocks(padded_buf + i, &H[0], &H[1], &H[2], &H[3], &H[4]);
    }

    for(int i = 0; i < 5; i++) {
        hash[i*4 + 0] = H[i] >> 24;
        hash[i*4 + 1] = H[i] >> 16;
        hash[i*4 + 2] = H[i] >>  8;
        hash[i*4 + 3] = H[i];
    }

    free(padded_buf);

}

void SHA1_compress_blocks(uint8_t *padded_buf, uint32_t *A, uint32_t *B, uint32_t *C, uint32_t *D, uint32_t *E)
{
    uint32_t words[80];
    int i;

    uint32_t A_temp = *A, B_temp = *B, C_temp = *C,
             D_temp = *D, E_temp = *E, temp, k, f_result;

    for (i = 0; i < 16; i++)
    {
        words[i] = (padded_buf[i*4 + 0] << 24) | (padded_buf[i*4 + 1] << 16) |
                   (padded_buf[i*4 + 2] << 8) | (padded_buf[i*4 + 3] << 0);
    }

    for (i = 16; i < 80; i++)
    {
        words[i] = rotate_left(words[i-3] ^ words[i-8] ^ words[i-14] ^ words[i-16], 1);
    }

    for (i = 0; i < 80; i++)
    {
        get_f_and_k_values(i,B_temp, C_temp, D_temp, &k, &f_result);
        temp = rotate_left(A_temp, 5) + f_result + E_temp + k + words[i];
        E_temp = D_temp;
        D_temp = C_temp;
        C_temp = rotate_left(B_temp, 30);
        B_temp = A_temp;
        A_temp = temp;
    } 
    *A += A_temp;
    *B += B_temp;
    *C += C_temp;
    *D += D_temp;
    *E += E_temp;
}


uint32_t rotate_left (uint32_t word, int n)
{
    return ((word << n) | (word >> (32-n)));
}


void get_f_and_k_values(int i, uint32_t B_temp, uint32_t C_temp, uint32_t D_temp, uint32_t *k, uint32_t *f_result)
{
    *k = K[i/20];
    if (i<=19)
        *f_result = (B_temp & C_temp) | (~B_temp & D_temp);
    else if (i <= 39)
        *f_result = B_temp ^ C_temp ^ D_temp;
    else if (i <= 59)
        *f_result = (B_temp & C_temp) | (B_temp & D_temp) | (C_temp & D_temp);
    else
        *f_result = B_temp ^ C_temp ^ D_temp;

    return;
}
