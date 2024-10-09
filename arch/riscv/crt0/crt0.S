#include <asm.h>

.section ".entry_function","ax"
ENTRY(_start)

    /* TODO: [p1-task3] setup C runtime environment for the user program */
    mv t0, sp
    addi t0, t0, -16
    sd ra, 0(t0)
    sd sp, 8(t0)
    mv sp, t0
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

    /* TODO: [p1-task3] enter main function */
    call main

    /* TODO: [p1-task3] finish task and return to the kernel, replace this in p3-task2! */
    ld ra, 0(sp)
    ld sp, 8(sp)
    // li a7, 1
    // ecall
    ret

    /************************************************************/
	/* Do not touch this comment. Reserved for future projects. */
	/************************************************************/
// while(1) loop, unreachable here
loop:
    wfi
    j loop

END(_start)
