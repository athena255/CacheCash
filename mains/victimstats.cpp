//
// Created by sage on 1/22/21.
//
#include <stdafx.h>
#include <cacheutils.h>

const unsigned int array1_size = 16;
uint8_t unused[64] = {3};
const uint8_t array1[160] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t unused1[64];
const uint8_t array2[256*512] = {0xde, 0xad, 0xbe, 0xef, 69, 69, 69, 111};
uint8_t unused2[64];

#define _BLOCK_LEN 512

volatile uint8_t get_elem(size_t x)
{
    // Make this volatile or compiler might optimize out
    volatile uint8_t res = -1;
    if (x < array1_size )
    {
        res = array2[array1[x]*_BLOCK_LEN];
    }
    return res;
}


int main()
{
    printf("VA of array2: %p\n", &array2);
    printf("VA of array1_size: %p\n", &array1_size);
    std::vector<int> times;
    size_t user_input;
    uint64_t load_time;
    while(std::cin >> user_input && user_input != EOF)
    {
//        get_elem(user_input);
        load_time = load((void*)&array1_size);
        printf("(%llu): %llu\n", user_input, load_time);
    }

    return 0;
}