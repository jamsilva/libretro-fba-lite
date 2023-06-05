#ifndef __RETRO_NEOGEO_H__
#define __RETRO_NEOGEO_H__

#include "burner.h"

enum NeogeoBiosCategories
{
    NEOGEO_MVS = 1 << 0,
    NEOGEO_AES = 1 << 1,
    NEOGEO_UNI = 1 << 2,
    NEOGEO_EUR = 1 << 3,
    NEOGEO_USA = 1 << 4,
	NEOGEO_ASI = 1 << 5,
    NEOGEO_JAP = 1 << 6,
};

struct BiosInfo
{
	char *szName;
	UINT32 nCrc;
	UINT8 nSetting;
	char *szTitle;
	UINT32 nCateg;
	UINT8 nAvailable;
};

void ResetNeogeoBiosInfos();
void SetNeogeoBiosAvailability(char *szName, UINT32 nCrc);
BiosInfo *GetNeogeoBiosInfo(UINT32 nCateg);
void SetNeoSystem(UINT32 nCateg);

#endif
