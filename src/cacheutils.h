//
// Created by sage on 1/8/21.
//

#ifndef CACHECASH_CACHEUTILS_H
#define CACHECASH_CACHEUTILS_H

inline volatile uint64_t load(const volatile void* mem)
{
    volatile uint64_t diff;
//    __asm__ volatile(
//            "mfence;"
//            "rdtsc;"
//            "movl %%eax, %%esi;"    // save tsc in (volatile register) esi
//            "prefetch (%%rdi);"     // load thing at rdi into all cache levels
//            "mfence;"               // force the prefetch to finish
//            "rdtsc;"
//            "subl %%esi, %%eax;"    // subtract old tsc from new tsc
//            : "=A"(diff)
//            : "D"(mem)
//            : "%esi", "%rcx", "%rdx");

    __asm__ volatile(
            "rdtscp;"
            "movl %%eax, %%esi;"    // save tsc in (volatile register) esi
            "movb (%%rdi), %%al;"   // load the byte at mem
            "rdtscp;"
            "subl %%esi, %%eax;"    // subtract old tsc from new tsc
            : "=A"(diff)
            : "D"(mem)
            : "%esi", "%rcx", "%rdx");
    return diff;
}

inline volatile uint64_t flush(const volatile void* mem)
{
    volatile uint64_t diff;
    __asm__ volatile(
            "rdtscp;"
            "movl %%eax, %%esi;"
            "mfence;"               // ensure all stores included in the cache line are flushed
            "clflushopt (%%rdi);"
//            "mfence;"               // avoid invalidating prefetched cache lines
            "rdtscp;"
            "subl %%esi, %%eax;"
            : "=A"(diff)
            : "D"(mem)
            : "%esi", "%rcx", "%rdx");
    return diff;
}

uint64_t get_cachelinesize();

uint64_t get_thresh(bool get_max_hit);

inline bool is_hit(uint64_t load_time)
{
    return load_time <= get_thresh(true);
}

inline bool is_miss(uint64_t load_time)
{
    return load_time >= get_thresh(false);
}

#endif //CACHECASH_CACHEUTILS_H
