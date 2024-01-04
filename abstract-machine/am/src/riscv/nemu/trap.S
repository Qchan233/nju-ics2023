#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define MAP(c, f) c(f)

#if __riscv_xlen == 32
#define LOAD  lw
#define STORE sw
#define XLEN  4
#else
#define LOAD  ld
#define STORE sd
#define XLEN  8
#endif

#define REGS_LO16(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9) \
f(10) f(11) f(12) f(13) f(14) f(15)
#ifndef __riscv_e
#define REGS_HI16(f) \
                                    f(16) f(17) f(18) f(19) \
f(20) f(21) f(22) f(23) f(24) f(25) f(26) f(27) f(28) f(29) \
f(30) f(31)
#define NR_REGS 32
#else
#define REGS_HI16(f)
#define NR_REGS 16
#endif

#define REGS(f) REGS_LO16(f) REGS_HI16(f)

#define PUSH(n) STORE concat(x, n), (n * XLEN)(sp);
#define POP(n)  LOAD  concat(x, n), (n * XLEN)(sp);

#define CONTEXT_SIZE  ((NR_REGS + 3 + 1 + 1) * XLEN)
#define OFFSET_SP     ( 2 * XLEN)
#define OFFSET_CAUSE  ((NR_REGS + 0) * XLEN)
#define OFFSET_STATUS ((NR_REGS + 1) * XLEN)
#define OFFSET_EPC    ((NR_REGS + 2) * XLEN)
#define OFFSET_NP     ((NR_REGS + 4) * XLEN)
#define KERNEL 0
#define USER   1

.align 3
.globl __am_asm_trap
__am_asm_trap:
  STORE sp, OFFSET_SP(sp)

  csrrw sp, mscratch, sp   // (1) atomically exchange sp and mscratch
  bnez sp, save_context    // (2) take the branch if we trapped from user
  csrr sp, mscratch        // (3) if we trapped from kernel, restore the original sp

  save_context:
  addi sp, sp, -XLEN
  STORE t4, XLEN(sp)  // save t4
  
  li t4, USER
  STORE t4, (OFFSET_NP + XLEN)(sp)  // np = KERNEL;
  csrr t4, mscratch
  bnez t4, next_step // if (ksp != 0) np = USER
  li t4, KERNEL
  STORE t4, (OFFSET_NP + XLEN)(sp)  // np = USER;

  LOAD t4, XLEN(sp)   // restore t4
  addi sp, sp, XLEN

  next_step:
  csrw mscratch, x0       // ksp = 0;
  addi sp, sp, -CONTEXT_SIZE

  MAP(REGS, PUSH)

  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc

  STORE t0, OFFSET_CAUSE(sp)
  STORE t1, OFFSET_STATUS(sp)
  STORE t2, OFFSET_EPC(sp)

  # set mstatus.MPRV to pass difftest
  li a0, (1 << 17)
  or t1, t1, a0
  csrw mstatus, t1

  mv a0, sp
  jal __am_irq_handle
  mv sp, a0

restore_contexts:
  LOAD t0, OFFSET_CAUSE(sp)
  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)

  csrw mcause, t0
  csrw mstatus, t1
  csrw mepc, t2

  MAP(REGS, POP)

  addi sp, sp, -2 * XLEN
  STORE t1, (XLEN)(sp)
  STORE t2, (2 * XLEN)(sp)

  li t1, USER
  LOAD t2, (OFFSET_NP + 3 * XLEN)(sp)
  bne t1, t2, not_user
  csrw mscratch, sp

  not_user:
  LOAD t1, (XLEN)(t0)
  LOAD t2, (2 * XLEN)(t0)
  addi sp, sp, 2 * XLEN

  LOAD sp, OFFSET_SP(sp)

  // addi sp, sp, CONTEXT_SIZE
  mret
