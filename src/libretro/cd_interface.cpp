// CD/CD-ROM support
#include "burner.h"

extern struct CDEmuDo cdimgDo;

static struct CDEmuDo *pCDEmuDo[] = {
    &cdimgDo,
};
#define CDEMU_LEN (sizeof(pCDEmuDo) / sizeof(pCDEmuDo[0]))

bool bCDEmuOkay = false;
UINT32 nCDEmuSelect = 0;
CDEmuStatusValue CDEmuStatus;

TCHAR CDEmuImage[MAX_PATH] = _T("");

INT32 CDEmuExit()
{
    if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN)
    {
        return 1;
    }
    bCDEmuOkay = false;

    return pCDEmuDo[nCDEmuSelect]->CDEmuExit();
}

INT32 CDEmuInit()
{
    INT32 nRet;

    if (nCDEmuSelect >= CDEMU_LEN)
    {
        return 1;
    }

    CDEmuStatus = idle;

    if ((nRet = pCDEmuDo[nCDEmuSelect]->CDEmuInit()) == 0)
    {
        bCDEmuOkay = true;
    }

    return nRet;
}

INT32 CDEmuStop()
{
    if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN)
    {
        return 1;
    }

    return pCDEmuDo[nCDEmuSelect]->CDEmuStop();
}

INT32 CDEmuPlay(UINT8 M, UINT8 S, UINT8 F)
{
    if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN)
    {
        return 1;
    }

    return pCDEmuDo[nCDEmuSelect]->CDEmuPlay(M, S, F);
}

INT32 CDEmuLoadSector(INT32 LBA, char *pBuffer)
{
    if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN)
    {
        return 0;
    }

    return pCDEmuDo[nCDEmuSelect]->CDEmuLoadSector(LBA, pBuffer);
}

UINT8 *CDEmuReadTOC(INT32 track)
{
    if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN)
    {
        return NULL;
    }

    return pCDEmuDo[nCDEmuSelect]->CDEmuReadTOC(track);
}

UINT8 *CDEmuReadQChannel()
{
    if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN)
    {
        return NULL;
    }

    return pCDEmuDo[nCDEmuSelect]->CDEmuReadQChannel();
}

INT32 CDEmuGetSoundBuffer(INT16 *buffer, INT32 samples)
{
    if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN)
    {
        return 1;
    }

    return pCDEmuDo[nCDEmuSelect]->CDEmuGetSoundBuffer(buffer, samples);
}

INT32 CDEmuScan(INT32 nAction, INT32 *pnMin)
{
    if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN)
    {
        return 1;
    }

    return pCDEmuDo[nCDEmuSelect]->CDEmuScan(nAction, pnMin);
}

CDEmuStatusValue CDEmuGetStatus()
{
	return CDEmuStatus;
}

void CDEmuStartRead()
{
	CDEmuStatus = seeking;
}

void CDEmuPause()
{
	CDEmuStatus = paused;
}

void CDEmuResume()
{
	CDEmuStatus = playing;
}
