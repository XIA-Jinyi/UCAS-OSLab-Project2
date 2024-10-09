#include <syscall.h>
#include <stdint.h>
#include <kernel.h>
#include <unistd.h>

static const long IGNORE = 0L;

static long invoke_syscall(long sysno, long arg0, long arg1, long arg2,
                           long arg3, long arg4)
{
    long retval;
    asm volatile(
        "mv a7, %1\n\t"
        "mv a0, %2\n\t"
        "mv a1, %3\n\t"
        "mv a2, %4\n\t"
        "mv a3, %5\n\t"
        "mv a4, %6\n\t"
        "ecall\n\t"
        "mv %0, a0"
        : "=r"(retval)
        : "r"(sysno), "r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4)
    );
    return retval;
}

void sys_yield(void)
{
    /* TODO: [p2-task1] call call_jmptab to implement sys_yield */
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
    /* [p2-task3] call invoke_syscall to implement sys_yield */
}

void sys_move_cursor(int x, int y)
{
    /* TODO: [p2-task1] call call_jmptab to implement sys_move_cursor */
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE, IGNORE, IGNORE);
    /* [p2-task3] call invoke_syscall to implement sys_move_cursor */
}

void sys_write(char *buff)
{
    /* TODO: [p2-task1] call call_jmptab to implement sys_write */
    invoke_syscall(SYSCALL_WRITE, (long)buff, IGNORE, IGNORE, IGNORE, IGNORE);
    /* [p2-task3] call invoke_syscall to implement sys_write */
}

void sys_reflush(void)
{
    /* TODO: [p2-task1] call call_jmptab to implement sys_reflush */
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
    /* [p2-task3] call invoke_syscall to implement sys_reflush */
}

int sys_mutex_init(int key)
{
    /* TODO: [p2-task2] call call_jmptab to implement sys_mutex_init */
    return invoke_syscall(SYSCALL_LOCK_INIT, key, IGNORE, IGNORE, IGNORE, IGNORE);
    /* [p2-task3] call invoke_syscall to implement sys_mutex_init */
}

void sys_mutex_acquire(int mutex_idx)
{
    /* TODO: [p2-task2] call call_jmptab to implement sys_mutex_acquire */
    invoke_syscall(SYSCALL_LOCK_ACQ, mutex_idx, IGNORE, IGNORE, IGNORE, IGNORE);
    /* [p2-task3] call invoke_syscall to implement sys_mutex_acquire */
}

void sys_mutex_release(int mutex_idx)
{
    /* TODO: [p2-task2] call call_jmptab to implement sys_mutex_release */
    invoke_syscall(SYSCALL_LOCK_RELEASE, mutex_idx, IGNORE, IGNORE, IGNORE, IGNORE);
    /* [p2-task3] call invoke_syscall to implement sys_mutex_release */
}

long sys_get_timebase(void)
{
    /* [p2-task3] call invoke_syscall to implement sys_get_timebase */
    return invoke_syscall(SYSCALL_GET_TIMEBASE, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
}

long sys_get_tick(void)
{
    /* [p2-task3] call invoke_syscall to implement sys_get_tick */
    return invoke_syscall(SYSCALL_GET_TICK, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_sleep(uint32_t time)
{
    /* [p2-task3] call invoke_syscall to implement sys_sleep */
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE, IGNORE, IGNORE);
}

int sys_thread_create(void (*func)(void), long arg0, long arg1, long arg2, long arg3)
{
    return invoke_syscall(SYSCALL_THREAD_CREATE, (long)func, arg0, arg1, arg2, arg3);
}

void sys_thread_yield(void)
{
    invoke_syscall(SYSCALL_THREAD_YIELD, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
}

/************************************************************/
/* Do not touch this comment. Reserved for future projects. */
/************************************************************/