#include <common.h>

FILE *elf_fp = NULL;

void init_elf(const char *elf_file){
    if (elf_file != NULL){
        Log("Opening ELF %s", elf_file);
    }
}
