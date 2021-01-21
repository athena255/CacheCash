//
// Created by sage on 1/17/21.
//
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <lilelf.h>

LilElf::LilElf(const char* file_name)
        : data(reinterpret_cast<uint8_t*>(LilElf::map_file(file_name))),
          p_hdr{},
          p_dynsym_hdr{},
          p_sym_hdr{},
          p_str_hdr{}
{
    p_hdr = reinterpret_cast<Elf64_Ehdr*>(data);
    p_str_hdr = get_section(p_hdr->e_shstrndx-1); // TODO: ???
    process_sections();
    process_symtab();
}

void * LilElf::map_file(char const *file_name)
{
    auto fd = open(file_name, O_RDONLY, 0);
    if (fd < 0)
        ERR("[!] Failed to open file");

    struct stat buf{};
    stat(file_name, &buf);
    void *seg = mmap(0, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (seg == MAP_FAILED)
        ERR("[!] Failed to map victim file");
    close(fd);

    return seg;
}

void LilElf::process_sections()
{
    Elf64_Shdr *p_shdr;
    for (auto i = 0; i < p_hdr->e_shnum; ++i)
    {
        p_shdr = get_section(i);
        switch(p_shdr->sh_type)
        {
            case SHT_SYMTAB:
                p_sym_hdr = p_shdr;
                break;
            case SHT_DYNSYM:
                p_dynsym_hdr = p_shdr;
                break;
            case SHT_STRTAB:
//                    printf("found STRTAB at %d\n", i);
//                    for (auto i = 0; i < p_shdr->sh_size;)
//                    {
//                        i += printf("%s\n", PTR_ADD(char*, data, p_shdr->sh_offset + i));
//                    }
                break;
        }
    }
}

void LilElf::process_symtab()
{
    Elf64_Sym *p_sym;
    const char *key;
    for (auto i = 0; i < p_sym_hdr->sh_size/sizeof(Elf64_Sym); ++i)
    {
        p_sym = get_sym(i);
        key = get_str(p_sym->st_name);
        if (symtab.count(key) == 0)
        {
            symtab[key] = p_sym;
        }
//        else
//        {
//            D("[!] multiple symbols present: " << key)
//        }
//            printf("%s\n\tstr_idx: %d, sym_val: %p, size: %d, section_idx: %d, st_info %d\n",
//                   get_str(p_sym->st_name) , p_sym->st_name, p_sym->st_value, p_sym->st_size, p_sym->st_shndx, p_sym->st_info);
    }
}

void LilElf::set_permissions(void const *addr_begin, int len, int permissions)
{
    auto aligned_start = page_align(addr_begin);
    auto aligned_len = reinterpret_cast<uint8_t const*>(addr_begin) + len - reinterpret_cast<uint8_t*>(aligned_start);
    if (mprotect(aligned_start, aligned_len, permissions) == -1)
        ERR("[!] mprotect failed");
}

void* LilElf::page_align(void const *_addr)
{
    auto addr       = reinterpret_cast<unsigned long>(_addr);
    auto pagesize   = sysconf(_SC_PAGE_SIZE);
    auto offset     = addr % pagesize;

    return reinterpret_cast<void*>(addr - offset);
}
