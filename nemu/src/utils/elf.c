#include <common.h>
#include <elf.h>

FILE *elf_fp = NULL;

struct Func_Interval
/*Interval [start, end)*/
{
    word_t start;
    word_t end;
};

struct Func_Interval * intervals;


void init_elf(const char *elf_file){
    if (elf_file != NULL){
        Log("Opening ELF %s", elf_file);
    }


}
