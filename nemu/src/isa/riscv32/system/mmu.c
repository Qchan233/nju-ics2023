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
#include <memory/vaddr.h>
#include <memory/paddr.h>

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  // printf("%x\n", vaddr);
  word_t vpn1 = (vaddr >> 22) & 0x3ff;
  word_t vpn0 = (vaddr >> 12) & 0x3ff;
  word_t offset = vaddr & 0xfff;

  word_t pte1 = paddr_read(((cpu.satp & 0x3fffff) << 12) + vpn1 * 4, 4);
  if (!(pte1 & 0x1)) {
    printf("vaddr %x\n", vaddr);
    printf("pc: %x\n", cpu.pc);
    printf("sp: %x\n", cpu.gpr[2]);
  }
  assert(pte1 & 0x1); // check valid bit
  word_t pte2 = paddr_read(((pte1 & 0xfffffc00) << 2) + vpn0 * 4, 4);
  assert(pte2 & 0x1); //check valid bit
  word_t translated = ((pte2 & 0xfffffc00) << 2) + offset;
  // assert(translated == vaddr);
  return translated;
}
