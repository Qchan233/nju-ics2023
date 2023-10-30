#include <common.h>

FILE *elf_fp = NULL;

struct interval
/*Interval [start, end)*/
{
    word_t start;
    word_t end;
};


void init_elf(const char *elf_file){
    if (elf_file != NULL){
        Log("Opening ELF %s", elf_file);
    }

    
}
