#include <string>
#include <vector>

#include "burner.h"
#include "neogeo_bios.h"

#define STATUS_MISSING 0
#define STATUS_OK 1
#define STATUS_MISMATCH_CRC 2
#define STATUS_MISMATCH_SIZE 3

struct RomFind
{
    char *pszName;
    UINT32 nLen;
    UINT32 nCrc;
    UINT32 nType;
    UINT32 nState;
    INT32 nZip;
    INT32 nPos;
};

static std::vector<std::string> vecZipPathList;
static std::vector<struct RomFind> vecRomFindList;
static int nCurrentZip = -1;

static INT32 __cdecl BzipBurnLoadRom(UINT8 *Dest, INT32 *pnWrote, INT32 i)
{
    if (i < 0 || i >= vecRomFindList.size())
        return 1;

    log_cb(RETRO_LOG_INFO, "[LOAD ROM] %s...\n", vecRomFindList[i].pszName);

    INT32 nWantZip = vecRomFindList[i].nZip;
    if (nCurrentZip != nWantZip)
    {
        ZipClose();
        nCurrentZip = -1;
        if (ZipOpen((char *)vecZipPathList[nWantZip].c_str()) != 0)
            goto ERROR;
        nCurrentZip = nWantZip;
    }

    if (ZipLoadFile(Dest, vecRomFindList[i].nLen, pnWrote, vecRomFindList[i].nPos) != 0)
        goto ERROR;
    // log_cb(RETRO_LOG_INFO, "[LOAD ROM] %s...OK!\n", vecRomFindList[i].pszName);
    return 0;

ERROR:
    log_cb(RETRO_LOG_INFO, "[LOAD ROM] %s --- failed!\n", vecRomFindList[i].pszName);
    return 1;
}

static bool CheckZipExist(const char *szPath)
{
    char szFullPath[MAX_PATH];
    snprintf(szFullPath, MAX_PATH, "%s.zip", szPath);
    FILE *fp = fopen(szFullPath, "rb");
    if (fp)
    {
        fclose(fp);
        return true;
    }
#ifdef INCLUDE_7Z_SUPPORT
    snprintf(szFullPath, MAX_PATH, "%s.7z", szPath);
    fp = fopen(szFullPath, "rb");
    if (fp)
    {
        fclose(fp);
        return true;
    }
#endif
    return false;
}

static void FreeZipList(ZipEntry *List, INT32 nListCount)
{
    if (List)
    {
        for (INT32 i = 0; i < nListCount; i++)
            free(List[i].szName);
        free(List);
    }
}

static INT32 FindRomByCrc(UINT32 nCrc, ZipEntry *List, INT32 nListCount)
{
    for (INT32 i = 0; i < nListCount; i++)
    {
        if (nCrc == List[i].nCrc)
        {
            return i;
        }
    }
    return -1;
}

static INT32 FindRomByName(char *szName, ZipEntry *List, INT32 nListCount)
{
    for (INT32 i = 0; i < nListCount; i++)
    {
        if (List[i].szName && strcasecmp(szName, List[i].szName) == 0)
        {
            return i;
        }
    }
    return -1;
}

static INT32 LocateZipPath(const char *pszName)
{
    char szPath[MAX_PATH];

    snprintf(szPath, MAX_PATH, "%s%s", szAppRomDir, pszName);
    if (CheckZipExist(szPath))
    {
        log_cb(RETRO_LOG_INFO, "[CHECK ROM] Find %s OK!\n", pszName);
        vecZipPathList.push_back(szPath);
        return 0;
    }

    snprintf(szPath, MAX_PATH, "%s%s", szAppSystemDir, pszName);
    if (CheckZipExist(szPath))
    {
        log_cb(RETRO_LOG_INFO, "[CHECK ROM] Find %s OK!\n", pszName);
        vecZipPathList.push_back(szPath);
        return 0;
    }

    log_cb(RETRO_LOG_ERROR, "[CHECK ROM] Find %s failed!\n", pszName);
    return -1;
}

INT32 BzipOpen()
{
    INT32 nRet;
    char *pszName;
    struct BurnRomInfo ri;

    nRet = 1;
    BzipClose();
    ResetNeogeoBiosInfos();

    for (INT32 i = 0; BurnDrvGetZipName(&pszName, i) == 0; i++)
    {
        if (i == 0)
            pszName = szAppDrvName;
        LocateZipPath(pszName);
    }

    for (INT32 i = 0; BurnDrvGetRomInfo(&ri, i) == 0; i++)
    {
        BurnDrvGetRomName(&pszName, i, 0);
        vecRomFindList.push_back(RomFind());
        struct RomFind *rf = &(vecRomFindList.back());
        rf->pszName = pszName;
        rf->nCrc = ri.nCrc;
        rf->nLen = ri.nLen;
        rf->nType = ri.nType;
        rf->nState = STATUS_MISSING;
    }

    for (INT32 nZip = 0; nZip < vecZipPathList.size(); nZip++)
    {
        if (ZipOpen((char *)vecZipPathList[nZip].c_str()) != 0)
        {
            log_cb(RETRO_LOG_ERROR, "[CHECK ROM] Open %s failed!\n", vecZipPathList[nZip].c_str());
            return 1;
        }

        struct ZipEntry *List = NULL;
        INT32 nListCount = 0;
        ZipGetList(&List, &nListCount);

        for (INT32 i = 0; i < vecRomFindList.size(); i++)
        {
            if (vecRomFindList[i].nState == STATUS_OK)
                continue;

            // Skip empty rominfo
            if (vecRomFindList[i].nType == 0 && vecRomFindList[i].nLen == 0 && vecRomFindList[i].nCrc == 0)
            {
                vecRomFindList[i].nState = STATUS_OK;
                continue;
            }

            int nPos = -1;
            if (vecRomFindList[i].nCrc)
                nPos = FindRomByCrc(vecRomFindList[i].nCrc, List, nListCount);
            if (nPos < 0)
                nPos = FindRomByName(vecRomFindList[i].pszName, List, nListCount);
            if (nPos < 0)
                continue;

            if (List[nPos].nLen != vecRomFindList[i].nLen)
            {
                log_cb(RETRO_LOG_WARN, "[CHECK ROM] %s, %u, 0x%08x --- size=%u!\n",
                       vecRomFindList[i].pszName, vecRomFindList[i].nLen, vecRomFindList[i].nCrc, List[nPos].nLen);
                vecRomFindList[i].nState = STATUS_MISMATCH_SIZE;
                continue;
            }

            if (vecRomFindList[i].nCrc && vecRomFindList[i].nCrc != List[nPos].nCrc)
            {
                log_cb(RETRO_LOG_WARN, "[CHECK ROM] %s, %u, 0x%08x --- crc=0x%08x!\n",
                       vecRomFindList[i].pszName, vecRomFindList[i].nLen, vecRomFindList[i].nCrc, List[nPos].nCrc);
                // vecRomFindList[i].nState = STATUS_MISMATCH_CRC;
                // continue;
            }
            else
            {
                log_cb(RETRO_LOG_INFO, "[CHECK ROM] %s, %u, 0x%08x --- OK!\n",
                       vecRomFindList[i].pszName, vecRomFindList[i].nLen, vecRomFindList[i].nCrc);
            }

            vecRomFindList[i].nZip = nZip;
            vecRomFindList[i].nPos = nPos;
            vecRomFindList[i].nState = STATUS_OK;

            if (IS_NEOGEO_GAME)
                SetNeogeoBiosAvailability(List[nPos].szName, List[nPos].nCrc);
        }

        FreeZipList(List, nListCount);
        ZipClose();
    }

    nRet = 0;
    for (INT32 i = 0; i < vecRomFindList.size(); i++)
    {
        if (vecRomFindList[i].nState != STATUS_OK && !(vecRomFindList[i].nType & BRF_OPT) && !(vecRomFindList[i].nType & BRF_NODUMP))
        {
            nRet = 1;
            if (vecRomFindList[i].nState == STATUS_MISSING)
            {
                log_cb(RETRO_LOG_ERROR, "[CHECK ROM] %s, %u, 0x%08x --- missing!\n",
                       vecRomFindList[i].pszName, vecRomFindList[i].nLen, vecRomFindList[i].nCrc);
            }
            else
            {
                log_cb(RETRO_LOG_WARN, "[CHECK ROM] %s, %u, 0x%08x --- mismatching!\n",
                       vecRomFindList[i].pszName, vecRomFindList[i].nLen, vecRomFindList[i].nCrc);
            }
        }
    }

    BurnExtLoadRom = BzipBurnLoadRom;

    return nRet;
}

INT32 BzipClose()
{
    BurnExtLoadRom = NULL;

    vecZipPathList.clear();
    vecRomFindList.clear();

    ZipClose();
    nCurrentZip = -1;

    return 0;
}
