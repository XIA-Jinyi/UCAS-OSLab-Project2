#include <common.h>
#include <asm.h>
#include <asm/unistd.h>
#include <os/loader.h>
#include <os/irq.h>
#include <os/sched.h>
#include <os/lock.h>
#include <os/kernel.h>
#include <os/task.h>
#include <os/string.h>
#include <os/mm.h>
#include <os/time.h>
#include <sys/syscall.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>
#include <type.h>
#include <csr.h>
#include <asm/regs.h>

extern void ret_from_exception();

#define INPUT_BUFLEN 128

char input_buf[INPUT_BUFLEN];

// Task info array
task_info_t tasks[TASK_MAXNUM];


static void init_jmptab(void)
{
    volatile long (*(*jmptab))() = (volatile long (*(*))())KERNEL_JMPTAB_BASE;

    jmptab[CONSOLE_PUTSTR]  = (long (*)())port_write;
    jmptab[CONSOLE_PUTCHAR] = (long (*)())port_write_ch;
    jmptab[CONSOLE_GETCHAR] = (long (*)())port_read_ch;
    jmptab[SD_READ]         = (long (*)())sd_read;
    jmptab[SD_WRITE]        = (long (*)())sd_write;
    jmptab[QEMU_LOGGING]    = (long (*)())qemu_logging;
    jmptab[SET_TIMER]       = (long (*)())set_timer;
    jmptab[READ_FDT]        = (long (*)())read_fdt;
    jmptab[MOVE_CURSOR]     = (long (*)())screen_move_cursor;
    jmptab[PRINT]           = (long (*)())printk;
    jmptab[YIELD]           = (long (*)())do_scheduler;
    jmptab[MUTEX_INIT]      = (long (*)())do_mutex_lock_init;
    jmptab[MUTEX_ACQ]       = (long (*)())do_mutex_lock_acquire;
    jmptab[MUTEX_RELEASE]   = (long (*)())do_mutex_lock_release;

    // TODO: [p2-task1] (S-core) initialize system call table.
    jmptab[WRITE]           = (long (*)())screen_write;
    jmptab[REFLUSH]         = (long (*)())screen_reflush;
    jmptab[SLEEP]           = (long (*)())do_sleep;
    jmptab[GET_TIMEBASE]    = (long (*)())get_time_base;
    jmptab[GET_TICK]        = (long (*)())get_ticks;
}

static void init_task_info(void)
{
    // TODO: [p1-task4] Init 'tasks' array via reading app-info sector
    // NOTE: You need to get some related arguments from bootblock first
    bios_sd_read((uint64_t)tasks, 1, 1);
}

/************************************************************/
static void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb)
{
     /* TODO: [p2-task3] initialization of registers on kernel stack
      * HINT: sp, ra, sepc, sstatus
      * NOTE: To run the task in user mode, you should set corresponding bits
      *     of sstatus(SPP, SPIE, etc.).
      */
    regs_context_t *pt_regs =
        (regs_context_t *)(kernel_stack - sizeof(regs_context_t));
    pt_regs->regs[OFFSET_REG_SP >> 3] = user_stack;
    pt_regs->regs[OFFSET_REG_TP >> 3] = (reg_t)pcb;
    pt_regs->sepc = entry_point;
    pt_regs->sstatus = SR_SPIE;

    /* TODO: [p2-task1] set sp to simulate just returning from switch_to
     * NOTE: you should prepare a stack, and push some values to
     * simulate a callee-saved context.
     */
    switchto_context_t *pt_switchto =
        (switchto_context_t *)((ptr_t)pt_regs - sizeof(switchto_context_t));
    pcb->kernel_sp = (ptr_t)pt_switchto;
    pcb->user_sp = user_stack;
    pt_switchto->regs[SWITCH_TO_RA >> 3] = (reg_t)ret_from_exception;
    pt_switchto->regs[SWITCH_TO_SP >> 3] = (reg_t)pt_switchto;
}

static void init_pcb(void)
{
    /* TODO: [p2-task1] load needed tasks and init their corresponding PCB */
    for (int i = 0; tasks[i].name[0]; i++) {
        uint64_t entry_point = load_task_img(tasks[i].name);
        ptr_t kernel_stack = allocKernelPage(1) + PAGE_SIZE;
        ptr_t user_stack = allocUserPage(16) + PAGE_SIZE * 16;
        pcb[i].pid = i + 1;
        pcb[i].status = TASK_READY;
        init_list_head(&pcb[i].list);
        init_list_head(&pcb[i].thread_group);
        init_pcb_stack(kernel_stack, user_stack, entry_point, &pcb[i]);
        list_add_tail(&pcb[i].list, &ready_queue);
    }

    /* TODO: [p2-task1] remember to initialize 'current_running' */
    current_running = &pid0_pcb;
}

static void init_syscall(void)
{
    // [p2-task3] initialize system call table.
    syscall[SYSCALL_SLEEP]         = (long (*)())do_sleep;
    syscall[SYSCALL_YIELD]         = (long (*)())do_scheduler;
    syscall[SYSCALL_WRITE]         = (long (*)())screen_write;
    syscall[SYSCALL_CURSOR]        = (long (*)())screen_move_cursor;
    syscall[SYSCALL_REFLUSH]       = (long (*)())screen_reflush;
    syscall[SYSCALL_GET_TIMEBASE]  = (long (*)())get_time_base;
    syscall[SYSCALL_GET_TICK]      = (long (*)())get_ticks;
    syscall[SYSCALL_LOCK_INIT]     = (long (*)())do_mutex_lock_init;
    syscall[SYSCALL_LOCK_ACQ]      = (long (*)())do_mutex_lock_acquire;
    syscall[SYSCALL_LOCK_RELEASE]  = (long (*)())do_mutex_lock_release;
    syscall[SYSCALL_EXIT]          = (long (*)())do_exit;
    syscall[SYSCALL_THREAD_CREATE] = (long (*)())do_thread_create;
    syscall[SYSCALL_THREAD_YIELD]  = (long (*)())do_thread_yield;
}
/************************************************************/

int main(void)
{
    // Init jump table provided by kernel and bios(ΦωΦ)
    init_jmptab();

    // Init task information (〃'▽'〃)
    init_task_info();

    // Init Process Control Blocks |•'-'•) ✧
    init_pcb();
    printk("> [INIT] PCB initialization succeeded.\n");

    // Read CPU frequency (｡•ᴗ-)_
    time_base = bios_read_fdt(TIMEBASE);

    // Init lock mechanism o(´^｀)o
    init_locks();
    printk("> [INIT] Lock mechanism initialization succeeded.\n");

    // Init interrupt (^_^)
    init_exception();
    printk("> [INIT] Interrupt processing initialization succeeded.\n");

    // Init system call table (0_0)
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n");

    // Init screen (QAQ)
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n");

    // TODO: [p2-task4] Setup timer interrupt and enable all interrupt globally
    // NOTE: The function of sstatus.sie is different from sie's
    bios_set_timer((uint64_t)(get_ticks() + TIMER_INTERVAL));

    // TODO: Load tasks by either task id [p1-task3] or task name [p1-task4],
    //   and then execute them.


    // Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
    while (1)
    {
        // If you do non-preemptive scheduling, it's used to surrender control
        // do_scheduler();

        // If you do preemptive scheduling, they're used to enable CSR_SIE and wfi
        enable_preempt();
        // asm volatile("wfi");
    }

    return 0;
}
