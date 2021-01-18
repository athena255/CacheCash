//
// Created by sage on 1/11/21.
//

#ifndef CACHECASH_UTILS_H
#define CACHECASH_UTILS_H

#define ERR(msg) do {perror(msg); exit(EXIT_FAILURE);} while(0)
#ifdef _DEBUG
#define D(msg) do{std::cout << msg << std::endl;}while(0);
#else
#define D(msg)do{}while(0);
#endif
#define MEM_ADD(addr, off) reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(addr) + off)
#define CVMEM_ADD(addr, off) reinterpret_cast<void volatile * const>(reinterpret_cast<uint8_t volatile * const>(addr) + off)
#define PTR_ADD(type, mem, off) reinterpret_cast<type>(reinterpret_cast<uint8_t const*>(mem) + (off))

#define RAND(min, max) (rand()%(max-min + 1) + min)

// Require: bit < 32
#define RD_BIT(reg, bit)  ( (bool)( reg & (1 << bit)) )

// Require: hi >= lo
#define RD_INT(reg, hi, lo)  ( ( reg & (((1ul << (1 + hi - lo)) - 1) << lo) ) >> lo )

// Mix up the index order to prevent stride prediction
// Require: 0 <= i <= 255
// Can replace 173 and 17, prime numbers seem to work best
#define MIX(_i) (((_i*167) + 11) & 255)

#endif //CACHECASH_UTILS_H
