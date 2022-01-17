#include "arch/memory/vmm.h"
#include "fs/vfs.h"
#include "lib/lock.h"
#include "lib/vec.h"
#include <arch/memory.h>
#include <lib/print.h>
#include <sched.h>
#include <syscalls.h>

#define AT_FDCWD -100

typedef int64_t Syscall(Stack *params);

uint32_t lock;

int64_t sys_log(Stack *stack)
{
    spin_lock(&lock);

    log((char *)stack->rdi);

    spin_release(&lock);

    return 0;
}

int64_t sys_mmap(Stack *stack)
{
    size_t ret = 0;

    vmm_mmap(sched_current_task()->pagemap, stack->rdi, stack->rsi, stack->r10, &sched_current_task()->used_pages, &ret);

    stack->rax = ret;
    return ret;
}

int64_t sys_set_fs(Stack *stack)
{
    asm_write_msr(0xc0000100, (uintptr_t)stack->rdi);

    return 0;
}

FileDescriptor get_fd(int fd)
{
    if (fd - 1 > sched_current_task()->descriptors.length)
    {
        log("INVALID FD: %d", fd);
    }

    return sched_current_task()->descriptors.data[fd];
}

int64_t sys_openat(Stack *stack)
{
    int dirfd = (int)stack->rdi;
    const char *path = (const char *)stack->rsi;
    mode_t mode = (mode_t)stack->r10;

    VfsNode *parent = NULL;

    if (dirfd == AT_FDCWD)
    {
        parent = vfs_get_parent(sched_current_task()->cwd, (char *)path);
    }

    else
    {
        parent = vfs_get_parent(get_fd(dirfd).file, (char *)path);
    }

    if (!parent)
    {
        set_errno(ENOENT);
        stack->rax = (uint64_t)-1;
        return -1;
    }

    FileDescriptor new_fd = {.file = vfs_open(parent, (char *)path, mode), .fd = sched_current_task()->current_fd++};

    vec_push(&sched_current_task()->descriptors, new_fd);

    if (nerd_logs())
    {
        log("opened file \"%s\" in dir \"%s\" with fd %d", path, parent->name, new_fd.fd);
    }

    stack->rax = new_fd.fd;

    return new_fd.fd;
}

int64_t sys_read(Stack *stack)
{

    int fd = (int)stack->rdi;
    void *buf = (void *)stack->rsi;
    size_t count = (size_t)stack->rdx;

    ssize_t ret = vfs_read(get_fd(fd).file, get_fd(fd).file->cursor, count, buf);
    get_fd(fd).file->cursor += count;

    stack->rax = (uint64_t)ret;

    return ret;
}

int64_t sys_write(Stack *stack)
{
    int fd = (int)stack->rdi;
    void *buf = (void *)stack->rsi;
    size_t count = (size_t)stack->rdx;

    ssize_t ret = vfs_write(get_fd(fd).file, count, buf);

    stack->rax = (uint64_t)ret;
    return ret;
}

int64_t sys_close(Stack *stack)
{
    int fildes = (int)stack->rdi;

    vec_splice(&sched_current_task()->descriptors, fildes, 1);

    VfsNode *file = get_fd(fildes).file;

    file->init = false;
    file->cursor = 0;

    free((void *)file->address);

    sched_current_task()->current_fd--;

    if (nerd_logs())
    {
        log("closed fd %d", fildes);
    }
    stack->rax = 0;
    return 0;
}

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 3

int64_t sys_seek(Stack *stack)
{
    int fd = (int)stack->rdi;
    off_t offset = (off_t)stack->rsi;
    int whence = (int)stack->rdx;

    VfsNode *file = get_fd(fd).file;
    off_t base = 0;
    switch (whence)
    {
    case SEEK_SET:
        base = offset;
        break;
    case SEEK_CUR:
        base = file->cursor + offset;
        break;
    case SEEK_END:
        base = file->stat.st_size + offset;
        break;
    default:
        set_errno(EINVAL);
        stack->rax = (uint64_t)-1;
        return -1;
    }

    file->cursor = base;
    stack->rax = (uint64_t)base;
    return base;
}

int64_t sys_exit(Stack *stack)
{
    int status = (int)stack->rdi;

    sched_remove(sched_current_task());

    sched_current_task()->state = -1;
    sched_current_task()->exit_code = status;

    pmm_free(sched_current_task()->sp, STACK_SIZE / PAGE_SIZE);
    pmm_free((void *)((uintptr_t)sched_current_task()->pagemap - MMAP_KERNEL_BASE), 1);
    vec_deinit(&sched_current_task()->descriptors);

    for (;;)
        ;
}

int64_t sys_execve(Stack *stack)
{
    const char *path = (const char *)stack->rdi;
    const char **argv = (const char **)stack->rsi;
    const char **envp = (const char **)stack->rdx;

    sched_new_elf_process((char *)path, argv, envp, NULL, NULL, NULL);
    sched_set_index(sched_get_last_pid() - 2);
    sched_set_tick(SWITCH_TICK - 1);

    stack->rax = 0;
    return 0;
}

Syscall *syscalls[] = {
    [SYSCALL_LOG] = sys_log,
    [SYSCALL_MMAP] = sys_mmap,
    [SYSCALL_SET_FS_BASE] = sys_set_fs,
    [SYSCALL_OPENAT] = sys_openat,
    [SYSCALL_READ] = sys_read,
    [SYSCALL_WRITE] = sys_write,
    [SYSCALL_CLOSE] = sys_close,
    [SYSCALL_SEEK] = sys_seek,
    [SYSCALL_EXIT] = sys_exit,
    [SYSCALL_EXECVE] = sys_execve,
};

int64_t syscall_dispatch(int num, Stack *args)
{

    syscalls[num](args);

    if (get_errno() > 0)
    {
        args->rdx = get_errno();

        args->rax = (uint64_t)-1;

        set_errno(0);

        return args->rdx;
    }

    return 0;
}