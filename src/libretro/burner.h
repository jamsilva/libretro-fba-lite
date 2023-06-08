// FB Alpha - Emulator for MC68000/Z80 based arcade games
//            Refer to the "license.txt" file for more info
#ifndef __BURNER_H__
#define __BURNER_H__

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>

#include "burnint.h"
#include "burn.h"

#include <libretro.h>
#include <file/file_path.h>
#include <streams/file_stream.h>

#ifndef MAX_NAME
#define MAX_NAME 256
#endif

// Macros for parsing text
// Skip whitespace
#define SKIP_WS(s)        \
	while (_istspace(*s)) \
	{                     \
		s++;              \
	}
// Find whitespace
#define FIND_WS(s)               \
	while (*s && !_istspace(*s)) \
	{                            \
		s++;                     \
	}
// Find quote
#define FIND_QT(s)               \
	while (*s && *s != _T('\"')) \
	{                            \
		s++;                     \
	}

#ifdef FBA_DEBUG
#define APP_TITLE "FB Alpha [DEBUG]"
#else
#define APP_TITLE "FB Alpha"
#endif
#define APP_DESCRIPTION "Emulator for arcade games"
#define APP_VERSION "v0.2.97.44"

#include "cd_interface.h"

// libretro.cpp
#define RETRO_GAME_TYPE_DEF 0
#define RETRO_GAME_TYPE_NEOCD 1

#define RETRO_DEVICE_ID_JOYPAD_EMPTY 255

extern INT32 kNetGame;
extern INT32 bRunPause;
extern INT32 nAudSegLen;

extern char szAppBurnVer[16];

extern INT32 nBurnDrvOkay;
extern INT32 nBurnGameType;
extern INT32 nBurnGameWidth;
extern INT32 nBurnGameHeight;
extern INT32 nBurnColorDepth;
extern UINT32 nBurnRotation;

extern bool bBurnNeogeoGame;
extern bool bBurnPgmGame;

extern UINT32 nFrameskipNum;
extern INT32 nAudSampleRate;
extern INT32 nAnalogSpeed;
extern UINT8 *pDiagActivatedKey;

extern retro_log_printf_t log_cb;
extern retro_environment_t environ_cb;
extern retro_video_refresh_t video_cb;
extern retro_audio_sample_batch_t audio_batch_cb;
extern retro_input_state_t input_state_cb;
extern retro_input_poll_t input_poll_cb;

// misc.cpp
#define QUOTE_MAX (128)												// Maximum length of "quoted strings"
INT32 QuoteRead(TCHAR **ppszQuote, TCHAR **ppszEnd, TCHAR *pszSrc); // Read a quoted string from szSrc and poINT32 to the end
TCHAR *LabelCheck(TCHAR *s, TCHAR *pszLabel);

TCHAR *ExtractFilename(TCHAR *fullname);
TCHAR *StrReplace(TCHAR *str, TCHAR find, TCHAR replace);
TCHAR *StrLower(TCHAR *str);
TCHAR *FileExt(TCHAR *str);
bool IsFileExt(TCHAR *str, TCHAR *ext);

INT32 BurnAddEndSlash(char *szPath, size_t nSize);
void BurnMakeFilename(char *szName, const char *szPath, size_t nSize);
void BurnMakeBasename(char *szName, const char *szPath, size_t nSize);
void BurnMakeBaseDir(char *szDir, const char *szPath, size_t nSize);

bool BurnCheckFileExist(const char *szPath);

char *BurnDrvGetNameByIndex(UINT32 nDrv);
UINT32 BurnDrvGetIndexByName(const char *szName);

INT32 SetBurnHighCol(INT32 nDepth);

// zipfn.cpp
struct ZipEntry
{
	char *szName;
	UINT32 nLen;
	UINT32 nCrc;
};

INT32 ZipOpen(char *szZip);
INT32 ZipClose();
INT32 ZipGetList(struct ZipEntry **pList, INT32 *pnListCount);
INT32 ZipLoadFile(UINT8 *Dest, INT32 nLen, INT32 *pnWrote, INT32 nEntry);
INT32 __cdecl ZipLoadOneFile(char *arcName, const char *fileName, void **Dest, INT32 *pnWrote);

// bzip.cpp
INT32 BzipOpen();
INT32 BzipClose();

// extdrv.cpp
extern bool bBurnExtDrvMode;

INT32 BurnDrvLoadExtDriver(const char *szPath);
void BurnDrvUnloadExtDriver();

// statec.cpp
INT32 BurnStateCompress(UINT8 **pDef, INT32 *pnDefLen, INT32 bAll);
INT32 BurnStateDecompress(UINT8 *Def, INT32 nDefLen, INT32 bAll);

// state.cpp
INT32 BurnStateLoadEmbed(FILE *fp, INT32 nOffset, INT32 bAll, INT32 (*pLoadGame)());
INT32 BurnStateLoad(TCHAR *szName, INT32 bAll, INT32 (*pLoadGame)());
INT32 BurnStateSaveEmbed(FILE *fp, INT32 nOffset, INT32 bAll);
INT32 BurnStateSave(TCHAR *szName, INT32 bAll);

// avplay.cpp
void BurnAVPlayInit();
void BurnAVPlayExit();
void BurnAVPlayFrame();

// input_bind.cpp
struct InputBind
{
	UINT8 nType;
	union
	{
		UINT8 nVal;
		UINT16 nShortVal;
	};
	UINT8 nDefVal;
	struct BurnInputInfo bii;
	struct retro_input_descriptor *prid;
};

extern struct InputBind *InputBindList;
extern UINT32 nInputBindListCount;
extern INT32 nInputBindDIPOffset;
extern INT32 nInputBindResetOffset;
extern INT32 nInputBindDiagOffset;

INT32 InputBindInit();
INT32 InputBindExit();
INT32 InputBindCleanKeys();

// input.cpp
extern UINT32 nDiagKeyActivateHoldCount;

void BurnInputInit();
void BurnInputExit();
void BurnInputFrame();

// libretro_memory.cpp
void BurnCheevosInit();
void BurnCheevosExit();

// libretro_core_options.cpp
void BurnSetVariables();
void BurnUpdateVariables();

// support_paths.cpp
extern char szAppDataPath[MAX_PATH];
extern char szAppIpsPath[MAX_PATH];

extern char szAppDrvName[MAX_NAME];
extern char szAppRomName[MAX_NAME];

extern char szAppRomDir[MAX_PATH];
extern char szAppSaveDir[MAX_PATH];
extern char szAppSystemDir[MAX_PATH];
extern char szAppAutoFsPath[MAX_PATH];
extern char szAppExtDrvDatPath[MAX_PATH];

void BurnMakeAppPaths(const char *szRomPath);

#endif