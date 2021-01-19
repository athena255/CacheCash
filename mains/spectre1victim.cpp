//
// Created by sage on 1/17/21.
//
// Victim test program for reading secret from another process

#include <random>
#include <cstdint>

#define _SECRET_LEN 25
#define _BLOCK_LEN 512

unsigned int array1_size = 16;
uint8_t unused[64]; // seperate by a cacheline
uint8_t array1[160] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t unused2[64]; // seperate by a cacheline
volatile uint8_t *array2;

uint8_t secret[_SECRET_LEN] = {1, 2, 1, 2, 3, 69};

volatile uint8_t get_elem(size_t x)
{
    // Make this volatile or compiler might optimize out
    volatile uint8_t res = -1;
    if (x < array1_size)
    {
        res = array2[array1[x]*_BLOCK_LEN];
    }
    return res;
}

int main()
{
    srand(time(0));

    // Allocate for array2
    array2 = (uint8_t*)malloc(256*_BLOCK_LEN);

    // Generate secret
    for (int i = 0; i < sizeof(secret); ++i)
    {
        secret[i] = rand();
        printf("%02x ", secret[i]);
    }

    printf("\n");
    // Write some stuff in array2
    for (int i = 0; i < 16; ++i)
    {
        array2[i*_BLOCK_LEN] = i;
    }

    // Make sure array2 is backed
    for (auto i = 0; i < _BLOCK_LEN*256; i += 4096)
    {
        array2[i] = 1;
    }

    // Read from stdin
    size_t user_input;
    while(true)
    {
        scanf("%lu", &user_input);
        printf("--> 0x%02x\n", get_elem(user_input));
    }

}