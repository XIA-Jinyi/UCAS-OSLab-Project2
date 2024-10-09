#include <stdio.h>
#include <unistd.h>

#define MAIN_LOC (2)
#define IGNORE (0)
int print_loc[] = {7, 8, 6};
int cnt[] = {1, 1, 0};
int running = 0;

void count(int id)
{
    while (1)
    {
        sys_move_cursor(0, print_loc[id]);
        printf("\t> I am Thread with ID %d, cnt: %d.", id, cnt[id]++);
        if (cnt[id] - cnt[!id] >= 100) {
            running = !id;
            cnt[MAIN_LOC]++;
            sys_thread_yield();
        }
    }
}

int main(void)
{
    sys_thread_create((void (*)())count, 0, IGNORE, IGNORE, IGNORE);
    sys_thread_create((void (*)())count, 1, IGNORE, IGNORE, IGNORE);
    while(1)
    {
        sys_move_cursor(0, print_loc[MAIN_LOC]);
        printf("> [TASK] Test thread: yield %d times.", cnt[MAIN_LOC]);
        // cnt[MAIN_LOC]++;
        sys_thread_yield();
    }
}