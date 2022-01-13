#include <lib/base.h>
#include <sched.h>

#define ELF_HEADER_MAGIC "\177ELF"

typedef enum
{
    ELF_CLASS_INVALID = 0,
    ELF_CLASS_32 = 1,
    ELF_CLASS_64 = 2,
} ElfClass;

typedef enum
{
    ELF_ENCODING_INVALID = 0,
    ELF_ENCODING_LITTLE_ENDIAN = 1,
    ELF_ENCODING_BIG_ENDIAN = 2,
} ElfEncoding;

typedef enum
{
    ELF_TYPE_NONE = 0,
    ELF_TYPE_RELOCATABLE = 1,
    ELF_TYPE_EXECUTABLE = 2,
    ELF_TYPE_DYNAMIC = 3,
    ELF_TYPE_CORE = 4,
} ElfType;

typedef enum
{
    ELF_SECTION_HEADER_NULL = 0,
    ELF_SECTION_HEADER_PROGBITS = 1,
    ELF_SECTION_HEADER_SYMTAB = 2,
    ELF_SECTION_HEADER_STRTAB = 3,
    ELF_SECTION_HEADER_RELA = 4,
    ELF_SECTION_HEADER_NOBITS = 8,
    ELF_SECTION_HEADER_REL = 9,
} ElfSectionHeaderType;

typedef enum
{
    ELF_PROGRAM_HEADER_NULL = 0,
    ELF_PROGRAM_HEADER_LOAD = 1,
    ELF_PROGRAM_HEADER_DYNAMIC = 2,
    ELF_PROGRAM_HEADER_INTERPRET = 3,
    ELF_PROGRAM_HEADER_NOTE = 4,

} ElfProgramHeaderType;
typedef enum
{
    ELF_PROGRAM_HEADER_EXECUTABLE = 1 << 0,
    ELF_PROGRAM_HEADER_WRITABLE = 1 << 1,
    ELF_PROGRAM_HEADER_READABLE = 1 << 2,
} ElfProgramHeaderFlags;

typedef struct PACKED
{
    uint8_t magics[4];
    uint8_t elf_class;
    uint8_t data_encoding;
    uint8_t version;
    uint8_t os;
    uint8_t abi_version;
    uint8_t _padding[7];
} Elf64Ident;

typedef struct PACKED
{
    Elf64Ident ident;

    uint16_t object_type;
    uint16_t machine_type;
    uint32_t object_version;

    uint64_t entry;

    uint64_t program_header_table_file_offset;
    uint64_t section_header_table_file_offset;

    uint32_t flags;

    uint16_t elf_header_size;

    uint16_t program_header_table_entry_size;
    uint16_t program_header_table_entry_count;

    uint16_t section_header_table_entry_size;
    uint16_t section_header_table_entry_count;

    uint16_t section_header_string_table_idx;
} Elf64Header;

typedef struct PACKED
{
    uint32_t type;
    uint32_t flags;

    uint64_t file_offset;
    uint64_t virtual_address;
    uint64_t physical_address;

    uint64_t file_size;
    uint64_t memory_size;

    uint64_t alignment;
} Elf64ProgramHeader;
typedef struct
{
    uintptr_t at_entry;
    uintptr_t at_phdr;
    uintptr_t at_phent;
    uintptr_t at_phnum;
} Auxval;

typedef struct PACKED
{
    uint32_t name;
    uint32_t type;
    uint64_t flags;

    uint64_t virtual_address;
    uint64_t file_offset;
    uint64_t file_size;
    uint32_t link;
    uint32_t info;
    uint64_t addralign;
    uint64_t entry_size;
} Elf64SectionHeader;

inline bool elf_validate(Elf64Header const *header)
{
    return header->ident.magics[0] == '\177' &&
           header->ident.magics[1] == 'E' &&
           header->ident.magics[2] == 'L' &&
           header->ident.magics[3] == 'F';
}

uint64_t elf_load_program(uint64_t elf_base, uintptr_t base, Task *task, Auxval *auxval, char **ld_path);
