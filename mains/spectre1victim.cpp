//
// Created by sage on 1/17/21.
//
// Victim test program for reading secret from another process

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <ctime>

#define _SECRET_LEN 25
#define _BLOCK_LEN 512
//#define _DEBUG

unsigned int array1_size = 16;
// [??] Initializing the unused* to something value makes the attack work better
uint8_t unused[64] = {3};
uint8_t array1[160] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t unused1[64];
uint8_t array2[256*_BLOCK_LEN];
uint8_t unused2[64];

// [??] Not initializing secret to anything seems to make the attack work better
uint8_t secret[_SECRET_LEN];

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
    // [??] Calling srand(time(0)) seems to make the attack work better
    srand(time(0));

    // Generate secret and print it
    for (int i = 0; i < sizeof(secret); ++i)
    {
        secret[i] = rand();
        printf("%02x ", secret[i]);
    }
    printf("\n");

#ifdef _DEBUG
    // Populate array2 with something interesting
    for (int i = 0; i < 16; ++i)
    {
        array2[i*_BLOCK_LEN] = i;
    }
    size_t counter = 0;
#endif

    // Make sure array2 is backed
    for (auto i = 0; i < _BLOCK_LEN*256; i += 4096)
    {
        array2[i] = 1;
    }

    // Read from stdin
    size_t user_input;
    while(scanf("%llu", &user_input) != EOF)
    {
#ifdef _DEBUG
        printf("[%llu] %llu --> 0x%02x\n", time(0), counter++, get_elem(user_input));
#else
        get_elem(user_input);
#endif
    }

}
