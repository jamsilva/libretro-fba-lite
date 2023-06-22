#include "libretro.h"
#include "burner.h"

// Savestates support
static UINT8 *pStateBuffer;
static UINT32 nStateLen;
static UINT32 nStateTmpLen;
static UINT32 nCurDrvActive = ~0U;

static int StateWriteAcb(BurnArea *pba)
{
    nStateTmpLen += pba->nLen;
    if (nStateTmpLen > nStateLen)
        return 1;
    memcpy(pStateBuffer, pba->Data, pba->nLen);
    pStateBuffer += pba->nLen;

    return 0;
}

static int StateReadAcb(BurnArea *pba)
{
    nStateTmpLen += pba->nLen;
    if (nStateTmpLen > nStateLen)
        return 1;
    memcpy(pba->Data, pStateBuffer, pba->nLen);
    pStateBuffer += pba->nLen;

    return 0;
}

static int StateLenAcb(BurnArea *pba)
{
    nStateLen += pba->nLen;

    return 0;
}

static INT32 LibretroAreaScan(INT32 nAction)
{
    nStateTmpLen = 0;

    // The following value is sometimes used in game logic (xmen6p, ...),
    // and will lead to various issues if not handled properly.
    // On standalone, this value is stored in savestate files headers
    // (and has special logic in runahead feature ?).
    // Due to core's logic, this value is increased at each frame iteration,
    // including multiple iterations of the same frame through runahead,
    // but it needs to stay synced between multiple iterations of a given frame
    SCAN_VAR(nCurrentFrame);

    BurnAreaScan(nAction, 0);

    return 0;
}

static void TweakScanFlags()
{
    // With RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE, we can't guess accurately, so let's use the safest tweaks (netplay)
    int nAudioVideoEnable = -1;
    environ_cb(RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE, &nAudioVideoEnable);
    kNetGame = nAudioVideoEnable & 4 ? 1 : 0;
    if (kNetGame == 1)
    {
        EnableHiscores = false;
    }
}

size_t retro_serialize_size()
{
    if (nBurnDrvActive == ~0U)
        return 0;

    nCurDrvActive = nBurnDrvActive;

    INT32 nAction = ACB_FULLSCAN | ACB_READ;

    TweakScanFlags();

    nStateLen = 0;
    BurnAcb = StateLenAcb;
    LibretroAreaScan(nAction);

    return nStateLen;
}

bool retro_serialize(void *data, size_t size)
{
    if (nBurnDrvActive == ~0U)
        return false;

    INT32 nAction = ACB_FULLSCAN | ACB_READ;

    TweakScanFlags();

    BurnAcb = StateWriteAcb;
    pStateBuffer = (UINT8 *)data;

    LibretroAreaScan(nAction);

    // size is bigger than expected
    if (nStateTmpLen > size)
        return false;

    return true;
}

bool retro_unserialize(const void *data, size_t size)
{
    if (nBurnDrvActive == ~0U)
        return false;

    INT32 nAction = ACB_FULLSCAN | ACB_WRITE;

    TweakScanFlags();

    if (nBurnDrvActive != nCurDrvActive)
        nStateLen = retro_serialize_size();

    if (size != nStateLen)
        return false;

    BurnAcb = StateReadAcb;
    pStateBuffer = (UINT8 *)data;

    LibretroAreaScan(nAction);

    // size is bigger than expected
    if (nStateTmpLen > size)
        return false;

    // Some driver require to recalc palette after loading savestates
    BurnRecalcPal();

    return true;
}
