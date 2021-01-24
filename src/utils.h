//
// Created by sage on 1/11/21.
//

#ifndef CACHECASH_UTILS_H
#define CACHECASH_UTILS_H

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

template <typename T>
static inline T* find_pattern(T *pattern, size_t pattern_len, T *data, size_t data_size, size_t offset = 0)
{
    size_t i, j;
    if (pattern_len > data_size - offset)
        return nullptr;
    for (i = offset; i < data_size; ++i)
    {
        for (j = 0ul; j < pattern_len; ++j)
        {
            if (data[i+j] != pattern[j])
            {
                break;
            }
        }
        if (j == pattern_len)
            return &data[i];
    }
    return nullptr;
}

static inline void ERR(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fflush(stderr);
    exit(EXIT_FAILURE);
}

static inline void * map_file(char const *file_name, size_t *n_bytes)
{
    auto fd = open(file_name, O_RDONLY, 0);
    if (fd < 0)
        ERR("[!] Failed to open file");

    struct stat buf{};
    stat(file_name, &buf);
    void *seg = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (seg == MAP_FAILED)
        ERR("[!] Failed to map file");

    *n_bytes = buf.st_size;

    return seg;
}

template <typename T>
static inline T page_align(T _addr)
{
    auto addr       = reinterpret_cast<uint64_t>(_addr);
    auto pagesize   = sysconf(_SC_PAGE_SIZE);
    auto offset     = addr % pagesize;

    return reinterpret_cast<T>(addr - offset);
}

static inline void set_permissions(void const *addr_begin, int len, int permissions)
{
    auto aligned_start = const_cast<void*>(page_align(addr_begin));
    auto aligned_len = reinterpret_cast<uint8_t const*>(addr_begin) + len - reinterpret_cast<const uint8_t*>(aligned_start);
    if (mprotect(aligned_start, aligned_len, permissions) == -1)
        ERR("[!] mprotect failed");
}


#endif //CACHECASH_UTILS_H
