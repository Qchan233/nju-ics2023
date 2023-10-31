#include <common.h>
#include <elf.h>
#include <string.h>

FILE *elf_fp = NULL;

#define NAME_BUF_SIZE 32
typedef struct Func_Interval
/*Interval [start, end)*/
{
    word_t start;
    word_t end;
    char func_name[NAME_BUF_SIZE + 1];
} Func_Interval;

Func_Interval *intervals;

void safe_read (void *__restrict ptr, size_t size, size_t n,
       FILE *__restrict stream)
{
    size_t ret = fread(ptr, size, n, stream);
    if (ret != n) {
        Log("File Read Error");
        exit(EXIT_FAILURE);
    }
}


static uint32_t func_count = 0;

void init_elf(const char *elf_file){
    if (elf_file != NULL){
        Log("Opening ELF %s", elf_file);
    }

    FILE *file = fopen(elf_file, "rb");
    Elf32_Ehdr ehdr;
    safe_read(&ehdr, sizeof(Elf32_Ehdr), 1, file);

    fseek(file, ehdr.e_shoff, SEEK_SET);
    Elf32_Shdr shdr[ehdr.e_shnum];
    safe_read(shdr, sizeof(Elf32_Shdr), ehdr.e_shnum, file);

    // Locate the .symtab and .strtab sections
    Elf32_Shdr *symtab_shdr = NULL;
    Elf32_Shdr *strtab_shdr = NULL;
    for (int i = 0; i < ehdr.e_shnum; ++i) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            symtab_shdr = &shdr[i];
        }
        if (shdr[i].sh_type == SHT_STRTAB && i != ehdr.e_shstrndx) {  // Avoid using section header string table
            strtab_shdr = &shdr[i];
        }
    }

    if (!symtab_shdr || !strtab_shdr) {
        fprintf(stderr, "Failed to find the .symtab or .strtab section\n");
        exit(1);
    }

    // Read the .strtab section into memory
    char *strtab = malloc(strtab_shdr->sh_size);
    fseek(file, strtab_shdr->sh_offset, SEEK_SET);
    safe_read(strtab, 1, strtab_shdr->sh_size, file);

    // Read and print the symbols from the .symtab section
    fseek(file, symtab_shdr->sh_offset, SEEK_SET);
    int symbol_count = symtab_shdr->sh_size / symtab_shdr->sh_entsize;
    for (int i = 0; i < symbol_count; ++i) {
        Elf32_Sym sym;
        safe_read(&sym, sizeof(Elf32_Sym), 1, file);
        if (ELF32_ST_TYPE(sym.st_info) == STT_FUNC){
            // printf("Symbol %d:\n", i);
            // printf("  Name: %s\n", strtab + sym.st_name);
            // printf("  Value: %08x\n", sym.st_value);
            // printf("  Size: %08x\n", sym.st_size);
            // printf("  Info: %02x\n", sym.st_info);
            // printf("  Other: %02x\n", sym.st_other);
            // printf("  Shndx: %04x\n", sym.st_shndx);
           func_count++; 
        }
    }

    fseek(file, symtab_shdr->sh_offset, SEEK_SET);
    intervals = malloc(func_count * sizeof(Func_Interval));
    func_count = 0;
    for (int i = 0; i < symbol_count; ++i) {
        Elf32_Sym sym;
        safe_read(&sym, sizeof(Elf32_Sym), 1, file);
        if (ELF32_ST_TYPE(sym.st_info) == STT_FUNC){
            Func_Interval current;
            current.start = sym.st_value;
            current.end = sym.st_value + sym.st_size;
            strncpy(current.func_name, strtab + sym.st_name, NAME_BUF_SIZE);
            Log("Start: %x, End:%x, Name:%s", current.start, current.end, current.func_name);
            func_count++; 
        }
    }

    free(strtab);
    fclose(file);
}

void check_call(word_t pc, word_t dnpc){
   uint32_t i;
   Log("checking calls\n");
   for(i = 0; i < func_count; i++){
    if (intervals[i].start == dnpc){
        Log("%#08x: call [%s@%#08x]\n", pc, intervals[i].func_name, dnpc);
        break;
    }
   } 
}