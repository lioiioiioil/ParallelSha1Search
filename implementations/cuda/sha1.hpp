#pragma once
#include <cstdint>

// Циклический сдвиг 32-битного числа влево.
__host__ __device__ inline std::uint32_t rotate(std::uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

// SHA1 только для пароля из 7 символов, как требуется в этом варианте.
__host__ __device__ inline void sha1(const char* password, char* hash) {
    std::uint32_t w[80] = {};
    for (int i = 0; i < 7; ++i)
        w[i / 4] |= std::uint32_t(static_cast<unsigned char>(password[i])) << (24 - 8 * (i % 4));
    w[1] |= 0x80; // После семи байтов добавляем бит 1, затем нули.
    w[15] = 56;   // Длина исходного пароля: 7 * 8 бит.
    for (int i = 16; i < 80; ++i)
        w[i] = rotate(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

    std::uint32_t h[5] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};
    std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
    for (int i = 0; i < 80; ++i) {
        std::uint32_t f, k;
        if (i < 20) { f = (b & c) | (~b & d); k = 0x5a827999; }
        else if (i < 40) { f = b ^ c ^ d; k = 0x6ed9eba1; }
        else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8f1bbcdc; }
        else { f = b ^ c ^ d; k = 0xca62c1d6; }
        std::uint32_t temp = rotate(a, 5) + f + e + k + w[i];
        e = d; d = c; c = rotate(b, 30); b = a; a = temp;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;

    // Пять 32-битных чисел превращаем в строку из 40 hex-символов.
    const char digits[] = "0123456789abcdef";
    int position = 0;
    for (int i = 0; i < 5; ++i)
        for (int shift = 28; shift >= 0; shift -= 4)
            hash[position++] = digits[(h[i] >> shift) & 15];
    hash[40] = '\0';
}
