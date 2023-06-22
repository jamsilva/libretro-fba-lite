#include "burnint.h"

bool bBurnDraw = true;
UINT8 *pBurnDrawBuffers[2];
INT32 nBurnDrawBuffersPos = 0;
UINT8 *pBurnPlayDrawOut;

bool bBurnSound = true;
INT16 *pBurnSoundBuffers[2];
INT32 nBurnSoundBuffersPos = 0;
INT16 *pBurnPlaySoundOut;

BurnWCLThread *pBurnDrvDrawThread = NULL;
BurnWCLThread *pBurnAudPlayThread = NULL;
BurnWCLThread *pBurnVidPlayThread = NULL;

void BurnSetPlaySoundOut(INT16 *pOut)
{
    if (pBurnAudPlayThread)
        BurnLockLock(pBurnAudPlayThread->Lock);
    pBurnPlaySoundOut = pOut;
    if (pBurnAudPlayThread)
        BurnLockUnlock(pBurnAudPlayThread->Lock);
}

void BurnSetPlayDrawOut(UINT8 *pOut)
{
    if (pBurnVidPlayThread)
        BurnLockLock(pBurnVidPlayThread->Lock);
    pBurnPlayDrawOut = pOut;
    if (pBurnVidPlayThread)
        BurnLockUnlock(pBurnVidPlayThread->Lock);
}

static void WCLThreadFunc(void *pArg)
{
    BurnWCLThread *pThread = (BurnWCLThread *)pArg;

    while (!pThread->bStop)
    {
        BurnLockLock(pThread->Lock);
        if (pThread->bWait)
            BurnCondWait(pThread->Cond, pThread->Lock);
        pThread->bWait = true;

        pThread->Func();

        BurnLockUnlock(pThread->Lock);
    }
}

INT32 BurnWCLThreadDoStep(BurnWCLThread *pThread)
{
    if (!pThread)
        return 1;

    BurnLockLock(pThread->Lock);
    pThread->bWait = false;
    BurnCondSignal(pThread->Cond);
    BurnLockUnlock(pThread->Lock);

    return 0;
}

INT32 BurnWCLThreadCreate(BurnWCLThread **pThread, void (*Func)())
{
    BurnWCLThread *Thread = (BurnWCLThread *)malloc(sizeof(BurnWCLThread));
    if (!Thread)
        goto FAILED;
    memset(Thread, 0, sizeof(*Thread));

    Thread->Func = Func;
    Thread->bStop = false;
    Thread->bWait = true;

    if (BurnLockCreate(&Thread->Lock) != 0)
        goto FAILED;

    if (BurnCondCreate(&Thread->Cond) != 0)
        goto FAILED;

    if (BurnThreadCreate(&Thread->Thd, WCLThreadFunc, Thread) != 0)
        goto FAILED;

    *pThread = Thread;
    return 0;

FAILED:
    if (Thread)
    {
        BurnThreadDestroy(Thread->Thd);
        BurnLockDestroy(Thread->Lock);
        BurnCondDestroy(Thread->Cond);
        free(Thread);
    }
    return 1;
}

INT32 BurnWCLThreadDestroy(BurnWCLThread *pThread)
{
    if (!pThread)
        return 1;

    BurnLockLock(pThread->Lock);
    pThread->bStop = true;
    pThread->bWait = false;
    BurnCondSignal(pThread->Cond);
    BurnLockUnlock(pThread->Lock);

    BurnThreadDestroy(pThread->Thd);
    BurnLockDestroy(pThread->Lock);
    BurnCondDestroy(pThread->Cond);

    free(pThread);

    return 0;
}
