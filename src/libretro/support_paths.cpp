#include "burner.h"

//-------------------------------------------
TCHAR szAppHiscorePath[MAX_PATH];
TCHAR szAppSamplesPath[MAX_PATH];
TCHAR szAppBlendPath[MAX_PATH];
TCHAR szAppHDDPath[MAX_PATH];
TCHAR szAppEEPROMPath[MAX_PATH];

//-------------------------------------------
char szAppDataPath[MAX_PATH];
char szAppIpsPath[MAX_PATH];

char szAppDrvName[MAX_NAME];
char szAppRomName[MAX_NAME];

char szAppRomDir[MAX_PATH];
char szAppSaveDir[MAX_PATH];
char szAppSystemDir[MAX_PATH];
char szAppAutoFsPath[MAX_PATH];
char szAppExtDrvDatPath[MAX_PATH];

void BurnMakeAppPaths(const char *szRomPath)
{
    char *szTmpPath = NULL;

    BurnMakeBasename(szAppRomName, szRomPath, MAX_PATH);
    BurnMakeBaseDir(szAppRomDir, szRomPath, MAX_PATH);
    snprintf(szAppExtDrvDatPath, sizeof(szAppExtDrvDatPath), "%s%s.dat", szAppRomDir, szAppRomName);

    if (nBurnGameType == RETRO_GAME_TYPE_NEOCD)
        strncpy(szAppDrvName, "neocdz", MAX_NAME);
    else
        strncpy(szAppDrvName, szAppRomName, MAX_NAME);

    // If save directory is defined use it, ...
    if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &szTmpPath) && szTmpPath)
    {
        strncpy(szAppSaveDir, szTmpPath, MAX_PATH);
        BurnAddEndSlash(szAppSaveDir, MAX_PATH);
        log_cb(RETRO_LOG_INFO, "Setting save dir to %s\n", szAppSaveDir);
    }
    else
    {
        // ... otherwise use rom directory
        strncpy(szAppSaveDir, szAppRomDir, MAX_PATH);
        log_cb(RETRO_LOG_INFO, "Save dir not defined => use roms dir %s\n", szAppSaveDir);
    }

    // If system directory is defined use it, ...
    if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &szTmpPath) && szTmpPath)
    {
        strncpy(szAppSystemDir, szTmpPath, MAX_PATH);
        BurnAddEndSlash(szAppSystemDir, MAX_PATH);
        log_cb(RETRO_LOG_INFO, "Setting system dir to %s\n", szAppSystemDir);
    }
    else
    {
        // ... otherwise use rom directory
        strncpy(szAppSystemDir, szAppRomDir, MAX_PATH);
        log_cb(RETRO_LOG_INFO, "System dir not defined => use roms dir %s\n", szAppSystemDir);
    }

    // Init data path
    snprintf(szAppDataPath, sizeof(szAppDataPath), "%s%s%c", szAppSaveDir, szAppRomName, PATH_DEFAULT_SLASH_C());
    path_mkdir(szAppDataPath);

    // Init paths for burn
    strcpy(szAppHiscorePath, szAppDataPath);
    strcpy(szAppSamplesPath, szAppDataPath);
    strcpy(szAppHDDPath, szAppDataPath);
    strcpy(szAppBlendPath, szAppDataPath);
    strcpy(szAppEEPROMPath, szAppDataPath);
}
