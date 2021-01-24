//
// Created by sage on 1/8/21.
//

#include <stdafx.h>
#include <cpu_info.h>
#include <cacheutils.h>

uint64_t get_cachelinesize()
{
    static uint64_t linesize = 0;
    // Assuming that clflush size is the cacheline size
    if (!linesize)
    {
        linesize = RD_INT(CPUInfo(1).cpui.EBX, 15, 8)*8;
    }
    return linesize;
}

uint64_t get_thresh(bool get_max_hit)
{
    static uint64_t max_hit = HIT_THRESH;
    static uint64_t min_miss = MISS_THRESH;
    if (max_hit == 0 && min_miss == -1)
    {
        uint64_t block_len = get_cachelinesize()*8;
        auto mem = mmap(NULL, MAX_BYTE*block_len, PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);

        void* mem_addr;
        size_t mix_i;
        uint64_t miss_time, hit_time;
        for (volatile int i = 0; i < MAX_BYTE; ++i)
        {
            mix_i = MIX(i);
            mem_addr = MEM_ADD(void*, mem, mix_i*block_len);

            flush(mem_addr);
            miss_time = load(mem_addr);
            min_miss = (min_miss < miss_time) ? min_miss : miss_time;

            hit_time = load(mem_addr);
            max_hit = (max_hit > hit_time) ? max_hit : hit_time;

        }
        munmap(mem, MAX_BYTE*block_len);
        D("[*] max_hit: %llu min_miss: %llu\n", max_hit, min_miss);
        if (max_hit >= min_miss)
            ERR("[!] cannot differentiate between hits and misses (max_hit %llu) (min_hit %llu)", max_hit, min_miss);
    }
    return (get_max_hit) ? max_hit : min_miss;
}

