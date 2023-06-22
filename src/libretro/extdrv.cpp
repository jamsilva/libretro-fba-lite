#include <mxml.h>

#include "burner.h"

typedef struct RomInfoListEntry
{
    struct RomInfoListEntry *next;
    struct RomInfoListEntry *previous;
    struct BurnRomInfo *info;
} RomInfoListEntry;

typedef struct RomInfoList
{
    RomInfoListEntry *head;
    RomInfoListEntry *tail;
    UINT32 length;
} RomInfoList;

typedef struct ExtDriverEntry
{
    char *szName;
    char *szDep;
    char *szParent;
    char *szBoard;
    char *szDesc;
    RomInfoList pGameRomInfoList;
    RomInfoList pBoardRomInfoList;
    struct BurnDriver *pOriDriver;
    struct BurnDriver *pBoardDriver;
    int nDrvActive;
} ExtDriverEntry;

bool bBurnExtDrvMode = false;

static ExtDriverEntry *pExtDriverEntry = NULL;

static struct BurnRomInfo emptyRomDesc[] = {
    {"", 0, 0, 0},
};

static RomInfoListEntry *RomInfoListGetEntryByNumber(RomInfoList *list, int n)
{
    if (!list)
        return NULL;

    RomInfoListEntry *entry = list->head;

    while (n > 0 && entry)
    {
        n--;
        entry = entry->next;
    }

    if (n != 0)
        return NULL;

    return entry;
}

static BurnRomInfo *RomInfoListGetInfoByNumber(RomInfoList *list, int n)
{
    RomInfoListEntry *entry = RomInfoListGetEntryByNumber(list, n);
    if (!entry)
        return NULL;

    return entry->info;
}

static void RomInfoListAddEntry(RomInfoList *list, RomInfoListEntry *entry)
{
    if (!list || !entry)
        return;

    entry->next = NULL;
    entry->previous = NULL;

    if (list->head == NULL)
    {
        list->head = entry;
        list->tail = entry;
    }
    else
    {
        RomInfoListEntry *tail = list->tail;
        tail->next = entry;
        entry->previous = tail;
        list->tail = entry;
    }

    list->length++;
}

static bool RomInfoListAddEntryByInfo(RomInfoList *list, struct BurnRomInfo *info)
{
    if (!list)
        return false;

    RomInfoListEntry *entry = (RomInfoListEntry *)malloc(sizeof(RomInfoListEntry));
    if (!entry)
        return false;

    entry->info = info;
    RomInfoListAddEntry(list, entry);
    return true;
}

void RomInfoListEmpty(RomInfoList *list)
{
    if (!list)
        return;

    RomInfoListEntry *entry = list->head;

    while (entry)
    {
        RomInfoListEntry *next = entry->next;
        free(entry->info);
        free(entry);
        entry = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}

int RomInfoListGetEntries(RomInfoList *list, mxml_node_t *parent_node)
{
    if (!list || !parent_node)
        return -1;

    mxml_node_t *node;
    const char *element;
    const char *szName, *szSize, *szCrc, *szType;
    struct BurnRomInfo *info;

    node = mxmlGetFirstChild(parent_node);

    while (node)
    {
        element = mxmlGetElement(node);
        if (element && strcmp(element, "rom") == 0)
        {
            szName = mxmlElementGetAttr(node, "name");
            szSize = mxmlElementGetAttr(node, "size");
            szCrc = mxmlElementGetAttr(node, "crc");
            szType = mxmlElementGetAttr(node, "type");

            info = (struct BurnRomInfo *)malloc(sizeof(struct BurnRomInfo));
            if (info)
            {
                memset(info, 0, sizeof(struct BurnRomInfo));
                if (szName)
                    strcpy(info->szName, szName);
                if (szSize)
                    info->nLen = strtoul(szSize, NULL, 10);
                if (szCrc)
                    info->nCrc = strtoul(szCrc, NULL, 16);
                if (szType)
                    info->nType = strtoul(szType, NULL, 16);
                RomInfoListAddEntryByInfo(list, info);
                // log_cb(RETRO_LOG_INFO, "rom: name=%s, size=%u, crc=0x%x, type=0x%x\n", info->szName, info->nLen, info->nCrc, info->nType);
            }
        }
        node = mxmlGetNextSibling(node);
    }

    return 0;
}

static INT32 BurnGetExtRomInfo(struct BurnRomInfo *pri, UINT32 i)
{
    struct BurnRomInfo *por = NULL;

    if (i >= 0x80)
    {
        i &= 0x7F;
        if (pExtDriverEntry->pBoardDriver)
            return pExtDriverEntry->pBoardDriver->GetRomInfo(pri, i);

        if (i >= pExtDriverEntry->pBoardRomInfoList.length)
            por = NULL;
        else
            por = RomInfoListGetInfoByNumber(&pExtDriverEntry->pBoardRomInfoList, i);
    }
    else
    {
        if (i >= pExtDriverEntry->pGameRomInfoList.length)
        {
            if (pExtDriverEntry->pBoardDriver || pExtDriverEntry->pBoardRomInfoList.length > 0)
                por = emptyRomDesc + 0;
            else
                por = NULL;
        }
        else
        {
            por = RomInfoListGetInfoByNumber(&pExtDriverEntry->pGameRomInfoList, i);
        }
    }

    if (por == NULL)
    {
        return 1;
    }
    if (pri)
    {
        pri->nLen = por->nLen;
        pri->nCrc = por->nCrc;
        pri->nType = por->nType;
    }
    return 0;
}

static INT32 BurnGetExtRomName(char **pszName, UINT32 i, INT32 nAka)
{
    struct BurnRomInfo *por = NULL;

    if (i >= 0x80)
    {
        i &= 0x7F;
        if (pExtDriverEntry->pBoardDriver)
            return pExtDriverEntry->pBoardDriver->GetRomName(pszName, i, nAka);

        if (i >= pExtDriverEntry->pBoardRomInfoList.length)
            por = NULL;
        else
            por = RomInfoListGetInfoByNumber(&pExtDriverEntry->pBoardRomInfoList, i);
    }
    else
    {
        if (i >= pExtDriverEntry->pGameRomInfoList.length)
        {
            if (pExtDriverEntry->pBoardRomInfoList.length > 0 || pExtDriverEntry->pBoardDriver)
                por = emptyRomDesc + 0;
            else
                por = NULL;
        }
        else
        {
            por = RomInfoListGetInfoByNumber(&pExtDriverEntry->pGameRomInfoList, i);
        }
    }

    if (por == NULL)
    {
        return 1;
    }
    if (nAka)
    {
        return 1;
    }
    *pszName = por->szName;
    return 0;
}

INT32 BurnDrvLoadExtDriver(const char *szPath)
{
    INT32 nDrvActive;
    FILE *fp;
    int nFileSize;
    void *Buffer;
    mxml_node_t *TreeNode, *GameNode, *DescNode;
    mxml_node_t *BoardRomsNode, *GameRomsNode;
    const char *szName, *szDep, *szParent, *szBoard;
    const char *szDesc;
    int nBoardDrvIndex;
    struct BurnDriver **pDriver;

    log_cb(RETRO_LOG_INFO, "[FBA] Load external driver: %s\n", szPath);

    fp = NULL;
    Buffer = NULL;
    TreeNode = NULL;
    nBurnDrvActive = ~0U;

    BurnDrvUnloadExtDriver();
    BurnDrvGetDriverList(&pDriver);

    fp = fopen(szPath, "r");
    if (!fp)
        goto END;
    fseek(fp, 0, SEEK_END);
    nFileSize = ftell(fp);
    if (nFileSize <= 0)
        goto END;
    Buffer = malloc(nFileSize);
    if (!Buffer)
        goto END;
    fseek(fp, 0, SEEK_SET);
    fread(Buffer, 1, nFileSize, fp);

    // TreeNode = mxmlLoadFile(NULL, fp, MXML_OPAQUE_CALLBACK);
    TreeNode = mxmlLoadString(NULL, (char *)Buffer, MXML_OPAQUE_CALLBACK);
    if (!TreeNode)
        goto END;
    // log_cb(RETRO_LOG_INFO, "mxmlLoadFile OK!\n");

    GameNode = mxmlFindElement(TreeNode, TreeNode, "game", NULL, NULL, MXML_DESCEND);
    if (!GameNode)
        goto END;
    // log_cb(RETRO_LOG_INFO, "mxmlFindElement \"game\" OK!\n");

    szName = mxmlElementGetAttr(GameNode, "name");
    szDep = mxmlElementGetAttr(GameNode, "dep");
    szParent = mxmlElementGetAttr(GameNode, "parent");
    szBoard = mxmlElementGetAttr(GameNode, "board");
    // log_cb(RETRO_LOG_INFO, "game: name=%s, dep=%s, parent=%s, board=%s\n", szName, szDep, szParent, szBoard);

    if (!szDep)
        goto END;
    
    log_cb(RETRO_LOG_INFO, "[FBA] External driver depend on: %s\n", szDep);

    nDrvActive = BurnDrvGetIndexByName(szDep);
    if (nDrvActive == ~0U)
        goto END;

    szDesc = NULL;
    DescNode = mxmlFindElement(GameNode, TreeNode, "description", NULL, NULL, MXML_DESCEND);
    if (DescNode)
        szDesc = mxmlGetOpaque(DescNode);
    // log_cb(RETRO_LOG_INFO, "description=%s\n", szDesc);

    pExtDriverEntry = (ExtDriverEntry *)malloc(sizeof(ExtDriverEntry));
    if (!pExtDriverEntry)
        goto END;

    memset(pExtDriverEntry, 0, sizeof(ExtDriverEntry));
    if (szName)
    {
        pExtDriverEntry->szName = (char *)malloc(strlen(szName) + 1);
        if (pExtDriverEntry->szName)
            strcpy(pExtDriverEntry->szName, szName);
    }
    if (szDep)
    {
        pExtDriverEntry->szDep = (char *)malloc(strlen(szDep) + 1);
        if (pExtDriverEntry->szDep)
            strcpy(pExtDriverEntry->szDep, szDep);
    }
    if (szParent)
    {
        pExtDriverEntry->szParent = (char *)malloc(strlen(szParent) + 1);
        if (pExtDriverEntry->szParent)
            strcpy(pExtDriverEntry->szParent, szParent);
    }
    if (szBoard)
    {
        pExtDriverEntry->szBoard = (char *)malloc(strlen(szBoard) + 1);
        if (pExtDriverEntry->szBoard)
            strcpy(pExtDriverEntry->szBoard, szBoard);
    }
    if (szDesc)
    {
        pExtDriverEntry->szDesc = (char *)malloc(strlen(szDesc) + 1);
        if (pExtDriverEntry->szDesc)
            strcpy(pExtDriverEntry->szDesc, szDesc);
    }

    GameRomsNode = mxmlFindElement(GameNode, TreeNode, "game-roms", NULL, NULL, MXML_DESCEND);
    if (GameRomsNode)
    {
        // log_cb(RETRO_LOG_INFO, "mxmlFindElement \"game-roms\" OK!\n");
        RomInfoListGetEntries(&pExtDriverEntry->pGameRomInfoList, GameRomsNode);
    }

    BoardRomsNode = mxmlFindElement(GameNode, TreeNode, "board-roms", NULL, NULL, MXML_DESCEND);
    if (BoardRomsNode)
    {
        // log_cb(RETRO_LOG_INFO, "mxmlFindElement \"board-roms\" OK!\n");
        RomInfoListGetEntries(&pExtDriverEntry->pBoardRomInfoList, BoardRomsNode);
    }

    if (pExtDriverEntry->pBoardRomInfoList.length <= 0 && pExtDriverEntry->pGameRomInfoList.length > 0)
    {
        nBoardDrvIndex = ~0U;
        if (szBoard)
            nBoardDrvIndex = BurnDrvGetIndexByName(szBoard);
        else if (pDriver[nDrvActive]->szBoardROM)
            nBoardDrvIndex = BurnDrvGetIndexByName(pDriver[nDrvActive]->szBoardROM);
        if (nBoardDrvIndex != ~0U)
            pExtDriverEntry->pBoardDriver = pDriver[nBoardDrvIndex];
    }

    pExtDriverEntry->nDrvActive = nDrvActive;
    pExtDriverEntry->pOriDriver = (struct BurnDriver *)malloc(sizeof(struct BurnDriver));
    if (!pExtDriverEntry->pOriDriver)
        goto END;

    memcpy(pExtDriverEntry->pOriDriver, pDriver[nDrvActive], sizeof(struct BurnDriver));

    // pDriver[nDrvActive]->szShortName = pExtDriverEntry->szName;
    if (pExtDriverEntry->szParent)
        pDriver[nDrvActive]->szParent = pExtDriverEntry->szParent;
    if (pExtDriverEntry->szBoard)
        pDriver[nDrvActive]->szBoardROM = pExtDriverEntry->szBoard;
    if (pExtDriverEntry->szDesc)
        pDriver[nDrvActive]->szFullNameA = pExtDriverEntry->szDesc;
    if (pExtDriverEntry->pGameRomInfoList.length > 0)
    {
        pDriver[nDrvActive]->GetRomInfo = BurnGetExtRomInfo;
        pDriver[nDrvActive]->GetRomName = BurnGetExtRomName;
    }

    bBurnExtDrvMode = true;
    nBurnDrvActive = nDrvActive;

END:
    if (fp)
        fclose(fp);
    if (Buffer)
        free(Buffer);
    if (TreeNode)
        mxmlDelete(TreeNode);

    if (nBurnDrvActive == ~0U || nBurnDrvActive >= nBurnDrvCount)
    {
        BurnDrvUnloadExtDriver();
        log_cb(RETRO_LOG_ERROR, "[FBA] Load external driver failed!\n");
    }
    else
    {
        log_cb(RETRO_LOG_INFO, "[FBA] Load external driver OK!\n");
    }

    return nBurnDrvActive;
}

void BurnDrvUnloadExtDriver()
{
    struct BurnDriver **pDriver;

    bBurnExtDrvMode = false;

    if (!pExtDriverEntry)
        return;

    log_cb(RETRO_LOG_INFO, "[FBA] Unload external driver...\n");

    BurnDrvGetDriverList(&pDriver);

    // Backup this active driver
    if (pExtDriverEntry->pOriDriver)
    {
        // pDriver[pExtDriverEntry->nDrvActive]->szShortName = pExtDriverEntry->pOriDriver->szShortName;
        pDriver[pExtDriverEntry->nDrvActive]->szParent = pExtDriverEntry->pOriDriver->szParent;
        pDriver[pExtDriverEntry->nDrvActive]->szBoardROM = pExtDriverEntry->pOriDriver->szBoardROM;
        pDriver[pExtDriverEntry->nDrvActive]->szFullNameA = pExtDriverEntry->pOriDriver->szFullNameA;
        pDriver[pExtDriverEntry->nDrvActive]->GetRomInfo = pExtDriverEntry->pOriDriver->GetRomInfo;
        pDriver[pExtDriverEntry->nDrvActive]->GetRomName = pExtDriverEntry->pOriDriver->GetRomName;
        free(pExtDriverEntry->pOriDriver);
    }

    RomInfoListEmpty(&pExtDriverEntry->pGameRomInfoList);
    RomInfoListEmpty(&pExtDriverEntry->pBoardRomInfoList);

    if (pExtDriverEntry->szName)
        free(pExtDriverEntry->szName);
    if (pExtDriverEntry->szDep)
        free(pExtDriverEntry->szDep);
    if (pExtDriverEntry->szParent)
        free(pExtDriverEntry->szParent);
    if (pExtDriverEntry->szBoard)
        free(pExtDriverEntry->szBoard);
    if (pExtDriverEntry->szDesc)
        free(pExtDriverEntry->szDesc);

    free(pExtDriverEntry);
    pExtDriverEntry = NULL;
}