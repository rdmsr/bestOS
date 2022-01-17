#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#define SYSCALL_LOG 0
#define SYSCALL_MMAP 1
#define SYSCALL_SET_FS_BASE 2
#define SYSCALL_OPENAT 3
#define SYSCALL_READ 4
#define SYSCALL_WRITE 5
#define SYSCALL_CLOSE 6
#define SYSCALL_SEEK 7
#define SYSCALL_EXIT 8

#include <arch/arch.h>

int64_t syscall_dispatch(int num, Stack *args);

#endif