#include <os/list.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/mm.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>

pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack = INIT_KERNEL_STACK + PAGE_SIZE;
pcb_t pid0_pcb = {
    .pid = 0,
    .kernel_sp = (ptr_t)pid0_stack,
    .user_sp = (ptr_t)pid0_stack
};

LIST_HEAD(ready_queue);
LIST_HEAD(sleep_queue);

/* global process id */
pid_t process_id = 1;

void do_scheduler(void)
{
    // [p2-task3] Check sleep queue to wake up PCBs
    check_sleeping();

    /************************************************************/
    /* Do not touch this comment. Reserved for future projects. */
    /************************************************************/

    // TODO: [p2-task1] Modify the current_running pointer.
    pcb_t *pre_running = current_running;
    if (list_empty(&ready_queue))
    {
        return;
    }
    else if (pre_running->status == TASK_RUNNING)
    {
        pre_running->status = TASK_READY;
        list_add_tail(&pre_running->list, &ready_queue);
        current_running = list_entry(ready_queue.next, pcb_t, list);
    }
    else
    {
        current_running = list_entry(ready_queue.next, pcb_t, list);
    }
    current_running->status = TASK_RUNNING;
    list_del_init(&current_running->list);

    // TODO: [p2-task1] switch_to current_running
    switch_to(pre_running, current_running);
}

void do_sleep(uint32_t sleep_time)
{
    // [p2-task3] sleep(seconds)
    // NOTE: you can assume: 1 second = 1 `timebase` ticks
    // 1. block the current_running
    // 2. set the wake up time for the blocked task
    // 3. reschedule because the current_running is blocked.
    do_block(&current_running->list, &sleep_queue);
    current_running->wakeup_time = get_timer() + sleep_time;
    do_scheduler();
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
    // TODO: [p2-task2] block the pcb task into the block queue
    pcb_t *block_pcb = list_entry(pcb_node, pcb_t, list);
    block_pcb->status = TASK_BLOCKED;
    list_move_tail(pcb_node, queue);
}

void do_unblock(list_node_t *pcb_node)
{
    // TODO: [p2-task2] unblock the `pcb` from the block queue
    pcb_t *unblock_pcb = list_entry(pcb_node, pcb_t, list);
    unblock_pcb->status = TASK_READY;
    list_move_tail(pcb_node, &ready_queue);
}

void do_exit()
{
    current_running->status = TASK_EXITED;
    do_scheduler();
}