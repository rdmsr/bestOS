#ifndef LIB_LOCK_H
#define LIB_LOCK_H
#include <lib/base.h>

static inline void spin_lock(void *lock)
{
    while (__atomic_test_and_set(lock, __ATOMIC_ACQUIRE))
        ;
}

static inline void spin_release(void *lock)
{
    __atomic_clear(lock, __ATOMIC_RELEASE);
}

#endif
