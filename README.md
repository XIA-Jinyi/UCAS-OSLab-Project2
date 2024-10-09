# 中国科学院大学“操作系统研讨课”Project 2

## 进程调度

### PCB 初始化（Task 3 版本）

初始化 PCB 部分实现了两个关键函数。

```cpp
static void init_pcb(void) {
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
    current_running = &pid0_pcb;
}
```

`init_pcb` 函数用于加载 tasks 并初始化 PCB，其主要操作如下：

- 为进程分配内核栈空间（中断时保存现场）与用户栈空间（运行时进程使用）；
- 初始化部分字段（如 `pid`, `status`）；
- 调用 `init_pcb_stack` 初始化内核栈；
- 将进程加入就绪队列（队列操作参考 Linux 内核实现）；
- 将 `tp` 寄存器（`current_running`）设置为 `pid0_pcb`。

```cpp
static void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb)
{
    regs_context_t *pt_regs =
        (regs_context_t *)(kernel_stack - sizeof(regs_context_t));
    pt_regs->regs[OFFSET_REG_SP >> 3] = user_stack;
    pt_regs->regs[OFFSET_REG_TP >> 3] = (reg_t)pcb;
    pt_regs->sepc = entry_point;
    pt_regs->sstatus = SR_SPIE;

    switchto_context_t *pt_switchto =
        (switchto_context_t *)((ptr_t)pt_regs - sizeof(switchto_context_t));
    pcb->kernel_sp = (ptr_t)pt_switchto;
    pcb->user_sp = user_stack;
    pt_switchto->regs[SWITCH_TO_RA >> 3] = (reg_t)ret_from_exception;
    pt_switchto->regs[SWITCH_TO_SP >> 3] = (reg_t)pt_switchto;
}
```

`init_pcb_stack` 函数用于初始化进程的内核栈，其主要操作如下：

- 为系统中断和进程切换时需要保存的寄存器分配空间；
- 将 `switchto_context_t` 中 `ra` 寄存器设为 `ret_from_exception`，这样在第一次调度（进程启动）时能够执行 `sret` 指令，使进程切换至用户态执行；
- 将 `switchto_context_t` 中 `sp` 寄存器设为 `(ptr_t)pt_switchto`，方便从内核栈中恢复出寄存器的值；
- 将 `sepc` 寄存器的值设为 `entry_point`，在执行 `sret` 之后能够跳转至进程入口点执行；
- 设置 `sstatus` 寄存器的 `SR_SPIE` 标志位，保持当前的特权级别（Supervisor），这样在执行 `sret` 之后能使进程切换至用户态执行；
- 设置 `regs_context_t` 中的 `sp` 与 `tp` 寄存器，在返回用户态后确保这两个寄存器的值无误；
- 设置 PCB 的 `user_sp`，`kernel_sp` 字段。

### `do_scheduler`

调度函数 `do_scheduler` 实现如下。

```cpp
void do_scheduler(void) {
    check_sleeping();

    pcb_t *pre_running = current_running;
    if (list_empty(&ready_queue)) {
        return;
    }
    else if (pre_running->status == TASK_RUNNING) {
        pre_running->status = TASK_READY;
        list_add_tail(&pre_running->list, &ready_queue);
        current_running = list_entry(ready_queue.next, pcb_t, list);
    }
    else {
        current_running = list_entry(ready_queue.next, pcb_t, list);
    }
    current_running->status = TASK_RUNNING;
    list_del_init(&current_running->list);

    switch_to(pre_running, current_running);
}
```

- 首先检查 `sleep_queue` 中是否有进程满足唤醒条件，将唤醒的进程加入就绪队列中；
- 若当前进程正在运行，将其加入就绪队列，并从就绪队列中取出下一个要执行的进程，使用 `switch_to` 进行切换；
- 若当前进程未在运行，直接从就绪队列中取出下一个进程并切换。

## 互斥锁

### 锁的初始化

全局初始化函数 `init_locks` 如下。

```cpp
void init_locks(void) {
    for (int i = 0; i < LOCK_NUM; i++) {
        mlocks[i].lock.status = UNLOCKED;
        mlocks[i].key = -1;
        init_list_head(&mlocks[i].block_queue);
    }
}
```

锁的初始化函数 `do_mutex_lock_init` 如下。如果 `key` 已存在，返回锁的索引，否则初始化新锁并返回。

```cpp
int do_mutex_lock_init(int key) {
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
```

### 获取锁

```cpp
void do_mutex_lock_acquire(int mlock_idx) {
    if (mlocks[mlock_idx].key == -1) {
        return;
    }
    lock_status_t status = spin_lock_try_acquire(&mlocks[mlock_idx].lock);
    if (status == UNLOCKED) {
        return;
    }
    do_block(&current_running->list, &mlocks[mlock_idx].block_queue);
    do_scheduler();
}
```

- 使用 `spin_lock_try_acquire` 函数获取当前锁的状态（`spin_lock_try_acquire` 内部使用原子操作 `atomic_swap` 获取并设置锁的状态）；
- 若锁未被占用，则直接返回（此时 `spin_lock_try_acquire` 已将锁的状态设置为占用）；
- 若锁已被占用，将当前进程加入锁的阻塞队列，由于当前进程无法继续执行，需要使用 `do_scheduler` 进行调度。

### 释放锁

```cpp
void do_mutex_lock_release(int mlock_idx) {
    if (mlocks[mlock_idx].key == -1) {
        return;
    }
    if (list_empty(&mlocks[mlock_idx].block_queue)) {
        spin_lock_release(&mlocks[mlock_idx].lock);
    }
    else {
        do_unblock(mlocks[mlock_idx].block_queue.next);
    }
}
```

- 若锁的阻塞队列为空，直接使用 `spin_lock_release` 释放锁；
- 若锁的阻塞队列不为空，取出一个进程加入就绪队列，保持锁的占用状态。

## 系统调用

系统调用的完整流程如下。

1. 用户进程发起系统调用；

    ```cpp
    static long invoke_syscall(long sysno, long arg0, long arg1, long arg2, long arg3, long arg4)
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
    ```

2. 跳转至例外处理函数入口 `exception_handler_entry`（内核初始化时通过 `setup_exception` 设置），使用 `SAVE_CONTEXT` 保存 `regs_context_t`，调用 `interrupt_helper` 后通过 `ret_from_exception` 返回用户态；

    ```S
    ENTRY(exception_handler_entry)
        SAVE_CONTEXT

        la ra, ret_from_exception
        mv a0, sp
        csrr a1, CSR_STVAL
        csrr a2, CSR_SCAUSE

        j interrupt_helper
    ENDPROC(exception_handler_entry)
    ```

3. `interrupt_helper` 判断是中断还是异常，并通过跳转表执行相应的处理函数；

    ```cpp
    void interrupt_helper(regs_context_t *regs, uint64_t stval, uint64_t scause)
    {
        if (scause & SCAUSE_IRQ_FLAG)
            irq_table[scause & ~SCAUSE_IRQ_FLAG](regs, stval, scause);
        else
            exc_table[scause](regs, stval, scause);
    }
    ```

4. 如果为系统调用，`handle_syscall` 将通过跳转表执行相应的系统调用，完成后将保存的 `sepc` 值加 4，这样返回时将执行系统调用后的语句；

    ```cpp
    void handle_syscall(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
    {
        regs->regs[OFFSET_REG_A0 >> 3] = syscall[regs->regs[OFFSET_REG_A7 >> 3]](
            regs->regs[OFFSET_REG_A0 >> 3],
            regs->regs[OFFSET_REG_A1 >> 3],
            regs->regs[OFFSET_REG_A2 >> 3],
            regs->regs[OFFSET_REG_A3 >> 3],
            regs->regs[OFFSET_REG_A4 >> 3]);
        regs->regs[OFFSET_REG_SEPC >> 3] = regs->regs[OFFSET_REG_SEPC >> 3] + 4;
    }
    ```

5. `ret_from_exception` 恢复现场并返回至用户态。

    ```S
    ENTRY(ret_from_exception)
        RESTORE_CONTEXT
        sret
    ENDPROC(ret_from_exception)
    ```

## 时钟中断

时钟中断触发的完整流程如下。

1. 初始化内核时设置下一次时钟中断触发时间，并使用 `enable_preempt` 开启抢占式调度；

    ```cpp
    bios_set_timer((uint64_t)(get_ticks() + TIMER_INTERVAL));
    ```

2. 时钟中断触发时跳转至 `exception_handler_entry`，调用 `interrupt_helper`；
3. 若当前中断为时钟中断，调用 `handle_irq_timer`，设置下一次时钟中断时间并调度进程；

    ```cpp
    void handle_irq_timer(regs_context_t *regs, uint64_t stval, uint64_t scause)
    {
        bios_set_timer((uint64_t)(get_ticks() + TIMER_INTERVAL));
        do_scheduler();
    }
    ```

4. `ret_from_exception` 恢复现场并返回至用户态。

## 线程

### TCB

由于本实验 PCB 设计较为简洁，且满足 TCB 的基本需求，故没有单独设计 TCB，而是将现有 PCB 的结构体加以改造。

```cpp
typedef struct pcb {
    reg_t kernel_sp;
    reg_t user_sp;

    list_node_t list;
    pid_t pid;
    task_status_t status;

    int cursor_x;
    int cursor_y;

    uint64_t wakeup_time;

    list_node_t thread_group;   // 线程组
} pcb_t, tcb_t;
```

在原有 PCB 结构体的最后添加了链表节点 `thread_group`，用于表示线程组。同一线程组的线程同属于一个进程。这种设计支持当前进程的任意一个线程创建新线程。

### `thread_create`

创建线程与进程初始化类似。不同的是新的 TCB 存储在了动态分配的空间，且需要将新线程加入 `current_running` 所在的线程组，还需要完成运行参数的传递。

```cpp
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
```

```cpp
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
```

用户程序的调用接口设计如下。其中，第一个参数为新线程执行的函数，后四个参数为函数调用的参数（最多支持 4 个参数，每个参数均为 `long` 类型，用户可以考虑使用类似 `func(int argc, char **argv)` 的方式传入更多参数或其他类型的参数）。

```cpp
int sys_thread_create(void (*func)(void), long arg0, long arg1, long arg2, long arg3)
{
    return invoke_syscall(SYSCALL_THREAD_CREATE, (long)func, arg0, arg1, arg2, arg3);
}
```

### `thread_yield`

直接找到线程组中的下一个线程进行切换。这种设计保证了线程之间的平等性，也不会影响系统进行抢占式的调度。

```cpp
void do_thread_yield() {
    tcb_t *pre_running = current_running;
    current_running = list_entry(pre_running->thread_group.next, tcb_t, thread_group);
    pre_running->status = TASK_READY;
    current_running->status = TASK_RUNNING;
    init_list_head(&current_running->list);
    switch_to(pre_running, current_running);
}
```
