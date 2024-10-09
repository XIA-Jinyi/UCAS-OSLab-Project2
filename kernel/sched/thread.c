#include <os/list.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/mm.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>
#include <asm/regs.h>
#include <csr.h>

extern void ret_from_exception();

static void init_tcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    tcb_t *tcb, long a0, long a1, long a2, long a3)
{
    regs_context_t *pt_regs =
        (regs_context_t *)(kernel_stack - sizeof(regs_context_t));
    pt_regs->regs[OFFSET_REG_SP >> 3] = user_stack;
    pt_regs->regs[OFFSET_REG_TP >> 3] = (reg_t)tcb;
    pt_regs->regs[OFFSET_REG_A0 >> 3] = a0;
    pt_regs->regs[OFFSET_REG_A1 >> 3] = a1;
    pt_regs->regs[OFFSET_REG_A2 >> 3] = a2;
    pt_regs->regs[OFFSET_REG_A3 >> 3] = a3;
    pt_regs->sepc = entry_point;
    pt_regs->sstatus = SR_SPIE;

    switchto_context_t *pt_switchto =
        (switchto_context_t *)((ptr_t)pt_regs - sizeof(switchto_context_t));
    tcb->kernel_sp = (ptr_t)pt_switchto;
    tcb->user_sp = user_stack;
    pt_switchto->regs[SWITCH_TO_RA >> 3] = (reg_t)ret_from_exception;
    pt_switchto->regs[SWITCH_TO_SP >> 3] = (reg_t)pt_switchto;
}

void do_thread_create(void (*func)(void), long arg0, long arg1, long arg2, long arg3)
{
    ptr_t kernel_stack = allocKernelPage(1) + PAGE_SIZE;
    ptr_t user_stack = allocUserPage(16) + PAGE_SIZE * 16;
    kernel_stack -= sizeof(tcb_t);
    tcb_t *thread_tcb = (tcb_t *)kernel_stack;
    thread_tcb->pid = current_running->pid;
    thread_tcb->status = TASK_READY;
    init_list_head(&thread_tcb->list);
    init_list_head(&thread_tcb->thread_group);
    init_tcb_stack(kernel_stack, user_stack, (ptr_t)func, thread_tcb, arg0, arg1, arg2, arg3);
    list_add(&thread_tcb->thread_group, &current_running->thread_group);
}

void do_thread_yield()
{
    tcb_t *pre_running = current_running;
    current_running = list_entry(pre_running->thread_group.next, tcb_t, thread_group);
    pre_running->status = TASK_READY;
    current_running->status = TASK_RUNNING;
    init_list_head(&current_running->list);
    switch_to(pre_running, current_running);
}