/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>
#include <csr.h>

#define KERNEL_STACK		0x50500000

.section ".entry_function","ax"
ENTRY(_start)
  /* Mask all interrupts */
  csrw CSR_SIE, zero
  csrw CSR_SIP, zero

  /* TODO: [p1-task2] clear BSS for flat non-ELF images */
  la t0, __bss_start
  la t1, __BSS_END__
  bge t0, t1, end_clear

clear:
  andi t2, t0, 7
  beqz t2, aligned_8_clear

  sb zero, 0(t0)
  addi t0, t0, 1
  bge t0, t1, end_clear
  j clear

aligned_8_clear:
  bge t0, t1, end_clear
  sd zero, 0(t0)
  addi t0, t0, 8
  j aligned_8_clear

end_clear:

  /* TODO: [p1-task2] setup C environment */
  li sp, KERNEL_STACK
  j main

loop:
  wfi
  j loop

END(_start)
