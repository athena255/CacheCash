//
// Created by sage on 1/17/21.
//

#ifndef CACHECASH_LILELF_H
#define CACHECASH_LILELF_H
#include <stdafx.h>

#include <unordered_map>
#include <string>
#include <elf.h>

class LilElf{
public:
    explicit LilElf(const char* file_name);
    ~LilElf();

    inline Elf64_Shdr *get_section(size_t section_idx)
    {
        if (section_idx > p_hdr->e_shnum)
        {
            ERR("[!] Bad section idx");
        }
        return MEM_ADD(Elf64_Shdr*, data, p_hdr->e_shoff + section_idx*p_hdr->e_shentsize);
    }
    inline Elf64_Sym *get_sym(size_t sym_idx)
    {
        if (sym_idx > p_sym_hdr->sh_size/sizeof(Elf64_Sym))
        {
            ERR("[!] Bad symbol idx");
        }
        return MEM_ADD(Elf64_Sym*, data,p_sym_hdr->sh_offset + sym_idx*sizeof(Elf64_Sym));
    }
    inline const char *get_str(size_t str_idx)
    {
        if (str_idx > p_str_hdr->sh_size)
        {
            ERR("[!] Bad string idx");
        }
        return MEM_ADD(const char *, data,p_str_hdr->sh_offset + str_idx);
    }
    inline Elf64_Sym *get_sym(const char *sym_name)
    {
        return symtab[sym_name];
    }

    // Gets the virtual address of sym relative to data
    // Assuming that the symbol is NOT relocatable
    template <typename T>
    inline T get_sym_value(Elf64_Sym *sym)
    {
        // if symbol is not relocatable, then st_value is an absolute address
        // st_value is a section-relative offst if symbol is relocatable
        return reinterpret_cast<T>(&data[sym->st_value]);
    }

    inline uint8_t& operator[](int i) const
    {
        return data[i];
    }

    // Gets the file offset of sym relative to data
    // This is useful for looking at initialization values
    template <typename T>
    inline T get_sym_def(Elf64_Sym *sym)
    {
        // Get the section where this symbol is defined
        auto p_sec = get_section(sym->st_shndx);
        // subtract sym's VA from VA of its section and then add the file offset of its
        // section
        return reinterpret_cast<T>(&data[sym->st_value - p_sec->sh_addr + p_sec->sh_offset]);
    }

    inline uint8_t* search_bytes(char *pattern, size_t pattern_len, size_t offset = 0)
    {
        return find_pattern(reinterpret_cast<uint8_t*>(pattern), pattern_len, data, data_size, offset);
    }

private:
    void process_sections();
    void process_symtab();

    uint8_t* data;
    size_t data_size;
    Elf64_Ehdr *p_hdr;
    Elf64_Shdr *p_dynsym_hdr;
    Elf64_Shdr *p_sym_hdr;
    Elf64_Shdr *p_str_hdr;
    std::unordered_map<std::string, Elf64_Sym*> symtab;
};


#endif //CACHECASH_LILELF_H
