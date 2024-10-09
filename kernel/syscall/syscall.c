#include <sys/syscall.h>
#include <asm/regs.h>

long (*syscall[NUM_SYSCALLS])();

void handle_syscall(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
    /* TODO: [p2-task3] handle syscall exception */
    /**
     * HINT: call syscall function like syscall[fn](arg0, arg1, arg2),
     * and pay attention to the return value and sepc
     */
    regs->regs[OFFSET_REG_A0 >> 3] = syscall[regs->regs[OFFSET_REG_A7 >> 3]](
        regs->regs[OFFSET_REG_A0 >> 3],
        regs->regs[OFFSET_REG_A1 >> 3],
        regs->regs[OFFSET_REG_A2 >> 3],
        regs->regs[OFFSET_REG_A3 >> 3],
        regs->regs[OFFSET_REG_A4 >> 3]);
    regs->regs[OFFSET_REG_SEPC >> 3] = regs->regs[OFFSET_REG_SEPC >> 3] + 4;
}
