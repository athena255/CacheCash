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
#define PTR_ADD(type, mem, off) reinterpret_cast<type>(reinterpret_cast<uint8_t *>(mem) + (off))

// Require: bit < 32
#define RD_BIT(reg, bit)  ( (bool)( reg & (1 << bit)) )

// Require: hi >= lo
#define RD_INT(reg, hi, lo)  ( ( reg & (((1ul << (1 + hi - lo)) - 1) << lo) ) >> lo )

// Mix up the index order to prevent stride prediction
// Require: 0 <= i <= 255
// Can replace 173 and 17, prime numbers seem to work best
#define MIX(_i) (((_i*167) + 11) & 255)

static inline void * map_file(char const *file_name, size_t *n_bytes)
{
    auto fd = open(file_name, O_RDONLY, 0);
    if (fd < 0)
        ERR("[!] Failed to open file");

    struct stat buf{};
    stat(file_name, &buf);
    void *seg = mmap(0, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (seg == MAP_FAILED)
        ERR("[!] Failed to map file");
    close(fd);

    *n_bytes = buf.st_size;


    return seg;
}

static inline void* page_align(void const *_addr)
{
    auto addr       = reinterpret_cast<unsigned long>(_addr);
    auto pagesize   = sysconf(_SC_PAGE_SIZE);
    auto offset     = addr % pagesize;

    return reinterpret_cast<void*>(addr - offset);
}

static inline void set_permissions(void const *addr_begin, int len, int permissions)
{
    auto aligned_start = page_align(addr_begin);
    auto aligned_len = reinterpret_cast<uint8_t const*>(addr_begin) + len - reinterpret_cast<uint8_t*>(aligned_start);
    if (mprotect(aligned_start, aligned_len, permissions) == -1)
        ERR("[!] mprotect failed");
}


#endif //CACHECASH_UTILS_H
