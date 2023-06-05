#ifndef CD_INTERFACE_H_
#define CD_INTERFACE_H_

#include "burner.h"

enum CDEmuStatusValue
{
	idle = 0,
	reading,
	playing,
	paused,
	seeking,
	fastforward,
	fastreverse
};

enum CDEmuReadTOCFlags
{
	CDEmuTOC_FIRSTLAST = 0x1000,
	CDEmuTOC_LASTMSF,
	CDEmuTOC_FIRSTINDEX,
	CDEmuTOC_ENDOFDISC
};

struct CDEmuDo {
	INT32			(*CDEmuExit)();
	INT32			(*CDEmuInit)();
	INT32			(*CDEmuStop)();
	INT32			(*CDEmuPlay)(UINT8 M, UINT8 S, UINT8 F);
	INT32			(*CDEmuLoadSector)(INT32 LBA, char* pBuffer);
	UINT8*          (*CDEmuReadTOC)(INT32 track);
	UINT8*          (*CDEmuReadQChannel)();
	INT32			(*CDEmuGetSoundBuffer)(INT16* buffer, INT32 samples);
	INT32           (*CDEmuScan)(INT32 nAction, INT32 *pnMin);
	const TCHAR*     szModuleName;
};

extern bool bCDEmuOkay;
extern UINT32 nCDEmuSelect;
extern CDEmuStatusValue CDEmuStatus;

extern TCHAR CDEmuImage[MAX_PATH];

INT32 CDEmuInit();
INT32 CDEmuExit();
INT32 CDEmuStop();
INT32 CDEmuPlay(UINT8 M, UINT8 S, UINT8 F);
INT32 CDEmuLoadSector(INT32 LBA, char *pBuffer);
UINT8 *CDEmuReadTOC(INT32 track);
UINT8 *CDEmuReadQChannel();
INT32 CDEmuGetSoundBuffer(INT16 *buffer, INT32 samples);
INT32 CDEmuScan(INT32 nAction, INT32 *pnMin);

CDEmuStatusValue CDEmuGetStatus();
void CDEmuStartRead();
void CDEmuPause();
void CDEmuResume();

#endif /*CD_INTERFACE_H_*/
