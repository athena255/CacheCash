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
        uint8_t *mem = (uint8_t*) malloc(MAX_BYTE*BLOCK_LEN);
        void* mem_addr;
        size_t mix_i;
        uint64_t miss_time, hit_time;
        for (volatile int i = 0; i < MAX_BYTE; ++i)
        {
            mix_i = MIX(i);
            mem_addr = MEM_ADD(mem, mix_i*BLOCK_LEN);

            if (mix_i & 1)
            {
                flush(mem_addr);
                miss_time = load(mem_addr);
                min_miss = (min_miss < miss_time) ? min_miss : miss_time;
            }
            else
            {
                load(mem_addr);
                hit_time = load(mem_addr);
                max_hit = (max_hit > hit_time) ? max_hit : hit_time;
            }
        }
        free(mem);
        D("max_hit: " << max_hit << " min_miss " << min_miss);
        if (max_hit >= min_miss)
            ERR("cannot differentiate between hits and misses");
    }
    return (get_max_hit) ? max_hit : min_miss;
}

