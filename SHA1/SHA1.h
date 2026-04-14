#include <stddef.h>
#include <stdint.h>


static const uint32_t H0 = 0x67452301;
static const uint32_t H1 = 0xEFCDAB89;
static const uint32_t H2 = 0x98BADCFE;
static const uint32_t H3 = 0x10325476;
static const uint32_t H4 = 0xC3D2E1F0;

static const uint32_t K[4] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};


void SHA1_hash(const unsigned char *message, size_t length, uint8_t hash[20]);
uint8_t *SHA1_padding(const unsigned char *message, size_t length, size_t *padded_length);
void SHA1_compress_blocks(uint8_t *padded_buf, uint32_t *A, uint32_t *B,
                          uint32_t *C, uint32_t *D, uint32_t *E);
void SHA1_compression(char *message, size_t length);

uint32_t rotate_left (uint32_t word, int n);
void get_f_and_k_values(int i, uint32_t B_temp, uint32_t C_temp, 
                        uint32_t D_temp, uint32_t *k, uint32_t *f_result);
