#include <rthreads/rthreads.h>

#include "burner.h"

INT32 BurnLockCreate(TLOCK *pLock)
{
    *pLock = slock_new();
    if (!pLock)
        return 1;
    return 0;
}

INT32 BurnLockDestroy(TLOCK Lock)
{
    slock_free(Lock);
    return 0;
}

INT32 BurnLockLock(TLOCK Lock)
{
    slock_lock(Lock);
    return 0;
}

INT32 BurnLockUnlock(TLOCK Lock)
{
    slock_unlock(Lock);
    return 0;
}

INT32 BurnCondCreate(TCOND *pCond)
{
    *pCond = scond_new();
    if (!*pCond)
        return 1;
    return 0;
}

INT32 BurnCondDestroy(TCOND Cond)
{
    scond_free(Cond);
    return 0;
}

INT32 BurnCondWait(TCOND Cond, TLOCK Lock)
{
    scond_wait(Cond, Lock);
    return 0;
}

INT32 BurnCondSignal(TCOND Cond)
{
    scond_signal(Cond);
    return 0;
}

INT32 BurnThreadCreate(TTHREAD *pThread, void (*Func)(void *), void *pArg)
{
    *pThread = sthread_create(Func, pArg);
    if (!*pThread)
        return 1;
    return 0;
}

INT32 BurnThreadDestroy(TTHREAD Thread)
{
    sthread_join(Thread);
    return 0;
}
