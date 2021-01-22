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

    inline Elf64_Shdr *get_section(size_t section_idx)
    {
        if (section_idx > p_hdr->e_shnum)
        {
            ERR("[!] Bad section idx");
        }
        return PTR_ADD(Elf64_Shdr*, data, p_hdr->e_shoff + section_idx*p_hdr->e_shentsize);
    }
    inline Elf64_Sym *get_sym(size_t sym_idx)
    {
        if (sym_idx > p_sym_hdr->sh_size/sizeof(Elf64_Sym))
        {
            ERR("[!] Bad symbol idx");
        }
        return PTR_ADD(Elf64_Sym*, data,p_sym_hdr->sh_offset + sym_idx*sizeof(Elf64_Sym));
    }
    inline const char *get_str(size_t str_idx)
    {
        if (str_idx > p_str_hdr->sh_size)
        {
            ERR("[!] Bad string idx");
        }
        return PTR_ADD(const char *, data,p_str_hdr->sh_offset + str_idx);
    }
    inline Elf64_Sym *get_sym(const char *sym_name)
    {
        return symtab[sym_name];
    }

    // Gets the virtual address of sym relative to data
    template <typename T>
    inline T* get_sym_value(Elf64_Sym *sym)
    {
        return reinterpret_cast<T*>(&data[sym->st_value]);
    }

    // Gets the file offset of sym relative to data
    // This is useful for looking at initialization values
    template <typename T>
    inline T* get_static_sym_value(Elf64_Sym *sym)
    {
        auto p_sec = get_section(sym->st_shndx);
        // subtract sym's VA from VA of its section and then add the file offset of its
        // section
        return reinterpret_cast<T*>(&data[sym->st_value - p_sec->sh_addr + p_sec->sh_offset]);
    }

private:
    void process_sections();
    void process_symtab();

    uint8_t* data;
    Elf64_Ehdr *p_hdr;
    Elf64_Shdr *p_dynsym_hdr;
    Elf64_Shdr *p_sym_hdr;
    Elf64_Shdr *p_str_hdr;
    std::unordered_map<std::string, Elf64_Sym*> symtab;
};


#endif //CACHECASH_LILELF_H
