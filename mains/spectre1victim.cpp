//
// Created by sage on 1/17/21.
//
// Victim test program for reading secret from another process

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <iostream>

#define _SECRET_LEN 25
#define _BLOCK_LEN 1024

// [??] Initializing the unused* to something value makes the attack work better
const unsigned int array1_size = 16;
uint8_t unused[64] = {3};
const uint8_t array1[160] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t unused1[64] = {1};
static uint8_t array2[256*_BLOCK_LEN] = {1};
uint8_t unused2[64] = {0};

// [??] Not initializing secret to anything seems to make the attack work better
uint8_t secret[_SECRET_LEN] = {0xff, 0xfe, 0xfd, 0xde, 0xad, 0xbe, 0xef, 0x69, 'T', 'h', 'e', ' ', 'c', 'a', 'k', 'e'};

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
    auto m_secret = const_cast<uint8_t*>(secret);
    for (int i = 0; i < sizeof(secret); ++i)
    {
        m_secret[i] = rand();
        printf("%02x ", m_secret[i]);
    }
    printf("\n");

    for (auto i = 0; i < _BLOCK_LEN*256; i += 4096)
    {
        array2[i] = 1;
    }

    auto get_tsc = [](){uint32_t eax; asm volatile("rdtscp":"=a"(eax) : :"rcx", "rdx"); return eax; };
    // Read from stdin
    size_t user_input;
    uint32_t t1;
    while(std::cin >> user_input && user_input != EOF)
    {
        t1 = get_tsc();
        get_elem(user_input);
        printf("%llu: %llu\n", user_input, get_tsc() - t1);
    }
}
