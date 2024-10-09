#include <os/irq.h>
#include <os/time.h>
#include <os/sched.h>
#include <os/string.h>
#include <os/kernel.h>
#include <printk.h>
#include <assert.h>
#include <screen.h>
#include <csr.h>

handler_t irq_table[IRQC_COUNT];
handler_t exc_table[EXCC_COUNT];

void interrupt_helper(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
    // TODO: [p2-task3] & [p2-task4] interrupt handler.
    // call corresponding handler by the value of `scause`
    if (scause & SCAUSE_IRQ_FLAG)
        irq_table[scause & ~SCAUSE_IRQ_FLAG](regs, stval, scause);
    else
        exc_table[scause](regs, stval, scause);
}

void handle_irq_timer(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
    // [p2-task4] clock interrupt handler.
    // Note: use bios_set_timer to reset the timer and remember to reschedule
    bios_set_timer((uint64_t)(get_ticks() + TIMER_INTERVAL));
    do_scheduler();
}

void init_exception()
{
    /* [p2-task3] initialize exc_table */
    /* NOTE: handle_syscall, handle_other, etc.*/
    exc_table[EXCC_SYSCALL]          = (handler_t)handle_syscall;
    exc_table[EXCC_INST_MISALIGNED]  = (handler_t)handle_other;
    exc_table[EXCC_INST_ACCESS]      = (handler_t)handle_other;
    exc_table[EXCC_BREAKPOINT]       = (handler_t)handle_other;
    exc_table[EXCC_LOAD_ACCESS]      = (handler_t)handle_other;
    exc_table[EXCC_STORE_ACCESS]     = (handler_t)handle_other;
    exc_table[EXCC_INST_PAGE_FAULT]  = (handler_t)handle_other;
    exc_table[EXCC_LOAD_PAGE_FAULT]  = (handler_t)handle_other;
    exc_table[EXCC_STORE_PAGE_FAULT] = (handler_t)handle_other;
    /* [p2-task4] initialize irq_table */
    /* NOTE: handle_int, handle_other, etc.*/
    irq_table[IRQC_U_SOFT]  = (handler_t)handle_other;
    irq_table[IRQC_S_SOFT]  = (handler_t)handle_other;
    irq_table[IRQC_M_SOFT]  = (handler_t)handle_other;
    irq_table[IRQC_U_TIMER] = (handler_t)handle_other;
    irq_table[IRQC_S_TIMER] = (handler_t)handle_irq_timer;
    irq_table[IRQC_M_TIMER] = (handler_t)handle_other;
    irq_table[IRQC_U_EXT]   = (handler_t)handle_other;
    irq_table[IRQC_S_EXT]   = (handler_t)handle_other;
    irq_table[IRQC_M_EXT]   = (handler_t)handle_other;

    /* [p2-task3] set up the entrypoint of exceptions */
    setup_exception();
}

void handle_other(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
    char* reg_name[] = {
        "zero "," ra  "," sp  "," gp  "," tp  ",
        " t0  "," t1  "," t2  ","s0/fp"," s1  ",
        " a0  "," a1  "," a2  "," a3  "," a4  ",
        " a5  "," a6  "," a7  "," s2  "," s3  ",
        " s4  "," s5  "," s6  "," s7  "," s8  ",
        " s9  "," s10 "," s11 "," t3  "," t4  ",
        " t5  "," t6  "
    };
    for (int i = 0; i < 32; i += 3) {
        for (int j = 0; j < 3 && i + j < 32; ++j) {
            printk("%s : %016lx ",reg_name[i+j], regs->regs[i+j]);
        }
        printk("\n\r");
    }
    printk("sstatus: 0x%lx sbadaddr: 0x%lx scause: %lu\n\r",
           regs->sstatus, regs->sbadaddr, regs->scause);
    printk("sepc: 0x%lx\n\r", regs->sepc);
    printk("tval: 0x%lx cause: 0x%lx\n", stval, scause);
    assert(0);
}
