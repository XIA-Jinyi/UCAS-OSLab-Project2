#include <os/task.h>
#include <os/string.h>
#include <os/kernel.h>
#include <type.h>

uint64_t load_task_img(char *taskname)
{
    /**
     * TODO:
     * 1. [p1-task3] load task from image via task id, and return its entrypoint
     * 2. [p1-task4] load task via task name, thus the arg should be 'char *taskname'
     */
    int task_num = *(int *)0x502001f8;
    void *buffer = (void *)(0x52000000ul + ((task_num + 1) << 16));
    size_t entrypoint = 0, nbytes_size = 0;
    int begin_block = 0, end_block = 0;
    for (int i = 0; i < task_num; i++) {
        if (strcmp(tasks[i].name, taskname) == 0) {
            entrypoint = TASK_MEM_BASE + (i << 16);
            nbytes_size = tasks[i].end - tasks[i].begin;
            begin_block = NBYTES2SEC(tasks[i].begin) - 1;
            end_block = NBYTES2SEC(tasks[i].end - 1) - 1;
            bios_sd_read((uint64_t)buffer, end_block - begin_block + 1, begin_block);
            memcpy((void *)entrypoint, buffer + tasks[i].begin % SECTOR_SIZE, nbytes_size);
            break;
        }
    }
    return entrypoint;
}