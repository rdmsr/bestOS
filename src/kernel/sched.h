#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H
#include <arch/arch.h>
#include <fs/vfs.h>

#define SWITCH_TICK 10

typedef struct
{
    int fd;
    VfsNode *file;
} FileDescriptor;

typedef struct task
{
    struct task *parent;

    int current_fd;

    vec_t(FileDescriptor) descriptors;

    VfsNode *cwd;

    int pid;

    uintptr_t *sp;
    uintptr_t *pagemap;
    size_t used_pages;

    Stack stack;

    int state; // 0 = alive, 1 = waiting for death, -1 = dead
    int exit_code;

    void *fpu_storage;

} Task;

Task *task_init(uintptr_t entry_point);

void sched_push(Task *t);
void sched_init(void);
Task *sched_tick(void);

bool sched_started();
void sched_start();

void sched_remove(Task *t);

void sched_new_elf_process(char *path, const char **argv, const char **envp, char *stdin, char *stdout, char *stderr);

Task *sched_current_task();

void sched_set_tick(int t);
void sched_set_index(int i);
int sched_get_last_pid();

#endif