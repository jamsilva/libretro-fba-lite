#include "burner.h"

static bool bAVPlayOkey = false;

static void AudBufferInit()
{
    nAudSegLen = (nBurnSoundRate * 100 + (nBurnFPS >> 1)) / nBurnFPS;
    nBurnSoundLen = nAudSegLen;

    size_t nSize = (nAudSegLen << 2) * sizeof(INT16);
    for (INT32 i = 0; i < BURN_MAX_AUD_BUFFER; i++)
    {
        pBurnSoundBuffers[i] = (INT16 *)malloc(nSize);
        if (pBurnSoundBuffers[i])
            memset(pBurnSoundBuffers[i], 0, nSize);
    }
    nBurnSoundBuffersPos = 0;
    pBurnSoundOut = pBurnSoundBuffers[nBurnSoundBuffersPos];
    pBurnPlaySoundOut = pBurnSoundOut;
}

static void AudBufferDeinit()
{
    for (INT32 i = 0; i < BURN_MAX_AUD_BUFFER; i++)
    {
        if (pBurnSoundBuffers[i])
            free(pBurnSoundBuffers[i]);
        pBurnSoundBuffers[i] = NULL;
    }
    nBurnSoundBuffersPos = 0;
    pBurnSoundOut = NULL;
    pBurnPlaySoundOut = NULL;
}

static void VidBufferInit()
{
    size_t nSize = nBurnGameWidth * nBurnGameHeight * nBurnBpp;
    for (INT32 i = 0; i < BURN_MAX_VID_BUFFER; i++)
    {
        pBurnDrawBuffers[i] = (UINT8 *)malloc(nSize);
        if (pBurnDrawBuffers[i])
            memset(pBurnDrawBuffers[i], 0, nSize);
    }
    nBurnDrawBuffersPos = 0;
    pBurnDraw = pBurnDrawBuffers[nBurnDrawBuffersPos];
    pBurnPlayDrawOut = pBurnDraw;
}

static void VidBufferDeinit()
{
    for (INT32 i = 0; i < BURN_MAX_VID_BUFFER; i++)
    {
        if (pBurnDrawBuffers[i])
            free(pBurnDrawBuffers[i]);
        pBurnDrawBuffers[i] = NULL;
    }
    nBurnDrawBuffersPos = 0;
    pBurnDraw = NULL;
    pBurnPlayDrawOut = NULL;
}

static void AudPlayFrame()
{
    if (bBurnSound)
        audio_batch_cb(pBurnPlaySoundOut, nBurnSoundLen);
}

static void VidPlayFrame()
{
    video_cb(pBurnPlayDrawOut, nBurnGameWidth, nBurnGameHeight, nBurnPitch);
}

void BurnAVPlayFrame()
{
    if (pBurnAudPlayThread)
        BurnWCLThreadDoStep(pBurnAudPlayThread);
    else
        AudPlayFrame();

    if (pBurnVidPlayThread)
        BurnWCLThreadDoStep(pBurnVidPlayThread);
    else
        VidPlayFrame();
}

void BurnAVPlayInit()
{
    if (bAVPlayOkey)
        BurnAVPlayExit();

    AudBufferInit();
    VidBufferInit();

    if (pBurnDrvDrawThread)
    {
        BurnWCLThreadCreate(&pBurnAudPlayThread, AudPlayFrame);
        BurnWCLThreadCreate(&pBurnVidPlayThread, VidPlayFrame);
    }

    bAVPlayOkey = true;
}

void BurnAVPlayExit()
{
    if (pBurnAudPlayThread)
        BurnWCLThreadDestroy(pBurnAudPlayThread);
    pBurnAudPlayThread = NULL;

    if (pBurnVidPlayThread)
        BurnWCLThreadDestroy(pBurnVidPlayThread);
    pBurnVidPlayThread = NULL;

    AudBufferDeinit();
    VidBufferDeinit();

    bAVPlayOkey = false;
}
