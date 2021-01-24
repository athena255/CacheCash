//
// Created by sage on 1/17/21.
//
#include <lilelf.h>

LilElf::LilElf(const char* file_name)
        : p_dynsym_hdr{},
          p_sym_hdr{}
{
    data = reinterpret_cast<uint8_t*>(map_file(file_name, &data_size));
    p_hdr = reinterpret_cast<Elf64_Ehdr*>(data);
    p_str_hdr = get_section(p_hdr->e_shstrndx-1); // TODO: why is this -1?
    process_sections();
    process_symtab();
}

LilElf::~LilElf()
{
    symtab.clear();
    munmap(data, data_size);
    data_size = 0;
    data = nullptr;
    p_hdr = nullptr;
    p_str_hdr = nullptr;
    p_dynsym_hdr = nullptr;
    p_sym_hdr = nullptr;
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
                // There exists multiple SHT_STRTAB sections
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
        // Only map the first occurence of the symbol
        if (symtab.count(key) == 0)
        {
            symtab[key] = p_sym;
        }
    }
}

