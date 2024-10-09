#include <os/list.h>
#include <os/sched.h>
#include <type.h>

uint64_t time_elapsed = 0;
uint64_t time_base = 0;

uint64_t get_ticks()
{
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

uint64_t get_timer()
{
    return get_ticks() / time_base;
}

uint64_t get_time_base()
{
    return time_base;
}

void latency(uint64_t time)
{
    uint64_t begin_time = get_timer();

    while (get_timer() - begin_time < time);
    return;
}

void check_sleeping(void)
{
    // [p2-task3] Pick out tasks that should wake up from the sleep queue
    list_head *pos, *n;
    list_for_each_safe(pos, n, &sleep_queue)
    {
        pcb_t *pcb = list_entry(pos, pcb_t, list);
        if (get_timer() >= pcb->wakeup_time)
        {
            do_unblock(pos);
        }
    }
}