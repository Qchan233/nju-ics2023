/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};


void isa_reg_display() {
  int n = 0;
  printf("%-3s: %#-10x %-10u\n", "pc", cpu.pc, cpu.pc);
  for (n = 0; n < 32; n++){
    word_t val = gpr(n);
    printf("%-3s: %#-10x %-10u\n", regs[n], val, val);
  }
  printf("%-6s: %#-10x %-10u\n", "mcause", cpu.mcause, cpu.mcause);
  printf("%-6s: %#-10x %-10u\n", "mstatus", cpu.mstatus, cpu.mstatus);
  printf("%-6s: %#-10x %-10u\n", "mepc", cpu.mepc, cpu.mepc);
  printf("%-6s: %#-10x %-10u\n", "mtvec", cpu.mtvec, cpu.mtvec);
  printf("%-6s: %#-10x %-10u\n", "mscratch", cpu.mscratch, cpu.mscratch);
  printf("%-6s: %#-10x %-10u\n", "satp", cpu.satp, cpu.satp);
  
}

word_t isa_reg_str2val(const char *s, bool *success) {
  int n = 0;
  for (n = 0; n < 32; n++){
    if (strcmp(s, regs[n]) == 0){
      if (success != NULL){
        *success = true;
      }
      return gpr(n); 
    }
  }

  if (strcmp(s, "mepc") == 0){
    *success = true;
    return cpu.mepc;
  }
  if (strcmp(s, "mstatus") == 0){
    *success = true;
    return cpu.mstatus;
  }
  if (strcmp(s, "mcause") == 0){
    *success = true;
    return cpu.mcause;
  }
  if (strcmp(s, "mtvec") == 0){
    *success = true;
    return cpu.mtvec;
  }
  if (strcmp(s, "mscratch") == 0){
    *success = true;
    return cpu.mscratch;
  }
  if (strcmp(s, "pc") == 0){
    *success = true;
    return cpu.pc;
  }

  if (success != NULL){
    *success = false;
  }
  return 0;
}
