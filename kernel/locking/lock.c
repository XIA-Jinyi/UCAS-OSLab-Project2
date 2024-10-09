#include <os/lock.h>
#include <os/sched.h>
#include <os/list.h>
#include <atomic.h>

mutex_lock_t mlocks[LOCK_NUM];

void init_locks(void)
{
    /* TODO: [p2-task2] initialize mlocks */
    for (int i = 0; i < LOCK_NUM; i++)
    {
        mlocks[i].lock.status = UNLOCKED;
        mlocks[i].key = -1;
        init_list_head(&mlocks[i].block_queue);
    }
}

void spin_lock_init(spin_lock_t *lock)
{
    /* TODO: [p2-task2] initialize spin lock */
    lock->status = UNLOCKED;
}

int spin_lock_try_acquire(spin_lock_t *lock)
{
    /* TODO: [p2-task2] try to acquire spin lock */
    return atomic_swap(LOCKED, (volatile ptr_t)&lock->status);
}

void spin_lock_acquire(spin_lock_t *lock)
{
    /* TODO: [p2-task2] acquire spin lock */
    while (spin_lock_try_acquire(lock) == LOCKED);
}

void spin_lock_release(spin_lock_t *lock)
{
    /* TODO: [p2-task2] release spin lock */
    atomic_swap(UNLOCKED, (volatile ptr_t)&lock->status);
}

int do_mutex_lock_init(int key)
{
    /* TODO: [p2-task2] initialize mutex lock */
    for (int i=0; i < LOCK_NUM; i++){
        if (mlocks[i].key == key){
            return i;
        }
    }
    for (int i= 0; i < LOCK_NUM; i++){
        if (mlocks[i].key == -1){
            mlocks[i].key = key;
            return i;
        }
    }
    return -1;
}

void do_mutex_lock_acquire(int mlock_idx)
{
    /* TODO: [p2-task2] acquire mutex lock */
    if (mlocks[mlock_idx].key == -1){
        return;
    }
    lock_status_t status = spin_lock_try_acquire(&mlocks[mlock_idx].lock);
    if (status == UNLOCKED){
        return;
    }
    do_block(&current_running->list, &mlocks[mlock_idx].block_queue);
    do_scheduler();
}

void do_mutex_lock_release(int mlock_idx)
{
    /* TODO: [p2-task2] release mutex lock */
    if (mlocks[mlock_idx].key == -1){
        return;
    }
    if (list_empty(&mlocks[mlock_idx].block_queue)){
        spin_lock_release(&mlocks[mlock_idx].lock);
    }
    else {
        do_unblock(mlocks[mlock_idx].block_queue.next);
    }
}
